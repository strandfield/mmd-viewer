#include "tmdviewer.h"

#include "converters/tmd2object3d.h"

#include <QDebug>

TMD_Viewer::TMD_Viewer(QWidget* parent)
    : SceneViewer(parent)
{
}

TMD_Viewer::TMD_Viewer(const TMD_Model& model, QWidget* parent)
    : SceneViewer(parent)
{

  setModel(model);
}

TMD_Viewer::~TMD_Viewer()
{
 
}

const TMD_Model& TMD_Viewer::model() const
{
  return m_model;
}

void TMD_Viewer::setModel(const TMD_Model& model)
{
  m_model = std::move(model);
  m_modelNeedsUpdate = true;
  update();
}

void TMD_Viewer::addTIM(const TimImage& img)
{
  m_tims.push_back(img);
  m_modelNeedsUpdate = true;
  update();
}

void TMD_Viewer::setTIMs(std::vector<TimImage> tims)
{
  m_tims = std::move(tims);
  m_modelNeedsUpdate = true;
  update();
}

void TMD_Viewer::paintGL()
{
  if (std::exchange(m_modelNeedsUpdate, false))
  {
    TMD_ModelConverter converter;
    converter.setTIMs(m_tims);
    auto obj3d = converter.convertModel(m_model);
    sceneRoot().clear();
    sceneRoot().add(std::move(obj3d));
  }

  SceneViewer::paintGL();
}
