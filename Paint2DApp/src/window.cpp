#include "widget.h"
#include "window.h"

#include <QGridLayout>
#include <QLabel>
#include <QTimer>

//! [0]
Window::Window(): m_helper("C:/PROJECTS/SEGMENTATION/output_samples/704x576/analytics_mask3.bin", 704, 576) {

  setWindowTitle(m_helper.getFilename());

  Widget *native = new Widget(&m_helper, this);

  //QLabel *nativeLabel = new QLabel(tr("Native"));
  //nativeLabel->setAlignment(Qt::AlignHCenter);

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(native, 0, 0);

  //layout->addWidget(nativeLabel, 1, 0);

  setLayout(layout);

  //QTimer *timer = new QTimer(this);
  //connect(timer, &QTimer::timeout, native, &Widget::animate);
  //timer->start(50);
}
//! [0]
