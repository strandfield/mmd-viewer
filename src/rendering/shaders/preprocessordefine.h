// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef GLSL_PREPROCESSORDEFINE_H
#define GLSL_PREPROCESSORDEFINE_H

#include <QByteArray>

#include <vector>

namespace glsl
{

class PreprocessorDefine
{
public:
  QByteArray name;
  QByteArray value;

public:
  explicit PreprocessorDefine(const QByteArray& n, const QByteArray& v = {})
    : name(n)
    , value(v)
  {
  }
};

bool operator==(const PreprocessorDefine& lhs, const PreprocessorDefine& rhs);
bool operator!=(const PreprocessorDefine& lhs, const PreprocessorDefine& rhs);
bool operator<(const PreprocessorDefine& lhs, const PreprocessorDefine& rhs);

using PreprocessorDefines = std::vector<PreprocessorDefine>;

QByteArray print(const PreprocessorDefines& defines);

QByteArray inject_defines(QByteArray src, const PreprocessorDefines& defines);

} // namespace glsl

#endif // GLSL_PREPROCESSORDEFINE_H
