// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "defs.h"

/**
 * @brief performs angle conversion from degrees to radians
 * @param a  the angle in degrees
 * @return the angle in radians
 */
inline float deg2rad(float a)
{
  return a * float(M_PI) / 180;
}

/**
 * @brief performs angle conversion from radians to degress
 * @param a  the angle in radians
 * @return the angle in degrees
 */
inline float rad2deg(float a)
{
  return a * 180 / float(M_PI);
}
