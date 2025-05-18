// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "camera.h"
#include "viewfrustum.h"

#include <QObject>

#include <QMatrix4x4>

#include <cmath>

class Viewport : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QRect rect READ rect WRITE setRect NOTIFY rectChanged)
  Q_PROPERTY(Camera* camera READ camera WRITE setCamera NOTIFY cameraChanged)
  Q_PROPERTY(ViewFrustum* frustum READ frustum WRITE setFrustum NOTIFY frustumChanged)
public:
  explicit Viewport(QObject* parent = nullptr);
  Viewport(const Viewport&) = delete;
  ~Viewport();

  const QRect& rect() const;
  void setRect(const QRect& rect);

  Camera* camera() const;
  void setCamera(Camera* camera);

  ViewFrustum* frustum() const;
  void setFrustum(ViewFrustum* f);

Q_SIGNALS:
  void rectChanged();
  void cameraChanged();
  void frustumChanged();

protected:
  void updateFrustumAspectRatio();

private:
  QRect m_rect;
  Camera* m_camera = nullptr;
  ViewFrustum* m_frustum = nullptr;
};
