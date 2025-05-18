// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef SHADERPROGRAMCACHE_H
#define SHADERPROGRAMCACHE_H

#include "shaderprogram.h"

#include <map>
#include <memory>

class ShaderProgramCache
{
public:
  ShaderProgramCache() = default;

  std::shared_ptr<QOpenGLShaderProgram> findProgram(const ShaderProgramConfig& conf) const;

  std::shared_ptr<QOpenGLShaderProgram> getProgram(const QString& vert,
                                                   const QString& geom,
                                                   const QString& frag,
                                                   glsl::PreprocessorDefines defines,
                                                   glsl::ConfigurationVariables vars);

  std::shared_ptr<QOpenGLShaderProgram> getProgram(const ShaderProgramConfig& conf);

  void insert(const ShaderProgramConfig& conf, std::shared_ptr<QOpenGLShaderProgram> sp);

  void clear();

private:
  std::map<ShaderProgramConfig, std::shared_ptr<QOpenGLShaderProgram>> m_cache;
};

#endif // SHADERPROGRAMCACHE_H
