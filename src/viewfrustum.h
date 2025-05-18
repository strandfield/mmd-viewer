// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef VIEWFRUSTUM_H
#define VIEWFRUSTUM_H

#include "angles.h"

#include <QObject>

#include <QMatrix4x4>

#include <cmath>

/**
 * @brief a view frustum used to render a GL scene
 *
 * While the Camera defines where the camera looks and from which position,
 * the view frustum, roughly speaking, specifies how far and wide the camera
 * sees.
 *
 * @sa class Camera.
 */
class ViewFrustum : public QObject
{
  Q_OBJECT

  Q_PROPERTY(FrustumType type READ type WRITE setType NOTIFY typeChanged)
  Q_PROPERTY(float nearPlane READ nearPlane WRITE setNearPlane NOTIFY nearPlaneChanged)
  Q_PROPERTY(float farPlane READ farPlane WRITE setFarPlane NOTIFY farPlaneChanged)
  Q_PROPERTY(float verticalAngle READ verticalAngle WRITE setVerticalAngle NOTIFY verticalAngleChanged)
  Q_PROPERTY(float aspectRatio READ aspectRatio NOTIFY aspectRatioChanged)
public:
  explicit ViewFrustum(QObject* parent = nullptr);
  ViewFrustum(const ViewFrustum&) = delete;
  ~ViewFrustum();

  enum FrustumType
  {
    Perspective,
    Ortho
  };
  Q_ENUM(FrustumType);

  explicit ViewFrustum(FrustumType type,
                       float near_plane = 1.f,
                       float far_plane = 1000.f,
                       float vertical_angle = 45.f,
                       QObject* parent = nullptr);

  FrustumType type() const;
  void setType(FrustumType t);
  bool isOrthographic() const;

  float nearPlane() const;
  void setNearPlane(float near);

  float farPlane() const;
  void setFarPlane(float far);

  float verticalAngle() const;
  void setVerticalAngle(float angle);

  float aspectRatio() const;
  void setAspectRatio(float ar);

  float height() const;
  void setHeight(float h);

Q_SIGNALS:
  void typeChanged();
  void nearPlaneChanged();
  void farPlaneChanged();
  void verticalAngleChanged();
  void aspectRatioChanged();
  void projectionMatrixChanged();

private:
  FrustumType m_type;
  float m_near_plane = 1.f;
  float m_far_plane = 600.f;
  float m_vertical_angle = 45.f;
  float m_aspect_ratio = 1.f;
  float m_height = 1.f;
};

inline ViewFrustum::FrustumType ViewFrustum::type() const
{
  return m_type;
}

inline bool ViewFrustum::isOrthographic() const
{
  return type() == FrustumType::Ortho;
}

inline float ViewFrustum::nearPlane() const
{
  return m_near_plane;
}

inline float ViewFrustum::farPlane() const
{
  return m_far_plane;
}

inline float ViewFrustum::verticalAngle() const
{
  return m_vertical_angle;
}

inline float ViewFrustum::aspectRatio() const
{
  return m_aspect_ratio;
}

inline float ViewFrustum::height() const
{
  return 2 * nearPlane() * std::tan(::deg2rad(verticalAngle() * 0.5f));
}

inline QMatrix4x4 projectionMatrix(const ViewFrustum& frustum)
{
  QMatrix4x4 r;

  if (frustum.type() == ViewFrustum::Ortho)
  {
    float h = frustum.height() / 2;
    float w = h * frustum.aspectRatio();
    r.ortho(-w, w, -h, h, frustum.nearPlane(), frustum.farPlane());
  }
  else
  {
    r.perspective(frustum.verticalAngle(), frustum.aspectRatio(), frustum.nearPlane(), frustum.farPlane());
  }

  return r;
}

#endif // VIEWFRUSTUM_H
