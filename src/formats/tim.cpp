// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "tim.h"

#include "datastream.h"

#include <cassert>
#include <cmath>
#include <fstream>

inline unsigned long colorFromPsx24bit(u32 c)
{
  constexpr unsigned long mask = 0xff;
  unsigned long red = c & mask;
  unsigned long green = (c >> 8) & mask;
  unsigned long blue = (c >> 8) & mask;
  return (0xffu << 24) | (red << 16) | (green << 8) | (blue);
}

unsigned long colorFromPsx16bit(u16 c)
{
  constexpr unsigned long mask = 0b11111u;
  unsigned long red = c & mask;
  unsigned long green = (c >> 5) & mask;
  unsigned long blue = (c >> 10) & mask;
  const bool stpbit = (c >> 15); // special transparency processing bit

  red = (unsigned long) std::round((red * 255) / double(mask));
  green = (unsigned long) std::round((green * 255) / double(mask));
  blue = (unsigned long) std::round((blue * 255) / double(mask));

  unsigned long alpha = stpbit ? 0 : 255;

  // rgb(0,0,0) is transparent by default, unless the stp bit is set
  if (red == 0 && green == 0 && blue == 0)
  {
    alpha = 255 - alpha;
  }

  return (alpha << 24) | (red << 16) | (green << 8) | (blue);
}

void TimImageColorPalettes::fill(std::vector<TimImageColorPalette::Color>&& colors, int nbPalettes)
{
  m_colors = std::move(colors);
  m_number_of_palettes = nbPalettes;
  m_number_of_colors = m_colors.size() / m_number_of_palettes;
}

u16 TimImageColorPalettes::x() const
{
  return m_x;
}

u16 TimImageColorPalettes::y() const
{
  return m_y;
}

void TimImageColorPalettes::setVramCoordinates(u16 x, u16 y)
{
  m_x = x;
  m_y = y;
}

TimImage::TimImage(const std::filesystem::path& filePath)
{
  TimReader reader;
  reader.readTim(filePath, *this);
}

TimImage::TimImage(Buffer* buffer)
{
  if (buffer)
  {
    TimReader reader;
    reader.readTim(*buffer, *this);
  }
}

bool TimImage::isNull() const
{
  return width() == 0 && height() == 0;
}

int TimImage::width() const
{
  return (m_imdata.width * 16) / m_type.bpp();
}

int TimImage::height() const
{
  return m_imdata.height;
}

bool TimImage::load(const std::filesystem::path& filePath)
{
  TimReader reader;
  return reader.readTim(filePath, *this);
}

bool TimImage::usesPalette() const
{
  return m_type.clut();
}

int TimImage::numberOfPalettes() const
{
  return m_palettes.numberOfPalettes();
}

const TimImage::Palettes& TimImage::palettes() const
{
  return m_palettes;
}

TimImage::Image TimImage::generateImage() const
{
  if (usesPalette())
  {
    return generateImage(0);
  }

  Image result{};

  if (m_type.bpp() == 16)
  {
    result.width = width();
    result.height = height();
    result.pixelData.reserve(result.width * result.height);
    for (u16 p : m_imdata.data)
    {
      result.pixelData.push_back(colorFromPsx16bit(p));
    }
    return result;
  }
  else if (m_type.bpp() == 24)
  {
    result.width = width();
    result.height = height();
    result.pixelData.reserve(result.width * result.height);
    const u16* data = m_imdata.data.data();
    size_t i = 0;
    const size_t n = result.width * result.width;

    constexpr u32 mask = 0xffu;

    while (result.pixelData.size() < n)
    {
      u32 c = 0;

      if (i % 2 == 0)
      {
        c = data[i++];
        c |= (data[i] & mask) << 16;
      }
      else
      {
        c = data[i++] >> 8;
        c |= data[i++] << 8;
      }

      result.pixelData.push_back(colorFromPsx24bit(c));
    }

    return result;
  }
  else
  {
    return result;
  }
}

TimImage::Image TimImage::generateImage(int paletteIndex) const
{
  return generateImageFromPalette(paletteIndex);
}

TimImage::Image TimImage::generateImage(int clutX, int clutY) const
{
  if (!usesPalette())
  {
    std::cerr << "bad call to generateImage(int clutX, int clutY)" << std::endl;
    return generateImage();
  }

  if (usesPalette())
  {
    if (clutX == -1)
    {
      clutX = palettes().x();
    }

    if (clutY == -1)
    {
      clutY = palettes().y();
    }
  }

  const int paletteIndex = clutY - palettes().y();
  const int offset = clutX - palettes().x();

  return generateImageFromPalette(paletteIndex, offset);
}

TimImage::Image TimImage::generateImageFromPalette(int paletteIndex, int offset) const
{
  const Palette& p = m_palettes.palette(paletteIndex);

  Image result{};
  result.width = width();
  result.height = height();
  const size_t n = result.width * result.height;
  result.pixelData.reserve(n);

  const u16* data = m_imdata.data.data();
  size_t i = 0;

  if (m_type.bpp() == 4)
  {
    constexpr u16 mask = 0xf;
    while (result.pixelData.size() < n)
    {
      u16 indices = data[i++];
      u16 i1 = (indices >> 0) & mask;
      u16 i2 = (indices >> 4) & mask;
      u16 i3 = (indices >> 8) & mask;
      u16 i4 = (indices >> 12) & mask;
      result.pixelData.push_back(p.at(offset + i1));
      result.pixelData.push_back(p.at(offset + i2));
      result.pixelData.push_back(p.at(offset + i3));
      result.pixelData.push_back(p.at(offset + i4));
    }
  }
  else if (m_type.bpp() == 8)
  {
    constexpr u16 mask = 0xff;
    while (result.pixelData.size() < n)
    {
      u16 indices = data[i++];
      u16 i1 = indices & mask;
      u16 i2 = indices >> 8;
      result.pixelData.push_back(p.at(offset + i1));
      result.pixelData.push_back(p.at(offset + i2));
    }
  }
  else
  {
    return {};
  }

  return result;
}

bool TimReader::readTim(const std::filesystem::path& filePath, TimImage& outputImage)
{
  std::ifstream stream{filePath, std::ios::in | std::ios::binary};
  return readTim(stream, outputImage);
}

bool TimReader::readTim(std::ifstream& stream, TimImage& outputImage)
{
  if (!stream.is_open())
  {
    return false;
  }

  const u32 magic = read<u32>(stream);
  if (magic != 0x10)
  {
    return false;
  }

  outputImage.m_type.m_data = read<u32>(stream);

  if (outputImage.m_type.clut())
  {
    if (!(outputImage.m_type.bpp() == 4 || outputImage.m_type.bpp() == 4))
    {
      return false;
    }
  }
  else
  {
    if (!(outputImage.m_type.bpp() == 16 || outputImage.m_type.bpp() == 24))
    {
      return false;
    }
  }

  if (outputImage.m_type.clut())
  {
    const u32 clut_length = read<u32>(stream);
    const u16 clut_x = read<u16>(stream);
    const u16 clut_y = read<u16>(stream);
    const u16 clut_width = read<u16>(stream);
    const u16 clut_height = read<u16>(stream);
    const size_t n = clut_width * clut_height;
    assert(clut_width * clut_height * 2 == clut_length - 3 * 4);
    std::vector<TimImageColorPalette::Color> colors;
    colors.reserve(n);
    while (colors.size() < n)
    {
      const u16 c = read<u16>(stream);
      colors.push_back(colorFromPsx16bit(c));
    }
    outputImage.m_palettes.fill(std::move(colors), clut_height);
    outputImage.m_palettes.setVramCoordinates(clut_x, clut_y);
  }
  else
  {
    outputImage.m_palettes.fill({}, 0);
  }

  // read image data
  {
    outputImage.m_imdata.length = read<u32>(stream);
    outputImage.m_imdata.x = read<u16>(stream);
    outputImage.m_imdata.y = read<u16>(stream);
    outputImage.m_imdata.width = read<u16>(stream);
    outputImage.m_imdata.height = read<u16>(stream);
    assert(outputImage.m_imdata.width * outputImage.m_imdata.height * 2
           == outputImage.m_imdata.length - 3 * 4);
    outputImage.m_imdata.data = readvec<u16>(stream,
                                             outputImage.m_imdata.width
                                                 * outputImage.m_imdata.height);
  }

  return true;
}

bool TimReader::seekNext(std::ifstream& stream)
{
  if (stream.eof())
  {
    return false;
  }

  u32 magic = read<u32>(stream);
  while (!stream.eof() && magic != 0x10)
  {
    u8 ch = read<u8>(stream);
    u32 high = u32(ch) << u32(24);
    magic = (magic >> 8) | high;
  }

  if (!stream.eof())
  {
    stream.seekg(stream.tellg() - std::ifstream::pos_type(sizeof(u32)));
    return true;
  }
  else
  {
    return false;
  }
}

bool TimReader::readTim(Buffer& buffer, TimImage& outputImage)
{
  // TODO: remove duplication with the std::ifstream overload

  const u32 magic = readbuf<u32>(buffer);
  if (magic != 0x10)
  {
    return false;
  }

  outputImage.m_type.m_data = readbuf<u32>(buffer);

  if (outputImage.m_type.clut())
  {
    if (!(outputImage.m_type.bpp() == 4 || outputImage.m_type.bpp() == 4))
    {
      return false;
    }
  }
  else
  {
    if (!(outputImage.m_type.bpp() == 16 || outputImage.m_type.bpp() == 24))
    {
      return false;
    }
  }

  if (outputImage.m_type.clut())
  {
    const u32 clut_length = readbuf<u32>(buffer);
    const u16 clut_x = readbuf<u16>(buffer);
    const u16 clut_y = readbuf<u16>(buffer);
    const u16 clut_width = readbuf<u16>(buffer);
    const u16 clut_height = readbuf<u16>(buffer);
    const size_t n = clut_width * clut_height;
    assert(clut_width * clut_height * 2 == clut_length - 3 * 4);
    std::vector<TimImageColorPalette::Color> colors;
    colors.reserve(n);
    while (colors.size() < n)
    {
      const u16 c = readbuf<u16>(buffer);
      colors.push_back(colorFromPsx16bit(c));
    }
    outputImage.m_palettes.fill(std::move(colors), clut_height);
    outputImage.m_palettes.setVramCoordinates(clut_x, clut_y);
  }
  else
  {
    outputImage.m_palettes.fill({}, 0);
  }

  // read image data
  {
    outputImage.m_imdata.length = readbuf<u32>(buffer);
    outputImage.m_imdata.x = readbuf<u16>(buffer);
    outputImage.m_imdata.y = readbuf<u16>(buffer);
    outputImage.m_imdata.width = readbuf<u16>(buffer);
    outputImage.m_imdata.height = readbuf<u16>(buffer);
    assert(outputImage.m_imdata.width * outputImage.m_imdata.height * 2
           == outputImage.m_imdata.length - 3 * 4);

    const size_t s = outputImage.m_imdata.width * outputImage.m_imdata.height;
    outputImage.m_imdata.data.resize(s);
    for (size_t i(0); i < s; ++i)
    {
      outputImage.m_imdata.data[i] = readbuf<u16>(buffer);
    }
  }

  return true;
}
