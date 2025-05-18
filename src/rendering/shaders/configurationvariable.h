// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef GLSL_CONFIGURATIONVARIABLE_H
#define GLSL_CONFIGURATIONVARIABLE_H

#include <QByteArray>

#include <map>

namespace glsl
{

using ConfigurationVariables = std::map<QByteArray, QByteArray>;

QByteArray replace_variables(QByteArray src, const ConfigurationVariables& vars);

} // namespace glsl

#endif // GLSL_CONFIGURATIONVARIABLE_H
