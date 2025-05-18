#include "timviewer.h"

#include "converters/tim2image.h"

#include <QPainter>

TimViewer::TimViewer(QWidget* parent)
    : QWidget(parent)
{}

TimViewer::TimViewer(const TimImage& img, QWidget* parent)
    : QWidget(parent)
{
  setTimImage(img);
}

TimViewer::~TimViewer() {}

const TimImage& TimViewer::getTimImage() const
{
  return m_tim;
}

void TimViewer::setTimImage(const TimImage& img)
{
  m_tim = img;
  std::vector<QImage> images = tim2images(img);
  m_pixmaps.clear();
  m_pixmaps.reserve(images.size());

  for (const QImage& qimg : images)
  {
    m_pixmaps.push_back(QPixmap::fromImage(qimg));
  }

  update();
}

void TimViewer::paintEvent(QPaintEvent* ev)
{
  Q_UNUSED(ev);

  QPainter painter{this};

  int x = 0;
  int y = 0;

  for (const QPixmap& pix : m_pixmaps)
  {
    painter.drawPixmap(x, y, pix);
    x += m_tim.width();
    if (x + m_tim.width() > width())
    {
      x = 0;
      y += m_tim.height();
    }
  }
}
