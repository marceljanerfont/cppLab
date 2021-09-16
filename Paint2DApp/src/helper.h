#ifndef HELPER_H
#define HELPER_H

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QWidget>

#include <opencv2/core/types.hpp>


struct region {
  bool valid{false};
  char id;
  char *mask; // pixel mask: 0 or 255
  std::vector<QVector<QPoint>> qContours;
  QPen penMask;
  QPen penContour;
};

class Helper {
 public:
  Helper(const std::string &filename, const int  &width, const int &height);
  ~Helper();

 public:
  void paint(QPainter *painter, QPaintEvent *event, int elapsed);

  int getWidth() {
    return m_width;
  };
  int getHeight() {
    return m_height;
  };

  QString getFilename() {
    return QString::fromStdString(m_filename);
  }

 private:
  void loadSegmentation();
  void setPixelPen(char id, QPainter *painter);
  QColor getRandomColor();
  QBrush background;


  QFont textFont;
  QPen circlePen;
  QPen textPen;

  QPen m_blackPen;
  QPen m_redPen;

  //const size_t IMG_WIDTH{1920 / 4};
  //const size_t IMG_HEIGHT{1080 / 4};

  //const size_t IMG_WIDTH{1920};
  //const size_t IMG_HEIGHT{1080};

  const int FACTOR{4};

  std::string m_filename;
  int m_width;
  int m_height;
  int m_nbColumns; // = m_width / FACTOR
  int m_nbRows;    // = m_height / FACTOR

  char **m_segmentation2;
  char *m_segmentation;


  static const size_t MAX_PENS{256};
  QPen m_pens[MAX_PENS];

  std::map<char, region> m_regions;
};

#endif
