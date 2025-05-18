// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "orbitalcamera.h"

#include "viewport.h"

#include <QWidget>

#include <QDebug>

OrbitalCameraController::OrbitalCameraController(QObject* parent) : CameraController(parent)
{

}

bool OrbitalCameraController::xAxisInverted() const
{
  return m_invert_xaxis;
}

bool OrbitalCameraController::yAxisInverted() const
{
  return m_invert_yaxis;
}

void OrbitalCameraController::invertXAxis(bool on)
{
  m_invert_xaxis = on;
}

void OrbitalCameraController::invertYAxis(bool on)
{
  m_invert_yaxis = on;
}

float OrbitalCameraController::rotationSpeed() const
{
  return m_rotation_speed;
}

void OrbitalCameraController::setRotationSpeed(float rs)
{
  m_rotation_speed = rs;
}

float OrbitalCameraController::zoomSpeed() const
{
  return m_zoom_speed;
}

void OrbitalCameraController::setZoomSpeed(float zs)
{
  m_zoom_speed = zs;
}

QVector3D OrbitalCameraController::target() const
{
  return m_target_pos;
}

void OrbitalCameraController::setTarget(const QVector3D& pos)
{
  if (m_target_pos != pos)
  {
    m_target_pos = pos;
    Q_EMIT targetChanged();

    if (camera())
    {
      translateWorld(*camera(), camera()->viewCenter() - target());
    }
  }
}

void OrbitalCameraController::setCamera(Camera* cam)
{
  CameraController::setCamera(cam);

  if (cam)
  {
    translateWorld(*cam, cam->viewCenter() - target());
  }
}

void OrbitalCameraController::update(qint64 elapsed)
{
  { // keyboard rotate
    QPoint delta{};
    delta += m_keyboard_movement_flags & RRIGHT ? QPoint(-1, 0) : QPoint(0, 0);
    delta += m_keyboard_movement_flags & RLEFT ? QPoint(1, 0) : QPoint(0, 0);
    delta += m_keyboard_movement_flags & RDOWN ? QPoint(0, -1) : QPoint(0, 0);
    delta += m_keyboard_movement_flags & RUP ? QPoint(0, 1) : QPoint(0, 0);

    delta.rx() *= (xAxisInverted() ? -1 : 1);
    delta.ry() *= (yAxisInverted() ? -1 : 1);

    if (delta.x() != 0 || delta.y() != 0)
    {
      // TODO: what is this 0.001f ?
      float keybard_rotation_speed = -1.f * rotationSpeed() * rad2deg(0.001f) * static_cast<float>(elapsed);
      rotateCamera(delta.x() * keybard_rotation_speed, delta.y() * keybard_rotation_speed);
    }
  }

  {
    const float zoom_coeff
      = (m_keyboard_movement_flags & ZOOMIN ? 1.f : 0.f) + (m_keyboard_movement_flags & ZOOMOUT ? -1.f : 0.f);
    zoom(zoom_coeff * zoomSpeed() * 1.e-3f * elapsed);
  }
}

void OrbitalCameraController::rotateCamera(const QPoint& mouse_delta, const QSize& viewportSize)
{
  rotateCamera(-mouse_delta.x() * 360.f / viewportSize.width(),
               -mouse_delta.y() * 180.f / viewportSize.height());
}

void OrbitalCameraController::rotateCamera(float yaw, float pitch)
{
  yawAboutViewCenter(*camera(), yaw);

  float a = std::acos(QVector3D::dotProduct(camera()->forwardVector(), QVector3D(0, 0, 1)));
  float old_pitch = rad2deg(a);
  float new_pitch = old_pitch - pitch;

  if (new_pitch >= 0.0f && new_pitch <= 180.0f)
  {
    pitchAboutViewCenter(*camera(), pitch);
  }
}

void OrbitalCameraController::zoom(float amount)
{
  constexpr float mindist = 0.1f;
  if (camera()->distanceToViewCenter() - amount > mindist)
  {
    constexpr bool move_view_center = false;
    translate(*camera(), QVector3D(0.f, 0.f, amount), move_view_center);
  }
}

void OrbitalCameraController::wheelZoom(float step) // TODO: change to int ?
{
  float fwd_length = camera()->distanceToViewCenter();
  // +/- 10% for each step
  float new_length = fwd_length * std::pow(1.1f, step);
  float tr = fwd_length - new_length;

  zoom(tr);
}

void OrbitalCameraController::mousePressEvent(QMouseEvent* ev, Viewport* viewport)
{
  if ((ev->buttons() & Qt::RightButton) || (ev->buttons() & Qt::MiddleButton))
  {
    m_previous_move_pos = ev->pos();
    ev->accept();
  }
}

void OrbitalCameraController::mouseMoveEvent(QMouseEvent* ev, Viewport* viewport)
{
  if (m_previous_move_pos.has_value() && ev->buttons() == Qt::RightButton)
  {
    QPoint mouse_delta = ev->pos() - m_previous_move_pos.value();
    rotateCamera(mouse_delta, viewport->rect().size());
    m_previous_move_pos = ev->pos();
  }
}

void OrbitalCameraController::mouseReleaseEvent(QMouseEvent* ev, Viewport* viewport)
{
  m_previous_move_pos.reset();
}

void OrbitalCameraController::keyPressEvent(QKeyEvent* ev, Viewport* viewport)
{
  if (ev->modifiers() & Qt::ShiftModifier)
  {
    if (ev->key() == Qt::Key_Left)
      m_keyboard_movement_flags |= RLEFT;
    else if (ev->key() == Qt::Key_Right)
      m_keyboard_movement_flags |= RRIGHT;
    else if (ev->key() == Qt::Key_Up)
      m_keyboard_movement_flags |= RUP;
    else if (ev->key() == Qt::Key_Down)
      m_keyboard_movement_flags |= RDOWN;
  }
  
  if (ev->key() & Qt::AltModifier)
  {
    if (ev->key() == Qt::Key_Up)
      m_keyboard_movement_flags |= ZOOMIN;
    else if (ev->key() == Qt::Key_Down)
      m_keyboard_movement_flags |= ZOOMOUT;
  }

  if (m_keyboard_movement_flags)
  {
    startMovement();
  }
}

void OrbitalCameraController::keyReleaseEvent(QKeyEvent* ev, Viewport* viewport)
{
  if (ev->key() == Qt::Key_Left)
    m_keyboard_movement_flags &= ~RLEFT;
  else if (ev->key() == Qt::Key_Right)
    m_keyboard_movement_flags &= ~RRIGHT;
  else if (ev->key() == Qt::Key_Up)
    m_keyboard_movement_flags &= ~(RUP | ZOOMIN);
  else if (ev->key() == Qt::Key_Down)
    m_keyboard_movement_flags &= ~(RDOWN | ZOOMOUT);

  if (!m_keyboard_movement_flags)
  {
    endMovement();
  }
}

void OrbitalCameraController::wheelEvent(QWheelEvent* event, Viewport* viewport)
{
  int wheel_angle = event->angleDelta().y();

  wheelZoom(-wheel_angle / 120.f);
}
