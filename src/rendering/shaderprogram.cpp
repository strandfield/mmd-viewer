// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "shaderprogram.h"

#include <QFile>

#include <tuple>

bool operator==(const ShaderProgramConfig& lhs, const ShaderProgramConfig& rhs)
{
  return std::forward_as_tuple(lhs.vert, lhs.geom, lhs.frag, lhs.defines, lhs.variables)
    == std::forward_as_tuple(rhs.vert, rhs.geom, rhs.frag, rhs.defines, rhs.variables);
}

bool operator!=(const ShaderProgramConfig& lhs, const ShaderProgramConfig& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const ShaderProgramConfig& lhs, const ShaderProgramConfig& rhs)
{
  return std::forward_as_tuple(lhs.vert, lhs.geom, lhs.frag, lhs.defines, lhs.variables)
    < std::forward_as_tuple(rhs.vert, rhs.geom, rhs.frag, rhs.defines, rhs.variables);
}

namespace glsl
{

QByteArray load_source_from_file(const QString& filePath)
{
  QFile file{ filePath };
  file.open(QIODevice::ReadOnly);
  return file.readAll();
}

QByteArray prepare_shader(QByteArray src,
                          const glsl::PreprocessorDefines& defines,
                          const glsl::ConfigurationVariables& vars)
{
  src = glsl::inject_defines(src, defines);
  src = glsl::replace_variables(src, vars);
  return src;
}

QByteArray load_shader(const QString& filepath,
                          const glsl::PreprocessorDefines& defines,
                          const glsl::ConfigurationVariables& vars)
{
  QByteArray src = glsl::load_source_from_file(filepath);
  return prepare_shader(std::move(src), defines, vars);
}

std::unique_ptr<QOpenGLShaderProgram> compile_shader(const ShaderProgramConfig& conf)
{
  auto sp = std::make_unique<QOpenGLShaderProgram>();

  sp->addShaderFromSourceCode(QOpenGLShader::Vertex,
                              load_shader(conf.vert, conf.defines, conf.variables));
  sp->addShaderFromSourceCode(QOpenGLShader::Fragment,
                              load_shader(conf.frag, conf.defines, conf.variables));

  if (!conf.geom.isEmpty())
    sp->addShaderFromSourceCode(QOpenGLShader::Geometry,
                                load_shader(conf.geom, conf.defines, conf.variables));

  sp->link();

  return sp;
}

} // namespace glsl
