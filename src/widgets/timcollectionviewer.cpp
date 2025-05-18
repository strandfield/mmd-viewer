#include "timcollectionviewer.h"

#include <QListWidget>

#include <QHBoxLayout>

TimCollectionViewer::TimCollectionViewer(QWidget* parent)
    : QWidget(parent)
{
  m_list_widget = new QListWidget;
  m_viewer = new TimViewer;

  auto* layout = new QHBoxLayout(this);
  layout->addWidget(m_list_widget);
  layout->addWidget(m_viewer, 1);

  connect(m_list_widget,
          &QListWidget::currentRowChanged,
          this,
          &TimCollectionViewer::onCurrentRowChanged);
}

TimCollectionViewer::~TimCollectionViewer() {}

void TimCollectionViewer::addTim(const QString& name, const TimImage& img)
{
  m_list_widget->addItem(name);
  m_tims.push_back(img);
}

void TimCollectionViewer::onCurrentRowChanged(int row)
{
  if (row != -1)
  {
    m_viewer->setTimImage(m_tims.at(row));
  }
}
