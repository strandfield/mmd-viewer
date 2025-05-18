// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QVector3D>

#include <QQuaternion>

#include <cmath>

// rotation angles, in degrees
class EulerAngles
{
private:
  float m_x, m_y, m_z;

public:
  explicit EulerAngles(const QVector3D& angles = QVector3D())
      : m_x(angles.x())
      , m_y(angles.y())
      , m_z(angles.z())
  {}

  EulerAngles(float rx, float ry, float rz)
      : m_x(rx)
      , m_y(ry)
      , m_z(rz)
  {}

  float x() const { return m_x; }
  float y() const { return m_y; }
  float z() const { return m_z; }

  QVector3D toVector() const { return QVector3D(x(), y(), z()); }

  QQuaternion toQuaternion() const
  {
    QVector3D radians = (M_PI / 180.f) * toVector();

    // http://www.mathworks.com/matlabcentral/fileexchange/
    // 	20696-function-to-convert-between-dcm-euler-angles-quaternions-and-euler-vectors/
    //	content/SpinCalc.m

    const float c1 = std::cos(radians.x() / 2);
    const float c2 = std::cos(radians.y() / 2);
    const float c3 = std::cos(radians.z() / 2);

    const float s1 = std::sin(radians.x() / 2);
    const float s2 = std::sin(radians.y() / 2);
    const float s3 = std::sin(radians.z() / 2);

    const float x = s1 * c2 * c3 + c1 * s2 * s3;
    const float y = c1 * s2 * c3 - s1 * c2 * s3;
    const float z = c1 * c2 * s3 + s1 * s2 * c3;
    const float w = c1 * c2 * c3 - s1 * s2 * s3;

    return QQuaternion(w, x, y, z);
  }
};

inline bool operator==(const EulerAngles& lhs, const EulerAngles& rhs)
{
  return lhs.toVector() == rhs.toVector();
}

inline bool operator!=(const EulerAngles& lhs, const EulerAngles& rhs)
{
  return !(lhs == rhs);
}

// not sure this makes sense
inline EulerAngles operator+(const EulerAngles& lhs, const EulerAngles& rhs)
{
  return EulerAngles(lhs.toVector() + rhs.toVector());
}
