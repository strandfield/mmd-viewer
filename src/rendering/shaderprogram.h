// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "rendering/shaders/configurationvariable.h"
#include "rendering/shaders/preprocessordefine.h"

#include <QOpenGLShaderProgram>

struct ShaderProgramConfig
{
  QString vert;
  QString geom;
  QString frag;
  glsl::PreprocessorDefines defines;
  glsl::ConfigurationVariables variables;
};

bool operator==(const ShaderProgramConfig& lhs, const ShaderProgramConfig& rhs);
bool operator!=(const ShaderProgramConfig& lhs, const ShaderProgramConfig& rhs);

bool operator<(const ShaderProgramConfig& lhs, const ShaderProgramConfig& rhs);

namespace glsl
{

QByteArray load_source_from_file(const QString& filePath);

QByteArray prepare_shader(QByteArray src,
  const glsl::PreprocessorDefines& defines,
  const glsl::ConfigurationVariables& vars);

QByteArray load_shader(const QString& filepath,
  const glsl::PreprocessorDefines& defines,
  const glsl::ConfigurationVariables& vars);

std::unique_ptr<QOpenGLShaderProgram> compile_shader(const ShaderProgramConfig& conf);

} // namespace glsl

#endif // SHADERPROGRAM_H
