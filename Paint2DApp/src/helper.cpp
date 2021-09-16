#include "helper.h"

#include <fstream>
#include <iostream>

#include <QPainter>
#include <QPaintEvent>
#include <QWidget>

#include <opencv2/imgproc.hpp>

Helper::Helper(const std::string &filename, const int &width, const int &height):
  m_filename(filename), m_width(width), m_height(height), m_nbColumns(width / FACTOR), m_nbRows(height / FACTOR) {
  background = QBrush(QColor(64, 32, 64));
  // allocate
  //m_segmentation = new char *[m_nbRows];
  //for (int row = 0; row < m_nbRows; ++row) {
  //  m_segmentation[row] = new char[m_nbColumns];
  //  memset(m_segmentation[row], 0, m_nbColumns);
  //}

  m_segmentation = new char [m_nbColumns * m_nbRows];

  qsrand(time(0));

  m_blackPen = QPen(Qt::black);
  m_blackPen.setWidth(FACTOR);

  m_redPen = QPen(Qt::red);
  m_redPen.setWidth(FACTOR);

  for (int i = 0; i < MAX_PENS; ++i) {
    QColor color(qrand() % 255, qrand() % 255, qrand() % 255);
    while (color == Qt::black) {
      color = QColor(qrand() % 255, qrand() % 255, qrand() % 255);
    }
    m_pens[i] = QPen(color);
    m_pens[i].setWidth(FACTOR);
  }

  loadSegmentation();
}
Helper::~Helper() {
  // deallocate
  //for (int row = 0; row < m_nbRows; ++row) {
  //  delete [] m_segmentation[row];
  //}
  delete [] m_segmentation;
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
        it->second.mask[col + row * m_nbColumns] = 255;
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

    region.penContour = QPen(color.lightness() > 127 ? color.darker(170) : color.lighter(170));
    region.penContour.setWidth(FACTOR);

    // find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::Mat cvImage(m_nbRows, m_nbColumns, CV_8UC1, region.mask, m_nbColumns);
    cv::findContours(cvImage, contours, cv::RetrievalModes::RETR_EXTERNAL, cv::ContourApproximationModes::CHAIN_APPROX_SIMPLE);

    for (auto contour : contours) {
      if (contour.size() >= 3) {
        region.valid = true;
        QVector<QPoint> qContour;
        for (auto cvPoint : contour) {
          qContour.push_back(QPoint(cvPoint.x * FACTOR, cvPoint.y * FACTOR));
        }
        qContour.push_back(*qContour.begin());
        region.qContours.push_back(qContour);
      }
    }
  }
}
