// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "viewfrustum.h"

ViewFrustum::ViewFrustum(QObject* parent) : QObject(parent)
{

}

ViewFrustum::~ViewFrustum()
{

}

ViewFrustum::ViewFrustum(FrustumType type, float near_plane, float far_plane, float vertical_angle, QObject* parent) : QObject(parent)
  , m_near_plane(near_plane)
  , m_far_plane(far_plane)
  , m_vertical_angle(vertical_angle)
{

}

void ViewFrustum::setType(FrustumType t)
{
  if (m_type != t)
  {
    m_type = t;
    Q_EMIT typeChanged();
    Q_EMIT projectionMatrixChanged();
  }
}

void ViewFrustum::setNearPlane(float near)
{
  if (!qFuzzyCompare(near, m_near_plane))
  {
    m_near_plane = near;
    Q_EMIT nearPlaneChanged();
    Q_EMIT projectionMatrixChanged();
  }
}

void ViewFrustum::setFarPlane(float far)
{
  if (!qFuzzyCompare(far, m_far_plane))
  {
    m_far_plane = far;
    Q_EMIT farPlaneChanged();
    Q_EMIT projectionMatrixChanged();
  }
}

void ViewFrustum::setVerticalAngle(float angle)
{
  if (!qFuzzyCompare(angle, m_vertical_angle))
  {
    m_vertical_angle = angle;
    Q_EMIT verticalAngleChanged();
    Q_EMIT projectionMatrixChanged();
  }
}

void ViewFrustum::setAspectRatio(float ar)
{
  if (!qFuzzyCompare(ar, m_aspect_ratio))
  {
    m_aspect_ratio = ar;
    Q_EMIT aspectRatioChanged();
    Q_EMIT projectionMatrixChanged();
  }
}

void ViewFrustum::setHeight(float h)
{
  if (nearPlane() != 0)
  {
    float va = ::rad2deg(2.f * std::atan(0.5f * h / nearPlane()));

    setVerticalAngle(va);
  }
}
