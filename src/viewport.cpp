// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "viewport.h"

Viewport::Viewport(QObject* parent)
    : QObject(parent)
{
  setCamera(new Camera(this));
  setFrustum(new ViewFrustum(this));
  frustum()->setFarPlane(10000);
}

Viewport::~Viewport() {}

const QRect& Viewport::rect() const
{
  return m_rect;
}

void Viewport::setRect(const QRect& rect)
{
  if (m_rect != rect)
  {
    m_rect = rect;
    Q_EMIT rectChanged();

    updateFrustumAspectRatio();
  }
}

Camera* Viewport::camera() const
{
  return m_camera;
}

void Viewport::setCamera(Camera* camera)
{
  if (m_camera != camera)
  {
    m_camera = camera;
    Q_EMIT cameraChanged();
  }
}

ViewFrustum* Viewport::frustum() const
{
  return m_frustum;
}

void Viewport::setFrustum(ViewFrustum* f)
{
  if (m_frustum != f)
  {
    m_frustum = f;

    updateFrustumAspectRatio();

    Q_EMIT frustumChanged();
  }
}

void Viewport::updateFrustumAspectRatio()
{
  if (m_frustum)
  {
    m_frustum->setAspectRatio(m_rect.width() / float(m_rect.height()));
  }
}
