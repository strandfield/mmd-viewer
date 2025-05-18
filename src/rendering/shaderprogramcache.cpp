// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "shaderprogramcache.h"

#include <tuple>

std::shared_ptr<QOpenGLShaderProgram> ShaderProgramCache::findProgram(const ShaderProgramConfig& conf) const
{
  auto it = m_cache.find(conf);

  if (it != m_cache.end())
    return it->second;
  else
    return nullptr;
}

std::shared_ptr<QOpenGLShaderProgram> ShaderProgramCache::getProgram(
  const QString& vert,
  const QString& geom,
  const QString& frag,
  glsl::PreprocessorDefines defines,
  glsl::ConfigurationVariables vars)
{
  ShaderProgramConfig conf;
  conf.vert = vert;
  conf.geom = geom;
  conf.frag = frag;
  conf.defines = std::move(defines);
  conf.variables = std::move(vars);

  return getProgram(conf);
}

std::shared_ptr<QOpenGLShaderProgram> ShaderProgramCache::getProgram(
  const ShaderProgramConfig& conf)
{
  std::shared_ptr<QOpenGLShaderProgram> sp = findProgram(conf);

  if (sp)
    return sp;

  sp = std::make_shared<QOpenGLShaderProgram>(glsl::compile_shader(conf).release());

  insert(conf, sp);

  return sp;
}

void ShaderProgramCache::insert(const ShaderProgramConfig& conf, std::shared_ptr<QOpenGLShaderProgram> sp)
{
  m_cache[conf] = sp;
}

void ShaderProgramCache::clear()
{
  m_cache.clear();
}

