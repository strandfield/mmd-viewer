// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "sceneviewer.h"

#include "formats/tim.h"
#include "formats/tmd.h"

class TMD_Viewer : public SceneViewer
{
  Q_OBJECT
public:
  explicit TMD_Viewer(QWidget* parent = nullptr);
  explicit TMD_Viewer(const TMD_Model& model, QWidget* parent = nullptr);
  ~TMD_Viewer();

  const TMD_Model& model() const;
  void setModel(const TMD_Model& model);

  void addTIM(const TimImage& img);
  void setTIMs(std::vector<TimImage> tims);

protected:
  void paintGL() override;

private:
  TMD_Model m_model;
  std::vector<TimImage> m_tims;
  bool m_modelNeedsUpdate = true;
};
