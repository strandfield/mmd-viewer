// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "formats/tim.h"

#include <QImage>
#include <QPixmap>

#include <vector>

inline QImage tim2image(const TimImage::Image& src)
{
  QImage qimg{QSize(src.width, src.height), QImage::Format_ARGB32};
  for (int i(0); i < src.width; ++i)
  {
    for (int j(0); j < src.height; ++j)
    {
      qimg.setPixel(i, j, src.pixelData.at(j * src.width + i));
    }
  }
  return qimg;
}

inline std::vector<QImage> tim2images(const TimImage& src)
{
  std::vector<QImage> result;

  if (src.usesPalette())
  {
    for (int i(0); i < src.numberOfPalettes(); ++i)
    {
      TimImage::Image imgdata = src.generateImage(i);
      result.push_back(tim2image(imgdata));
    }
  }
  else
  {
    TimImage::Image imgdata = src.generateImage();
    result.push_back(tim2image(imgdata));
  }

  return result;
}
