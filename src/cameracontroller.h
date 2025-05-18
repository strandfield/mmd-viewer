// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>

#include <QElapsedTimer>

#include <QQuaternion>
#include <QVector3D>

#include <QMouseEvent>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QWheelEvent>

class Camera;
class Viewport;

/**
 * @brief base class for controlling a camera through user inputs
 *
 * This is a base class that does not respond to user inputs.
 * Actual event handling is implemented differently by various subclasses.
 */
class CameraController : public QObject
{
  Q_OBJECT
  Q_PROPERTY(Camera* camera READ camera NOTIFY cameraChanged)
public:
  explicit CameraController(QObject* parent = nullptr);

  explicit CameraController(Camera& camera); // TODO: maybe should be installed on Viewport?

  Camera* camera() const;
  virtual void setCamera(Camera* cam);

  virtual void mousePressEvent(QMouseEvent* e, Viewport* viewport);
  virtual void mouseMoveEvent(QMouseEvent* e, Viewport* viewport);
  virtual void mouseReleaseEvent(QMouseEvent* e, Viewport* viewport);

  virtual void hoverMoveEvent(QHoverEvent* e, Viewport* viewport);

  virtual void keyPressEvent(QKeyEvent* e, Viewport* viewport);
  virtual void keyReleaseEvent(QKeyEvent* e, Viewport* viewport);

  virtual void wheelEvent(QWheelEvent* e, Viewport* viewport);

Q_SIGNALS:
  void cameraChanged();

protected:
  void startMovement();
  virtual void update(qint64 elapsed);
  void endMovement();
  void timerEvent(QTimerEvent* ev) override;

private:
  Camera* m_camera = nullptr;
  int m_movement_timerid = -1;
  QElapsedTimer m_movement_elapsedtimer;
};

inline Camera* CameraController::camera() const
{
  return m_camera;
}

#endif // CAMERACONTROLLER_H
