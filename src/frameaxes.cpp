// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "frameaxes.h"

#include "coloredvertex.h"

FrameAxes::FrameAxes()
{

}

FrameAxes::~FrameAxes()
{
  releaseResources();
}

QRect FrameAxes::computeAxesViewport(const QRect& sceneViewport) const
{
  return computeAxesViewport(sceneViewport, m_max_width);
}

QRect FrameAxes::computeAxesViewport(const QRect& sceneViewport, int maxWidth)
{
  int width = 0.08 * sceneViewport.width();
  width = std::min(maxWidth, width);
  int height = width;
  int margin = 0.02 * sceneViewport.width();
  return QRect(sceneViewport.width() + sceneViewport.x() - width - margin, sceneViewport.y() + margin, width, height);
}

void FrameAxes::drawWorldFrameAxes(QOpenGLFunctions* gl, const QMatrix4x4& projectionMatrix, const QMatrix4x4& viewMatrix)
{
  QOpenGLVertexArrayObject& vao = get_vao(gl);
  vao.bind();

  QOpenGLShaderProgram& shader_program = get_shader_program();

  shader_program.bind();

  shader_program.setUniformValue("model_view_matrix", viewMatrix);
  shader_program.setUniformValue("projection_matrix", projectionMatrix);
  shader_program.setUniformValue("opacity", 1.f);

  gl->glDrawArrays(GL_LINES, 0, 6);

  shader_program.release();

  vao.release();
}

void FrameAxes::drawCameraOrientationAxes(QOpenGLFunctions* gl, const QRect& sceneViewport, const QSize& windowSize, QMatrix4x4 cameraViewMatrix)
{
  // We erase the 'translation' component of the view matrix, making it a rotation matrix.
  cameraViewMatrix(0, 3) = 0;
  cameraViewMatrix(1, 3) = 0;
  cameraViewMatrix(2, 3) = 0;

  {
    QRect viewport = computeAxesViewport(sceneViewport);
    GLint gl_y = windowSize.height() - viewport.height() - viewport.y();
    gl->glViewport(viewport.x(), gl_y, viewport.width(), viewport.height());
  }

  QOpenGLVertexArrayObject& vao = get_vao(gl);
  vao.bind();

  QOpenGLShaderProgram& shader_program = get_shader_program();

  shader_program.bind();

  shader_program.setUniformValue("model_view_matrix", cameraViewMatrix);
  shader_program.setUniformValue("projection_matrix", QMatrix4x4());
  shader_program.setUniformValue("opacity", 1.f);

  gl->glDrawArrays(GL_LINES, 0, 6);

  shader_program.release();

  vao.release();
}

void FrameAxes::releaseResources()
{
  m_shader_program.reset();
  m_buffer.reset();
  m_vao.reset();
}

QOpenGLVertexArrayObject& FrameAxes::get_vao(QOpenGLFunctions* gl)
{
  if (m_vao)
    return *m_vao;

  m_vao = std::make_unique<QOpenGLVertexArrayObject>();
  m_vao->create();

  m_vao->bind();

  m_buffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);

  m_buffer->create();
  assert(m_buffer->isCreated());

  m_buffer->bind();
  m_buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);

  std::vector<ColoredVertex> vertexdata{
    { QVector3D(0, 0, 0), RgbColor(255, 0, 0) },
    { QVector3D(1, 0, 0), RgbColor(255, 0, 0) },
    { QVector3D(0, 0, 0), RgbColor(0, 255, 0) },
    { QVector3D(0, 1, 0), RgbColor(0, 255, 0) },
    { QVector3D(0, 0, 0), RgbColor(0, 0, 255) },
    { QVector3D(0, 0, 1), RgbColor(0, 0, 255) },
  };

  m_buffer->allocate(vertexdata.data(), static_cast<int>(vertexdata.size() * sizeof(ColoredVertex)));

  gl->glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(ColoredVertex), nullptr);
  gl->glVertexAttribPointer(2, 3, GL_UNSIGNED_BYTE, false, sizeof(ColoredVertex), (void*)sizeof(QVector3D));

  m_buffer->release();

  gl->glEnableVertexAttribArray(0);
  gl->glEnableVertexAttribArray(2);

  m_vao->release();

  return *m_vao;
}

QOpenGLShaderProgram& FrameAxes::get_shader_program()
{
  if (m_shader_program)
    return *m_shader_program;

  m_shader_program = std::make_unique<QOpenGLShaderProgram>();

  m_shader_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/shaders/axes.vert"));
  m_shader_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/shaders/axes.frag"));

  m_shader_program->link();

  return *m_shader_program;
}
