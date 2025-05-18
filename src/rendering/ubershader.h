// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef UBERSHADER_H
#define UBERSHADER_H

#include "shaderprogram.h"
#include "shaderprogramcache.h"

#include <set>

struct UberShaderGetInstanceResult
{
  std::shared_ptr<QOpenGLShaderProgram> shader_program;
  bool cached;
  QStringList missing_vars;
  QStringList unused_vars;
  QString error;
};

class UberShader
{
public:
  UberShader();

  UberShader(const QString& vertexFilePath, const QString& fragmentFilePath);

  const QString& vertexShaderFilePath() const;
  void setVertexShaderFilePath(const QString& path);

  const QString& fragmentShaderFilePath() const;
  void setFragmentShaderFilePath(const QString& path);

  const QByteArray& vertexShaderSourceCode() const;
  const QByteArray& fragmentShaderSourceCode() const;

  const std::set<QByteArray>& configurationVariables() const;

  std::shared_ptr<QOpenGLShaderProgram> getProgram(glsl::PreprocessorDefines defines,
                                                    glsl::ConfigurationVariables vars);

  UberShaderGetInstanceResult getInstance(glsl::PreprocessorDefines defines,
                                                    glsl::ConfigurationVariables vars);


  ShaderProgramCache* cache() const;
  void clearCache();
  void resetCache(std::unique_ptr<ShaderProgramCache> cache);

protected:
  void refreshConfigurationVariables();

private:
  std::unique_ptr<ShaderProgramCache> m_cache;
  QString m_vertex_shader_filepath;
  QString m_fragment_shader_filepath;
  QByteArray m_vertex_shader_src;
  QByteArray m_fragment_shader_src;
  std::set<QByteArray> m_configuration_variables;
};

#endif // UBERSHADER_H
