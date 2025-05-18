// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QColor>

#include <cstdint>

/**
 * @brief a rgb color with 8-bit per channel
 *
 * This class is meant for storing colors in large buffers
 * that are uploaded to OpenGL.
 *
 * It is not suitable for manipulating colors in a friendly
 * way.
 *
 * Layout: one byte per channel, RGB, sizeof(RgbColor) == 3.
 */
struct RgbColor
{
  RgbColor() = default;
  explicit RgbColor(unsigned int rgb);
  RgbColor(uint8_t r, uint8_t g, uint8_t b);
  RgbColor(int r, int g, int b);
  RgbColor(float r, float g, float b);

  RgbColor(const QColor& color);

  operator QColor() const
  {
    return QColor(r, g, b);
  }

  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
};

static_assert(sizeof(RgbColor) == 3);

inline RgbColor::RgbColor(unsigned int rgb)
  : r((rgb >> 16) & 0xFF)
  , g((rgb >> 8) & 0xFF)
  , b(rgb & 0xFF)
{

}

inline RgbColor::RgbColor(uint8_t r_, uint8_t g_, uint8_t b_)
  : r(r_)
  , g(g_)
  , b(b_)
{
}

inline RgbColor::RgbColor(int r_, int g_, int b_)
  : RgbColor(static_cast<uint8_t>(r_), static_cast<uint8_t>(g_), static_cast<uint8_t>(b_))
{
}

inline RgbColor::RgbColor(float r_, float g_, float b_)
  : RgbColor(static_cast<uint8_t>(r_ * 255), static_cast<uint8_t>(g_ * 255), static_cast<uint8_t>(b_ * 255))
{
}

inline RgbColor::RgbColor(const QColor& color)
  : RgbColor(color.red(), color.green(), color.blue())
{
}
