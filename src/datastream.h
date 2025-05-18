// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <iostream>
#include <vector>

template<typename T>
T read(std::istream& stream)
{
  T result;
  stream.read(reinterpret_cast<char*>(&result), sizeof(T));
  return result;
}

template<typename T>
std::vector<T> readvec(std::istream& stream, size_t nElements)
{
  auto result = std::vector<T>(nElements);
  stream.read(reinterpret_cast<char*>(result.data()), nElements * sizeof(T));
  return result;
}
