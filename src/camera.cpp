// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "camera.h"

#include "cameracontroller.h"
#include "viewfrustum.h"

QVector3D compute_up_vector(const QVector3D& forward_vector, const QVector3D& up_vector_hint = QVector3D(0, 0, 1))
{
  QVector3D result = QVector3D::crossProduct(QVector3D::crossProduct(forward_vector, up_vector_hint), forward_vector);

  if (qFuzzyIsNull(result.length())) // bad: forward_vector and the up_vector_hint are collinear
  {
    if (!qFuzzyIsNull(up_vector_hint.y()) || !qFuzzyIsNull(up_vector_hint.z()))
    {
      // rotate hint 90 degress along x-axis
      return compute_up_vector(forward_vector, QVector3D(up_vector_hint.x(), up_vector_hint.z(), -up_vector_hint.y()));
    }
    else
    {
      // rotate hint 90 degress along z-axis
      return compute_up_vector(forward_vector, QVector3D(-up_vector_hint.y(), +up_vector_hint.x(), up_vector_hint.z()));
    }
  }

  return result.normalized();
}

Camera::Camera(QObject* parent)
  : QObject(parent)
  , m_position(2, 2, 10)
  , m_view_center(0, 0, 0)
  , m_up_vector(compute_up_vector(m_position.normalized()))
{
}

const QVector3D& Camera::position() const
{
  return m_position;
}

// TODO: shouldn't this function recompute the up vector ?
void Camera::setPosition(const QVector3D& pos)
{
  if (m_position != pos)
  {
    m_position = pos;

    Q_EMIT positionChanged();
    Q_EMIT forwardVectorChanged();
    Q_EMIT viewMatrixChanged();
  }
}

const QVector3D& Camera::viewCenter() const
{
  return m_view_center;
}

// TODO: shouldn't this function recompute the up vector ?
void Camera::setViewCenter(const QVector3D& pos)
{
  if (m_view_center != pos)
  {
    m_view_center = pos;
    Q_EMIT viewCenterChanged();
    Q_EMIT forwardVectorChanged();
    Q_EMIT viewMatrixChanged();
  }
}

void Camera::reset(const QVector3D& position, const QVector3D& view_center, const QVector3D& up_vector)
{
  bool forward_changed = false;

  if (m_position != position)
  {
    forward_changed = true;
    m_position = position;
    Q_EMIT positionChanged();
  }

  if (m_view_center != view_center)
  {
    forward_changed = true;
    m_view_center = view_center;
    Q_EMIT viewCenterChanged();
  }

  bool up_changed = m_up_vector != up_vector;

  if (up_changed)
  {
    m_up_vector = up_vector;
    Q_EMIT upVectorChanged();
  }

  if (forward_changed)
  {
    Q_EMIT forwardVectorChanged();
  }

  if (forward_changed || up_changed)
  {
    Q_EMIT viewMatrixChanged();
  }
}

void Camera::reset(const Camera& other)
{
  reset(other.position(), other.viewCenter(), other.upVector());
}

float Camera::distanceToViewCenter() const
{
  return (viewCenter() - position()).length();
}

QVector3D Camera::forwardVector() const
{
  return (viewCenter() - position()).normalized();
}

QVector3D Camera::upVector() const
{
  return m_up_vector;
}

QVector3D Camera::rightVector() const
{
  return QVector3D::crossProduct(forwardVector(), upVector());
}

QMatrix4x4 Camera::viewMatrix() const
{
  QMatrix4x4 r;
  r.lookAt(position(), viewCenter(), upVector());
  return r;
}

void translate(Camera& camera, const QVector3D& v, bool move_view_center)
{
  // v is expressed in the coordinate system (right, up, forward)

  // Express the translation as a world translation.
  QVector3D v_world = v.x() * camera.rightVector()
    + v.y() * camera.upVector()
    + v.z() * camera.forwardVector();

  auto view_center = camera.viewCenter() + (move_view_center ? v_world : QVector3D());

  QVector3D pos = camera.position() + v_world;

  if (move_view_center)
  {
    camera.reset(pos, view_center, camera.upVector());
  }
  else
  {
    QVector3D up = compute_up_vector((view_center - pos).normalized(), camera.upVector());
    camera.reset(pos, view_center, up);
  }
}

void translateWorld(Camera& camera, const QVector3D& v)
{
  camera.reset(camera.position() - v, camera.viewCenter() - v, camera.upVector());
}

void rotate(Camera& camera, const QQuaternion& q)
{
  auto up = q * camera.upVector();
  auto forward = q * (camera.viewCenter() - camera.position());
  camera.reset(camera.position(), camera.position() + forward, up);
}

void rotateAboutViewCenter(Camera& camera, const QQuaternion& q)
{
  auto up = q * camera.upVector();
  auto forward = q * (camera.viewCenter() - camera.position());
  camera.reset(camera.viewCenter() - forward, camera.viewCenter(), up);
}

void pitch(Camera& camera, float angle)
{
  auto ux = -camera.rightVector();
  auto q = QQuaternion::fromAxisAndAngle(ux, -angle);
  rotate(camera, q);
}

void pitchAboutViewCenter(Camera& camera, float angle)
{
  auto ux = -camera.rightVector();
  auto q = QQuaternion::fromAxisAndAngle(ux, -angle);
  rotateAboutViewCenter(camera, q);
}

void yaw(Camera& camera, float angle, const QVector3D& axis)
{
  auto q = QQuaternion::fromAxisAndAngle(axis, angle);
  rotate(camera, q);
}

void yawAboutViewCenter(Camera& camera, float angle, const QVector3D& axis)
{
  auto q = QQuaternion::fromAxisAndAngle(axis, angle);
  rotateAboutViewCenter(camera, q);
}

#include "moc_camera.cpp"
