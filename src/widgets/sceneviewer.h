// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "rendering/group.h"

#include <QOpenGLWidget>

#include <memory>

class SceneViewer : public QOpenGLWidget
{
  Q_OBJECT
public:
  explicit SceneViewer(QWidget* parent = nullptr);
  ~SceneViewer();

  Group& sceneRoot() const;

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

private:
  void init();

private:
  struct Data;
  std::unique_ptr<Data> d;
};
