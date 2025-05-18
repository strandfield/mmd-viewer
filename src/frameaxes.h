// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

/**
 * @brief a class that provides rendering of a frame
 *
 * This class can be used to render the world or camera axes.
 * The x,y,z axes are rendered respectively in red, green and blue.
 */
class FrameAxes
{
public:
  FrameAxes();
  ~FrameAxes();

  QRect computeAxesViewport(const QRect& sceneViewport) const;
  static QRect computeAxesViewport(const QRect& sceneViewport, int maxWidth);

  void drawWorldFrameAxes(QOpenGLFunctions* gl, const QMatrix4x4& projectionMatrix, const QMatrix4x4& viewMatrix);
  void drawCameraOrientationAxes(QOpenGLFunctions* gl, const QRect& sceneViewport, const QSize& windowSize, QMatrix4x4 cameraViewMatrix);

  void releaseResources();

protected:
  QOpenGLVertexArrayObject& get_vao(QOpenGLFunctions* gl);
  QOpenGLShaderProgram& get_shader_program();

private:
  int m_max_width = 90;
  std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
  std::unique_ptr<QOpenGLBuffer> m_buffer;
  std::unique_ptr<QOpenGLShaderProgram> m_shader_program;
};
