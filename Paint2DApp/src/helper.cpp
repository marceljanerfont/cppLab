#include "helper.h"

#include <fstream>
#include <iostream>

#include <QPainter>
#include <QPaintEvent>
#include <QWidget>

#include <opencv2/imgproc.hpp>

#include "geometry_utils.h"

Helper::Helper(const std::string &filename, const int &width, const int &height):
  m_filename(filename), m_width(width), m_height(height), m_nbColumns(width / FACTOR), m_nbRows(height / FACTOR) {
  background = QBrush(QColor(64, 32, 64));
  // allocate
  m_segmentation = new char [m_nbColumns * m_nbRows];

  qsrand(time(0));

  m_blackPen = QPen(Qt::black);
  m_blackPen.setWidth(FACTOR);

  m_redPen = QPen(Qt::red);
  m_redPen.setWidth(FACTOR);

  loadSegmentation();
}
Helper::~Helper() {

  delete [] m_segmentation;

  for (auto &it : m_regions) {
    region &region = it.second;
    delete [] region.mask;
    region.qContours.clear();
  }
}

void Helper::paint(QPainter *painter, QPaintEvent *event, int elapsed) {
  painter->fillRect(event->rect(), background);

  painter->save();

  for (int row = 0; row < m_nbRows; ++row) {
    for (int col = 0; col < m_nbColumns; ++col) {
      char id = m_segmentation[col + row * m_nbColumns];
      if (id > 0) {
        setPixelPen(id, painter);
        painter->drawPoint(col * FACTOR, row * FACTOR);
      } else {
        painter->setPen(m_blackPen);
        painter->drawPoint(col * FACTOR, row * FACTOR);
      }
    }
  }

  // paint contour regions
  for (auto &it : m_regions) {
    region &region = it.second;
    if (region.valid) {
      painter->setPen(region.penContour);
      for (auto &pPoints : region.qContours) {
        painter->drawPolyline(pPoints);
      }
    }
  }

  painter->restore();

}

void Helper::setPixelPen(char id, QPainter *painter) {
  auto it = m_regions.find(id);
  if (it == m_regions.end()) { // not found
    painter->setPen(m_redPen);
  } else {
    painter->setPen(it->second.penMask);
  }
}


QColor Helper::getRandomColor() {
  const int MAX = 200;
  QColor color(qrand() % MAX, qrand() % MAX, qrand() % MAX);
  while (color == Qt::black) {
    color = QColor(qrand() % MAX, qrand() % MAX, qrand() % MAX);
  }

  return color;
}

void Helper::loadSegmentation() {
  // read file
  std::ifstream ifs;
  ifs.open(m_filename.c_str(), std::ios::binary);
  if (!ifs.is_open()) {
    std::string error("Could not open ");
    error += m_filename;
    throw std::exception(error.c_str());
  }

  for (int row = 0; row < m_nbRows; ++row) {
    ifs.read(&m_segmentation[row * m_nbColumns], m_nbColumns);
  }
  ifs.close();

  // split regions
  for (int row = 0; row < m_nbRows; ++row) {
    for (int col = 0; col < m_nbColumns; ++col) {
      char id = m_segmentation[col + row * m_nbColumns];
      if (id > 0) { // is region
        // exist?
        auto it = m_regions.find(id);
        if (it == m_regions.end()) { // not found
          region newRegion;
          newRegion.id = id;
          newRegion.mask = new char [m_nbColumns * m_nbRows];
          memset(newRegion.mask, 0, m_nbColumns * m_nbRows);
          it = m_regions.insert(std::make_pair(newRegion.id, newRegion)).first;
        }
        it->second.mask[col + row * m_nbColumns] = (char)255;
      }
    }
  }

  // find contours
  for (auto &it : m_regions) {
    region &region = it.second;
    // set colors and pend
    QColor color = getRandomColor();
    region.penMask = QPen(color);
    region.penMask.setWidth(FACTOR);

    region.penContour = QPen(color.lightness() > 127 ? color.darker(175) : color.lighter(175));
    region.penContour.setWidth(FACTOR + 2);

    // find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::Mat cvImage(m_nbRows, m_nbColumns, CV_8UC1, region.mask, m_nbColumns);
    cv::findContours(cvImage, contours, cv::RetrievalModes::RETR_EXTERNAL, cv::ContourApproximationModes::CHAIN_APPROX_SIMPLE);

    const double MAX_DISTANCE_PIXELS = 1.5;
    for (auto contour : contours) {
      if (contour.size() >= 3) {
        region.valid = true;
        QVector<QPoint> qContour;

        bool is_closed = contour.begin() == contour.end();
        // no simplify contour
        //for (auto cvPoint : contour) {
        //  qContour.push_back(QPoint(cvPoint.x * FACTOR, cvPoint.y * FACTOR));
        //}

        ////// simplify contours //////

        // with geometry_utils
        int nbPoints = (int)contour.size();
        gu::Point *guContour = new gu::Point[nbPoints];
        for (int i = 0; i < nbPoints; ++i) {
          guContour[i] = gu::Point(contour[i].x, contour[i].y);
        }
        int nbPointsSimple;
        gu::Point *guContourSimple = new gu::Point[nbPoints];
        gu::approxPolyDP(guContour, nbPoints, guContourSimple, &nbPointsSimple, MAX_DISTANCE_PIXELS);
        for (int i = 0; i < nbPointsSimple; ++i) {
          gu::Point guPoint = guContourSimple[i];
          qContour.push_back(QPoint(guPoint.x * FACTOR, guPoint.y * FACTOR));
        }
        delete [] guContour;
        delete [] guContourSimple;


        //  with opencv
        //std::vector<cv::Point> contourApprox;
        //cv::approxPolyDP(contour, contourApprox, MAX_DISTANCE_PIXELS, false);
        //for (auto cvPoint : contourApprox) {
        //  qContour.push_back(QPoint(cvPoint.x * FACTOR, cvPoint.y * FACTOR));
        //}

        qContour.push_back(*qContour.begin());
        region.qContours.push_back(qContour);
      }
    }
  }
}
