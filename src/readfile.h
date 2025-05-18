// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

inline std::vector<uint8_t> read_all(const std::filesystem::path& filePath)
{
  const int64_t length = std::filesystem::file_size(filePath);
  auto result = std::vector<uint8_t>(length);

  std::ifstream input{filePath, std::ios::binary};
  input.read(reinterpret_cast<char*>(result.data()), length);

  return result;
}
