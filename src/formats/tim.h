// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef TIM_H
#define TIM_H

#include "typedefs.h"

#include "buffer.h"

#include <filesystem>
#include <vector>

// http://fileformats.archiveteam.org/wiki/TIM_(PlayStation_graphics)
// https://www.psxdev.net/forum/viewtopic.php?t=109

// convert color from psx A1B5G5R5 16-bit color
unsigned long colorFromPsx16bit(u16 c);

class TimImageColorPalette
{
public:
  TimImageColorPalette() = default;

  using Color = unsigned long; // 32-bit ARGB format (0xAARRGGBB)

  explicit TimImageColorPalette(std::vector<Color>&& colors)
      : m_colors(std::move(colors))
  {}

  int size() const;

  const Color& at(int i) const;
  const std::vector<Color>& colors() const;

private:
  std::vector<Color> m_colors;
};

inline int TimImageColorPalette::size() const
{
  return colors().size();
}

inline const TimImageColorPalette::Color& TimImageColorPalette::at(int i) const
{
  return colors().at(i);
}

inline const std::vector<TimImageColorPalette::Color>& TimImageColorPalette::colors() const
{
  return m_colors;
}

class TimImageColorPalettes
{
public:
  int numberOfPalettes() const;
  int numberOfColorsPerPalette() const;
  std::vector<TimImageColorPalette> palettes() const;
  TimImageColorPalette palette(int i) const;

  void fill(std::vector<TimImageColorPalette::Color>&& colors, int nbPalettes);

  u16 x() const;
  u16 y() const;
  void setVramCoordinates(u16 x, u16 y);

private:
  u16 m_number_of_colors = 0;
  u16 m_number_of_palettes = 0;
  std::vector<TimImageColorPalette::Color> m_colors;
  u16 m_x = 0;
  u16 m_y = 0;
};

inline int TimImageColorPalettes::numberOfPalettes() const
{
  return (int) m_number_of_palettes;
}

inline int TimImageColorPalettes::numberOfColorsPerPalette() const
{
  return (int) m_number_of_colors;
}

inline std::vector<TimImageColorPalette> TimImageColorPalettes::palettes() const
{
  std::vector<TimImageColorPalette> result;
  result.reserve(numberOfPalettes());

  for (size_t i(0); i < numberOfPalettes(); ++i)
  {
    result.push_back(palette(i));
  }

  return result;
}

inline TimImageColorPalette TimImageColorPalettes::palette(int i) const
{
  auto colors = std::vector<TimImageColorPalette::Color>(m_colors.begin() + i * m_number_of_colors,
                                                         m_colors.begin()
                                                             + (i + 1) * m_number_of_colors);
  return TimImageColorPalette(std::move(colors));
}

class TimImage
{
public:
  TimImage() = default;
  explicit TimImage(const std::filesystem::path& filePath);
  explicit TimImage(Buffer* buffer);

  struct Image
  {
    int width;
    int height;
    std::vector<unsigned long> pixelData;
  };

  bool isNull() const;

  int width() const;
  int height() const;

  bool load(const std::filesystem::path& filePath);

  using Palettes = TimImageColorPalettes;
  using Palette = TimImageColorPalette;
  bool usesPalette() const;
  int numberOfPalettes() const;
  const Palettes& palettes() const;

  uint32_t getPixelX() const { return m_imdata.x; }
  uint32_t getPixelY() const { return m_imdata.y; }
  const std::vector<u16>& pixelData() const { return m_imdata.data; }

  Image generateImage() const;
  Image generateImage(int paletteIndex) const;
  Image generateImage(int clutX, int clutY) const;

  Image generateImageFromPalette(int paletteIndex, int offset = 0) const;

private:
  friend class TimReader;

  class TimType
  {
  public:
    uint32_t m_data = 0;

  public:
    int bpp() const
    {
      switch (m_data & 0b111)
      {
      case 0:
        return 4;
      case 1:
        return 8;
      case 2:
        return 16;
      case 3:
        return 24;
      default:
        return -1;
      }
    }

    bool clut() const { return (m_data & 0b1000); }
  };
  TimType m_type;
  TimImageColorPalettes m_palettes;
  struct ImageData
  {
    u32 length;
    u16 x;
    u16 y;
    u16 width = 0;
    u16 height = 0;
    std::vector<u16> data;
  };
  ImageData m_imdata;
};

class TimReader
{  
public:
  bool readTim(const std::filesystem::path& filePath, TimImage& outputImage);
  bool readTim(std::ifstream& stream, TimImage& outputImage);
  bool seekNext(std::ifstream& stream);
  bool readTim(Buffer& buffer, TimImage& outputImage);
};

#endif // TIM_H
