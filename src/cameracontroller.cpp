// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "cameracontroller.h"

#include "camera.h"

#include <QTimerEvent>

CameraController::CameraController(QObject* parent) : QObject(parent)
{

}

CameraController::CameraController(Camera& camera) : QObject(&camera),
  m_camera(&camera)
{

}

void CameraController::setCamera(Camera* cam)
{
  if (m_camera != cam)
  {
    m_camera = cam;
    Q_EMIT cameraChanged();
  }
}

void CameraController::mousePressEvent(QMouseEvent* e, Viewport*) {}

void CameraController::mouseMoveEvent(QMouseEvent* e, Viewport*) {}

void CameraController::mouseReleaseEvent(QMouseEvent* e, Viewport*) {}

void CameraController::hoverMoveEvent(QHoverEvent* e, Viewport*) {}

void CameraController::keyPressEvent(QKeyEvent* e, Viewport*) {}

void CameraController::keyReleaseEvent(QKeyEvent* e, Viewport*) {}

void CameraController::wheelEvent(QWheelEvent* e, Viewport*) {}

void CameraController::startMovement()
{
  if (m_movement_timerid == -1)
  {
    m_movement_timerid = startTimer(16);
    m_movement_elapsedtimer.restart();
  }
}

void CameraController::update(qint64 /* elapsed */)
{

}

void CameraController::endMovement()
{
  if (m_movement_timerid != -1)
  {
    killTimer(m_movement_timerid);
    m_movement_timerid = -1;
  }
}

void CameraController::timerEvent(QTimerEvent* ev)
{
  if (ev->timerId() == m_movement_timerid)
  {
    update(m_movement_elapsedtimer.elapsed());

    if (m_movement_timerid != -1)
    {
      m_movement_elapsedtimer.restart();
    }
  }
}

#include "moc_cameracontroller.cpp"
