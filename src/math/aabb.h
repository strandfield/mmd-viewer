// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QVector3D>
#include <QMatrix4x4>

#include <array>
#include <limits>

class AABB
{
public:
  QVector3D min;
  QVector3D max;

public:
  AABB();
  AABB(const AABB&) = default;
  ~AABB() = default;

  AABB(const QVector3D& a, const QVector3D& b);

  AABB& extend(const QVector3D& pt);

  QVector3D center() const;

  std::array<QVector3D, 8> corners() const;

protected:
  static float fmin() { return std::numeric_limits<float>::lowest(); }
  static float fmax() { return std::numeric_limits<float>::max(); }
};

inline AABB::AABB() :
  min(fmax(), fmax(), fmax()),
  max(fmin(), fmin(), fmin())
{

}

inline AABB::AABB(const QVector3D& a, const QVector3D& b)
{
  min = QVector3D(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()));
  max = QVector3D(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()));
}

inline AABB& AABB::extend(const QVector3D& pt)
{
  min = QVector3D(std::min(min.x(), pt.x()), std::min(min.y(), pt.y()), std::min(min.z(), pt.z()));
  max = QVector3D(std::max(max.x(), pt.x()), std::max(max.y(), pt.y()), std::max(max.z(), pt.z()));
  return *this;
}

inline QVector3D AABB::center() const
{
  return 0.5 * (min + max);
}

inline std::array<QVector3D, 8> AABB::corners() const
{
  return {
    QVector3D(min.x(), min.y(), min.z()),
    QVector3D(min.x(), max.y(), min.z()),
    QVector3D(max.x(), max.y(), min.z()),
    QVector3D(max.x(), min.y(), min.z()),
    QVector3D(min.x(), min.y(), max.z()),
    QVector3D(max.x(), min.y(), max.z()),
    QVector3D(max.x(), max.y(), max.z()),
    QVector3D(min.x(), max.y(), max.z()),
  };
}

inline bool operator==(const AABB& lhs, const AABB& rhs)
{
  return lhs.min == rhs.min && lhs.max == rhs.max;
}

inline bool operator!=(const AABB& lhs, const AABB& rhs)
{
  return !(lhs == rhs);
}

inline AABB operator*(const AABB& lhs, const QMatrix4x4& tr)
{
  AABB result;

  auto cs = lhs.corners();

  for (QVector3D c : cs)
  {
    QVector3D trc = tr * c; // trap: it's in that order and not 'c * tr'
    result.extend(trc);
  }

  return result;
}

inline AABB united(const AABB& a, const AABB& b)
{
  return AABB(
    QVector3D(std::min(a.min.x(), b.min.x()), std::min(a.min.y(), b.min.y()), std::min(a.min.z(), b.min.z())),
    QVector3D(std::max(a.max.x(), b.max.x()), std::max(a.max.y(), b.max.y()), std::max(a.max.z(), b.max.z()))
  );
}
