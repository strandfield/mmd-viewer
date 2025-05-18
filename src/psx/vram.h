#pragma once

#include <cstdint>

#include <algorithm>
#include <array>

class VRAM
{
public:
  std::array<uint16_t, 1024 * 512> data;

public:
  VRAM() { std::fill(data.begin(), data.end(), uint16_t(0)); }
};
