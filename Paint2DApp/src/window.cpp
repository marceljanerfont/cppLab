#include "widget.h"
#include "window.h"

#include <QGridLayout>
#include <QLabel>
#include <QTimer>

Window::Window():
  //m_helper("C:/PROJECTS/SEGMENTATION/output_samples/704x576/analytics_mask3.bin", 704, 576) {
  m_helper("d:/segmentations/segmentation_16.bin", 1920, 1080) {

  setWindowTitle(m_helper.getFilename());

  Widget *native = new Widget(&m_helper, this);

  QGridLayout *layout = new QGridLayout;
  layout->addWidget(native, 0, 0);

  setLayout(layout);
}
