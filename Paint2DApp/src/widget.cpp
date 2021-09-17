#include "widget.h"
#include "helper.h"

#include <QPainter>
#include <QTimer>

Widget::Widget(Helper *helper, QWidget *parent)
  : QWidget(parent), helper(helper) {
  elapsed = 0;
  setFixedSize((int)helper->getWidth(), (int)helper->getHeight());
}

void Widget::paintEvent(QPaintEvent *event) {
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);
  helper->paint(&painter, event, elapsed);
  painter.end();
}
