// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "ubershader.h"

UberShader::UberShader() :
  m_cache(std::make_unique<ShaderProgramCache>())
{

}

UberShader::UberShader(const QString& vertexFilePath, const QString& fragmentFilePath) :
  m_cache(std::make_unique<ShaderProgramCache>()),
  m_vertex_shader_filepath(vertexFilePath),
  m_fragment_shader_filepath(fragmentFilePath)
{
  m_vertex_shader_src = glsl::load_source_from_file(vertexFilePath);
  m_fragment_shader_src = glsl::load_source_from_file(fragmentFilePath);

  refreshConfigurationVariables();
}

const QString& UberShader::vertexShaderFilePath() const
{
  return m_vertex_shader_filepath;
}

void UberShader::setVertexShaderFilePath(const QString& path)
{
  if (m_vertex_shader_filepath != path)
  {
    m_vertex_shader_filepath = path;
    m_vertex_shader_src = glsl::load_source_from_file(m_vertex_shader_filepath);
    refreshConfigurationVariables();
  }
}

const QString& UberShader::fragmentShaderFilePath() const
{
  return m_fragment_shader_filepath;
}

void UberShader::setFragmentShaderFilePath(const QString& path)
{
  if (m_fragment_shader_filepath != path)
  {
    m_fragment_shader_filepath = path;
    m_fragment_shader_src = glsl::load_source_from_file(m_fragment_shader_filepath);
    refreshConfigurationVariables();
  }
}

const QByteArray& UberShader::vertexShaderSourceCode() const
{
  return m_vertex_shader_src;
}

const QByteArray& UberShader::fragmentShaderSourceCode() const
{
  return m_fragment_shader_src;
}

const std::set<QByteArray>& UberShader::configurationVariables() const
{
  return m_configuration_variables;
}

namespace
{

std::unique_ptr<QOpenGLShaderProgram> compile_shader(const QByteArray& vertex, 
  const QByteArray& fragment, 
  const glsl::PreprocessorDefines& defines,
  const glsl::ConfigurationVariables& vars)
{
  auto sp = std::make_unique<QOpenGLShaderProgram>();

  sp->addShaderFromSourceCode(QOpenGLShader::Vertex,
                              glsl::prepare_shader(vertex, defines, vars));
  sp->addShaderFromSourceCode(QOpenGLShader::Fragment,
                              glsl::prepare_shader(fragment, defines, vars));

  sp->link();

  return sp;
}

}

std::shared_ptr<QOpenGLShaderProgram> UberShader::getProgram(glsl::PreprocessorDefines defines,
                                                             glsl::ConfigurationVariables vars)
{
  auto result = getInstance(std::move(defines), std::move(vars));
  return result.shader_program;
}

UberShaderGetInstanceResult UberShader::getInstance(glsl::PreprocessorDefines defines,
                                                    glsl::ConfigurationVariables vars)
{
  UberShaderGetInstanceResult result;

  ShaderProgramConfig conf;
  conf.vert = vertexShaderFilePath();
  conf.frag = fragmentShaderFilePath();
  conf.variables = std::move(vars);
  conf.defines = std::move(defines);

  if (cache())
  {
    if (std::shared_ptr<QOpenGLShaderProgram> sp = cache()->findProgram(conf))
    {
      result.cached = true;
      result.shader_program = sp;
      return result;
    }
  }

  for (const QByteArray& confvar : configurationVariables())
  {
    if (conf.variables.find(confvar) == conf.variables.end())
    {
      result.missing_vars.push_back(QString::fromLatin1(confvar));
    }
  }

  for (const auto& p : conf.variables)
  {
    if (configurationVariables().find(p.first) == configurationVariables().end())
    {
      result.unused_vars.push_back(QString::fromLatin1(p.first));
    }
  }

  result.cached = false;
  auto sp = std::make_shared<QOpenGLShaderProgram>();

  sp->addShaderFromSourceCode(QOpenGLShader::Vertex,
                              glsl::prepare_shader(vertexShaderSourceCode(), conf.defines, conf.variables));
  sp->addShaderFromSourceCode(QOpenGLShader::Fragment,
                              glsl::prepare_shader(fragmentShaderSourceCode(), conf.defines, conf.variables));

  if (sp->link())
  {
    result.shader_program = sp;

    if (cache())
    {
      cache()->insert(conf, sp);
    }
  }
  else
  {
    result.error = sp->log();
  }

  return result;
}

ShaderProgramCache* UberShader::cache() const
{
  return m_cache.get();
}

void UberShader::clearCache()
{
  if (cache())
  {
    cache()->clear();
  }
}

void UberShader::resetCache(std::unique_ptr<ShaderProgramCache> cache)
{
  m_cache = std::move(cache);
}

namespace
{

void collect_configuration_variables(std::set<QByteArray>& set, const QByteArray& src)
{
  int n = src.indexOf("${");

  while (n != -1)
  {
    int m = src.indexOf('}', n + 2);

    if (m != -1)
    {
      QByteArray varname = src.mid(n + 2, m - (n + 2));

      if (!varname.isEmpty())
      {
        set.insert(varname);
      }
    }

    n = src.indexOf("${", n + 2);
  }
}

}

void UberShader::refreshConfigurationVariables()
{
  m_configuration_variables.clear();
  collect_configuration_variables(m_configuration_variables, m_vertex_shader_src);
  collect_configuration_variables(m_configuration_variables, m_fragment_shader_src);
}
