#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

class Helper;

class Widget : public QWidget {
  Q_OBJECT

 public: Widget(Helper *helper, QWidget *parent);

 protected:
  void paintEvent(QPaintEvent *event) override;

 private:
  Helper *helper;
  int elapsed;
};
#endif
