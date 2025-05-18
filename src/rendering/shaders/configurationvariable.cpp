// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "configurationvariable.h"

namespace glsl
{

QByteArray replace_variables(QByteArray src, const ConfigurationVariables& vars)
{
  for (const auto& p : vars)
    src.replace("${" + p.first + "}", p.second);

  return src;
}

} // namespace glsl

