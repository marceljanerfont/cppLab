#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

#include "kalman_filter.h"


int main()
{
    // Camera frame
    cv::Mat frame;

    KalmanFilter kf;

    // Camera Index
    int idx = 0;

    // Camera Capture
    cv::VideoCapture cap;

    // Camera Settings
    if (!cap.open(idx))
    {
        std::cout << "Webcam not connected.\n" << "Please verify\n";
        return EXIT_FAILURE;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1024);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 768);
    // Camera Settings

    std::cout << "\nHit 'q' to exit...\n";

    char ch = 0;

    double ticks = 0;
    bool found = false;

    int notFoundCount = 0;
    const int MAX_SKIPPED = 2;
    int skippedMeasures = 0;

    // Main loop
    while (ch != 'q' && ch != 'Q')
    {
        // Frame acquisition
        cap >> frame;
        flip(frame, frame, 1);
        cv::Mat res;
        frame.copyTo( res );

        if (found)
        {
            kf.doPrediction();
            int x_min, y_min, x_max, y_max;
            kf.getCurrentState(x_min, y_min, x_max, y_max);
            cv::rectangle(res, cv::Point2i(x_min, y_min), cv::Point2i(x_max, y_max), CV_RGB(255, 0, 0), 2);
        }

        // Noise smoothing
        cv::Mat blur;
        cv::GaussianBlur(frame, blur, cv::Size(5, 5), 3.0, 3.0);
        // Noise smoothing

        // HSV conversion
        cv::Mat frmHsv;
        cv::cvtColor(blur, frmHsv, cv::COLOR_BGR2HSV);
        // HSV conversion

        // Color Thresholding
        // Note: change parameters for different colors
        cv::Mat rangeRes = cv::Mat::zeros(frame.size(), CV_8UC1);
        // Blue in HUE : [100, 150]
        cv::inRange(frmHsv, cv::Scalar(100, 150, 150), cv::Scalar(150, 255, 255), rangeRes);
        // Color Thresholding

        // Improving the result
        cv::erode(rangeRes, rangeRes, cv::Mat(), cv::Point(-1, -1), 4);
        cv::dilate(rangeRes, rangeRes, cv::Mat(), cv::Point(-1, -1), 4);
        // Improving the result

        // Thresholding viewing
        cv::imshow("Threshold", rangeRes);

        // Contours detection
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(rangeRes, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        // Contours detection

        // Filtering
        std::vector<std::vector<cv::Point> > balls;
        std::vector<cv::Rect> ballsBox;
        for (size_t i = 0; i < contours.size(); i++)
        {
            cv::Rect bBox;
            bBox = cv::boundingRect(contours[i]);

            float ratio = (float) bBox.width / (float) bBox.height;
            if (ratio > 1.0f)
                ratio = 1.0f / ratio;

            // Searching for a bBox almost square
            if (ratio > 0.75 && bBox.area() >= 500)
            {
                balls.push_back(contours[i]);
                ballsBox.push_back(bBox);
            }
        }
        // Filtering

        //cout << "Balls found:" << ballsBox.size() << endl;

        // Detection result
        for (size_t i = 0; i < balls.size(); i++) {
          //cv::drawContours(res, balls, i, CV_RGB(20,150,20), 1);
          cv::rectangle(res, ballsBox[i], CV_RGB(0,255,0), 2);

          cv::Point center;
          center.x = ballsBox[i].x + ballsBox[i].width / 2;
          center.y = ballsBox[i].y + ballsBox[i].height / 2;
          cv::circle(res, center, 2, CV_RGB(20,150,20), -1);

          std::stringstream sstr;
          sstr << "(" << center.x << ", " << center.y << ")";
          cv::putText(res, sstr.str(),
            cv::Point(center.x - ballsBox[i].width / 2, center.y - (ballsBox[i].height / 2) - 5),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0, 0, 0), 1);
        }
        // Detection result

        // Kalman Update
        if (balls.size() == 0) {
          notFoundCount++;
          //cout << "notFoundCount:" << notFoundCount << endl;
          if( notFoundCount >= 100 ) {
            found = false;
          }
        } else { 
          // we have a measure
          if (skippedMeasures == MAX_SKIPPED) {
            skippedMeasures = 0;
            notFoundCount = 0;
            if (!found) {
              // First detection!
              std::cout << "* * * * first measure!\n";
              kf.doMeasure(ballsBox[0].x, ballsBox[0].y, ballsBox[0].x + ballsBox[0].width, ballsBox[0].y + ballsBox[0].height, true);
              found = true;
            }
            else {
              // skip measure
              kf.doMeasure(ballsBox[0].x, ballsBox[0].y, ballsBox[0].x + ballsBox[0].width, ballsBox[0].y + ballsBox[0].height, false);
            }
          }
          else {
            ++skippedMeasures;
          }
        }
        // Kalman Update

        // Final result
        cv::imshow("Tracking", res);

        // User key
        ch = cv::waitKey(1);
    }
    // Main loop

    return EXIT_SUCCESS;
}
