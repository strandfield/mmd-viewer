// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "color.h"

#include <QVector3D>

/**
 * @brief a colored point in a 3D space
 */
struct ColoredVertex
{
  QVector3D position; ///< the position of the point
  RgbColor color;     ///< the rgb color of the point
};
