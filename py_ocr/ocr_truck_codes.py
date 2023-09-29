#pip install -r requirements.txt
#  Ctrl+Shift+P
# Python: Select Interpreter
# python env mmocr_env
from PIL import Image
import matplotlib.pyplot as plt
from mmocr.apis import MMOCRInferencer
import cv2
import numpy as np
from sklearn.cluster import DBSCAN
from datetime import datetime
import logging
import amqppy
import shutil
import json
from logging.handlers import RotatingFileHandler
import configparser
import re
import time
from elasticsearch import Elasticsearch

# logger
# Create a logger
logger = logging.getLogger("ocr")
logger.setLevel(logging.DEBUG)  # Set the logging level
# Create a formatter 
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

# Create a console handler
console_handler = logging.StreamHandler()
console_handler.setLevel(logging.DEBUG)
console_handler.setFormatter(formatter)

# Create a rotating file handler
log_file = "ocr.log"
max_log_size = 20 * 1024 * 1024  # 20 MB
backup_count = 100  # Number of backup log files
file_handler = RotatingFileHandler(log_file, maxBytes=max_log_size, backupCount=backup_count)
file_handler.setLevel(logging.DEBUG)
file_handler.setFormatter(formatter)

# Add handlers to the logger
logger.addHandler(console_handler)
logger.addHandler(file_handler)

# MMOCR
# only text region detection
logger.info("mmocr detector inferencer loading...")
#ocr_det = MMOCRInferencer(det='DBNetpp', rec=None, device='cuda:0')
ocr_det = MMOCRInferencer(det='FCENet', rec=None, device='cuda:0')

logger.info("mmocr detector inferencer loaded")
# only text cognition
logger.info("mmocr recognizer inferencer loading...")
ocr_rec = MMOCRInferencer(det=None, rec='MASTER', device='cuda:0')
logger.info("mmocr recognizer inferencer loaded")
######

# Load the configuration file
logger.info("Loading config.ini file...")
config = configparser.ConfigParser()
config.read('config.ini')

# Extract parameters
CODE_ORIENTATION = config['code']['orientation']
CODE_REGEX = config['code']['regex']
CLUSTER_PADDING = int(config['code']['cluster_padding'])
CLUSTER_INTER_SAPCE = float(config['code']['cluster_interspace'])
CHAR_WIDTH_MIN = int(config['code']['char_width_min'])
CHAR_WIDTH_MAX = int(config['code']['char_width_max'])
CHAR_HEIGHT_MIN = int(config['code']['char_height_min'])
CHAR_HEIGHT_MAX = int(config['code']['char_height_max'])

VIDEO_ROOT = config['folders']['video_root']
OUTPUT_PATH = config['folders']['output_path']



logger.info(f'Code Orientation: {CODE_ORIENTATION}')
logger.info(f'Regex Pattern: {CODE_REGEX}')
logger.info(f'Cluster Padding: {CLUSTER_PADDING}')
logger.info(f'Cluster Interspace: {CLUSTER_INTER_SAPCE}')
logger.info(f'char min size: {CHAR_WIDTH_MIN}x{CHAR_HEIGHT_MIN}')
logger.info(f'char max size: {CHAR_WIDTH_MAX}x{CHAR_HEIGHT_MAX}')

logger.info(f'Video Root: {VIDEO_ROOT}')
logger.info(f'Output Path: {OUTPUT_PATH}')
#####

# Filter low score detected polygons
def filter_polygons(polygons, scores, threshold):
    filtered_polygons = []
    for polygon, score in zip(polygons, scores):
        if score >= threshold:
            filtered_polygons.append(polygon)
    return filtered_polygons

# Convert polygons to bounding boxes
def poly_to_bbox(poly):
    x_coords = poly[0::2]
    y_coords = poly[1::2]
    return (min(x_coords), min(y_coords), max(x_coords), max(y_coords))

#convert polygons to bboxes
def polys_to_bboxes(polys):
    bboxes = [poly_to_bbox(poly) for poly in polys]
    return bboxes

# merge grouped bounding boxes
def merge_bboxes(bboxes, min_dist):
    # Extract center points of bounding boxes
    center_points = [[(bbox[0] + bbox[2]) / 2.0, (bbox[1] + bbox[3]) / 2.0] for bbox in bboxes]
    # Convert the list of center points to a numpy array
    center_points_array = np.array(center_points)
    
    # Use DBSCAN from scikit-learn to cluster center points
    clustering = DBSCAN(eps=min_dist, min_samples=1).fit(center_points_array)
    labels = clustering.labels_
    
    merged_bboxes = []
    for label in set(labels):
        idxs = np.where(labels == label)[0]
        clustered_bboxes = [bboxes[i] for i in idxs]

        # Merge bounding boxes in each cluster
        x_min = min([bbox[0] for bbox in clustered_bboxes])
        y_min = min([bbox[1] for bbox in clustered_bboxes])
        x_max = max([bbox[2] for bbox in clustered_bboxes])
        y_max = max([bbox[3] for bbox in clustered_bboxes])

        merged_bboxes.append([x_min, y_min, x_max, y_max])

    return merged_bboxes

def draw_polygons(img, polygons):
    img2 = cv2.copyTo(img, None)
    for polygon in polygons:
        # Reshape the polygon points and convert them to int32
        pts = np.array(polygon).reshape((-1, 2)).astype(np.int32)
        # Draw the polygon on the image
        cv2.polylines(img2, [pts], isClosed=True, color=(0, 255, 0), thickness=3)
    cv2.imshow('Image with Polygons', img2)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

def draw_bboxes(img, bboxes):
    img2 = cv2.copyTo(img, None)
    for bbox in bboxes:
        # Extract coordinates
        x_min, y_min, x_max, y_max = map(int, bbox)
        # Draw bounding box on the image
        cv2.rectangle(img2, (x_min, y_min), (x_max, y_max), color=(0, 255, 0), thickness=3)
    # Display the image
    cv2.imshow('Image with Bounding Boxes', img2)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

from PIL import Image

def mmocr_horizontal_text_from_image(img, show_images):
    height, width = img.shape[:2]
   
    # Display the ROI using OpenCV
    if (show_images):        
        cv2.imshow('Opened Image', img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    
    numpy_array_image = np.array(img)
    result = ocr_rec(numpy_array_image, return_vis=False, save_pred=False, save_vis=False)
    # # Visualize the results
    # plt.figure(figsize=(9, 16))
    # plt.imshow(result['visualization'][0])
    # plt.show()
    if len(result['predictions']) > 0:
        return result['predictions'][0]['rec_texts'][0], result['predictions'][0]['rec_scores'][0]
    return "", 0.

def mmocr_vertical_text_from_image(img, debug):
    height, width = img.shape[:2]
    
    # Binarization
    gray_image = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    _, img_bin = cv2.threshold(gray_image, 128, 255, cv2.THRESH_BINARY_INV)
    
    # Display the ROI using OpenCV
    if debug:        
        cv2.imshow('Opened Image', img_bin)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    # Detect the contours of the digits
    contours, _ = cv2.findContours(img_bin, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    # Display contours
    if debug:
        img_with_contours = img.copy()  # Create a copy of the original image
        cv2.drawContours(img_with_contours, contours, -1, (255, 255, 255), 2)  # Draw all contours
        cv2.imshow('Image with Contours', img_with_contours)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    
    digits = []
    scores = []
    
    for contour in contours:
        x, y, w, h = cv2.boundingRect(contour)
        
        # skip small countours
        if w < CHAR_WIDTH_MIN or h < CHAR_HEIGHT_MIN or w > CHAR_WIDTH_MAX or h > CHAR_HEIGHT_MAX:
            if debug:
                print(f"Skipping character, size: {w}x{h}")
            continue        
        
        # Extract the region of interest (ROI) containing the digit
        MARGIN = 6
        roi = img[max(0, y - MARGIN):min(height, y + h + MARGIN), max(0, x - MARGIN):min(width, x + w + MARGIN)]
        # Display the ROI using OpenCV
        if debug:
            cv2.imshow('ROI Image', roi)
            cv2.waitKey(0)
            cv2.destroyAllWindows()
            
        numpy_array_image = np.array(roi)
        result = ocr_rec(numpy_array_image, return_vis=False, save_pred=False, save_vis=False)
        if len(result['predictions']) > 0:
            text = result['predictions'][0]['rec_texts'][0]
            score = result['predictions'][0]['rec_scores'][0]
            print(f"vertical char: {text}, score: {score}")
            digits.append((y, text))  # Save the digit with its y-coordinate
            scores.append(score)

    # Sort the digits based on their y-coordinates (top-to-bottom)
    sorted_digits = sorted(digits, key=lambda x: x[0])
    # Extract just the digit values (discard the y-coordinates)
    text = [digit[1] for digit in sorted_digits]
    text_clean = ''.join(text)
    scores_mean = np.mean(scores)
    print(f"vertical code: {text_clean} -> {scores_mean}")
    
    return text_clean, scores_mean

def find_codes(image_path, debug=False):
    codes = []
    logger.info("mmocr detector inferencering...")
    result = ocr_det(image_path, out_dir='outputs/', return_vis=debug, save_pred=False, save_vis=True)
    logger.info("mmocr detector inferencered")

    # Visualize the results
    if debug:
        plt.figure(figsize=(9, 16))
        plt.imshow(result['visualization'][0])
        plt.show()

    # process detections regions
    polygons= result["predictions"][0]["det_polygons"]
    scores= result["predictions"][0]["det_scores"]
    # Load the image with OpenCV
    img = cv2.imread(image_path)
    # all detected polygons
    if debug:
        draw_polygons(img, polygons)
    # filtering polygons
    polygons = filter_polygons(polygons, scores, threshold = 0.8)
    if len(polygons) == 0:
        return codes
    
    if debug:
        draw_polygons(img, polygons)

    # all boxes
    bboxes = polys_to_bboxes(polygons)
    #draw_bboxes(img, bboxes)
    if len(bboxes) == 0:
        return codes

    # merge bboxes
    #merged_bboxes = bboxes
    merged_bboxes = merge_bboxes(bboxes, min_dist=CLUSTER_INTER_SAPCE)
    #draw_bboxes(img, merged_bboxes)

    height, width = img.shape[:2]
    # extract image of the detected region
    for bbox in merged_bboxes:
        # Extract coordinates
        x_min, y_min, x_max, y_max = map(int, bbox)
        w = x_max - x_min
        h = y_max -y_min
        # process only vertical
        aspect_ratio = w/h
    
        # Extract the region of interest (ROI) containing the digit
        MARGIN = CLUSTER_PADDING
        roi_img = img[max(0, y_min - MARGIN):min(height, y_max + MARGIN), max(0, x_min - MARGIN):min(width, x_max + MARGIN)]
        if debug:
            cv2.imshow('ROI Image', roi_img)
            cv2.waitKey(0)
            cv2.destroyAllWindows()
            print(f"{w}x{h} -> {aspect_ratio}")
        code = ""
        score = 0.0
        if aspect_ratio < 0.4 and CODE_ORIENTATION in ['vertical', 'both']: 
            # vertical
            if w < 80:
                code, score = mmocr_vertical_text_from_image(roi_img, debug)
        if aspect_ratio > 2.5 and CODE_ORIENTATION in ['horizontal', 'both']: 
            # horizontal
            if h < 80 and w < 250:
                code, score = mmocr_horizontal_text_from_image(roi_img, show_images=debug)
        if len(code) > 0:
            matches = re.findall(CODE_REGEX, code)
            #logger.info(f'Matches in code [{code}]: {matches}')
            if len(matches) > 0:
                codes.append({"code": code, "score": score, "bbox": bbox})


    return codes

def do_es_update_by_queries(update_query):
    index = update_query["index"]
    query = update_query["query"]
    script = update_query["script"]
    logger.debug(f"ES update_by_query, index: {index}, query: {query}, script: {script}, after 1 seconds...")
    time.sleep(1)
    response = {'updated': 0, 'failures': None}
    try:
        # TODO: try later in case it fails
        es = Elasticsearch([{'host': 'localhost', 'port': 9200, 'scheme': 'https'}],  basic_auth=('user_xx', 'pwd_xxx'), verify_certs=True, connections_per_node=1)
        response = es.update_by_query(index=index, query=query, script=script, conflicts='proceed')
        es.close()
    except Exception as e:
        logger.error("ES update_by_query exception: " + str(e))       
    docs_updated = response['updated']
    logger.info(f"ES updated docs: {docs_updated}")
    for failure in response['failures']:
        logger.warning(f"ES updating failure: {failure}")

def update_elasticsearch_event(object_id, code, bbox):
    #"BBOX (1870.0,0.0,1920.0,26.0)"
    image_position = "BBOX (" + str(bbox[0]) + "," + str(bbox[1]) + "," + str(bbox[2]) + "," + str(bbox[3]) + ")"
    update_query = {
        "index": "event_*",
        "query": {
            "bool": {
                "filter": { "terms": { "object_id.keyword": [object_id] } } 
            }
        },
        "script": {
            "source": "ctx._source.registration = params.new_field",
            "params": {
                "new_field": {
                    "text": code,
                    "image_position" : image_position
                    }
            }
        }
    }
    do_es_update_by_queries(update_query)

def on_topic_status(exchange, routing_key, headers, body):
    msg_str = body.rstrip('\n\x00')
    msg_dict = json.loads(msg_str)
    #print(json.dumps(msg_dict, indent=4))
    object_id = msg_dict["object_id"]
    event_time = msg_dict["event_time"]
    logger.info(f"event_time: {event_time}")
    date_part, time_part = event_time.split('T')
    hour, minute, second = time_part.split(':')
    camera = msg_dict["camera_uuid"]
    image_path = f"{VIDEO_ROOT}/{date_part}_{hour}/{minute}/{camera}/{second}.jpg"
    format_string = "%Y-%m-%dT%H:%M:%S.%f"
    parsed_datetime = datetime.strptime(event_time, format_string)
    datetime_str = parsed_datetime.strftime("%Y%m%dT%H%M%S")
    image_path_dst = f"{OUTPUT_PATH}/{datetime_str}.jpg"
    json_path_dst = f"{OUTPUT_PATH}/{datetime_str}.json"
    shutil.copy(image_path, image_path_dst)
    logger.info(f"Processing: {image_path_dst}")
    result = {"result": find_codes(image_path_dst)}
    logger.info(f"Result: {result}")
    
    if len(result['result']) > 0:
        update_elasticsearch_event(object_id, result['result'][0]['code'], result['result'][0]['bbox'])
    json_str = json.dumps(result, indent=4)
    logger.info(json_str)
    # Save the codes to the JSON file
    with open(json_path_dst, "w") as json_file:
        json_file.write(json_str)
    logger.info(f"Done: {image_path_dst}")

def listen_va_events():
    try:
        # connect to the broker
        worker = amqppy.Worker(broker='amqp://user_xxx:pwd_xxx@localhost:5672//')
        # subscribe to a topic: 'va-events'
        worker.add_topic(exchange='va-event',
                        routing_key='event.alarm.start',
                        on_topic_callback=on_topic_status,
                        exclusive=False)
        # wait until worker is stopped or an uncaught exception
        logger.info('Waiting for topics events, to cancel press ctrl + c')
        worker.run()
    except KeyboardInterrupt:
        worker.stop()

def process_image(image_path):
    logger.info(f"Processing: {image_path}")
    result = {"result": find_codes(image_path, debug=True)}
    logger.info(f"Result: {result}")

def main():
    #process_image('C:/PROJECTS/OCR/outputs/20230929T060910.jpg')
    listen_va_events();    
    logger.info('Exiting')    

# Call the main function if the script is executed
if __name__ == "__main__":
    main()