// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "preprocessordefine.h"

#include <tuple>

namespace glsl
{

bool operator==(const PreprocessorDefine& lhs, const PreprocessorDefine& rhs)
{
  return std::forward_as_tuple(lhs.name, lhs.value) == std::forward_as_tuple(rhs.name, rhs.value);
}

bool operator!=(const PreprocessorDefine& lhs, const PreprocessorDefine& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const PreprocessorDefine& lhs, const PreprocessorDefine& rhs)
{
  return std::forward_as_tuple(lhs.name, lhs.value) < std::forward_as_tuple(rhs.name, rhs.value);
}

QByteArray print(const PreprocessorDefines& defines)
{
  const QByteArray define{ "#define" };
  int size_estimate = 0;

  for (const auto& pd : defines)
  {
    size_estimate += define.size() + pd.name.size() + pd.value.size();
    // two spaces and a line feed
    size_estimate += 3;
  }

  QByteArray str;
  str.reserve(size_estimate);

  for (const auto& pd : defines)
  {
    str += define;
    str.push_back(' ');
    str += pd.name;
    str.push_back(' ');
    str += pd.value;
    str.push_back('\n');
  }

  return str;
}

/**
 * @brief injects a set of define at the top of a shader
 * @param src      the source code of the shader
 * @param defines  the defines
 *
 * If a #version directive is present in the source code, the #defines are inserted
 * right after it.
 */
QByteArray inject_defines(QByteArray src, const PreprocessorDefines& defines)
{
  if (defines.empty())
    return src;

  QByteArray inserted = print(defines);

  // It is expected that the shader source code will contain a #version directive at the
  // very first line, but this is not guaranteed.
  int index = src.indexOf("#version");

  // The #defines will be inserted at the top of the file, after the #version directive (if any)
  index = src.indexOf('\n', index + 1) + 1;

  return src.insert(index, inserted);
}

} // namespace glsl
