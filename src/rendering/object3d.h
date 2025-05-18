// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "math/eulerangles.h"

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>

#include <memory>

template<typename T>
class Lazy
{
private:
  T m_value;

public:
  bool dirty = true;

public:
  const T& value() const
  {
    assert(!dirty);
    return m_value;
  }

  void update(T value)
  {
    m_value = std::move(value);
    dirty = false;
  }
};

class Object3D
{
public:
  Object3D();
  Object3D(const Object3D&) = delete;
  virtual ~Object3D() = default;

  bool visible() const;
  void setVisible(bool visible);

  const QVector3D& position() const;
  void setPosition(const QVector3D& pos);

  const EulerAngles& rotation() const;
  void setRotation(const EulerAngles& angles);
  QQuaternion quaternion() const;

  const QVector3D& scale() const;
  void setScale(const QVector3D& scale);

  const QMatrix4x4& matrix() const;

  Object3D* parent() const;
  std::vector<Object3D*> children() const;
  int childCount() const;
  Object3D* childAt(int index) const;
  void add(std::unique_ptr<Object3D> child);
  std::unique_ptr<Object3D> takeChildAt(int index);
  std::unique_ptr<Object3D> takeChild(const Object3D* child);
  int indexOf(const Object3D* child) const;
  void clear();

private:
  // visibility
  bool m_visible = true;
  // transformations
  QVector3D m_position;
  EulerAngles m_rotation;
  QVector3D m_scale = QVector3D(1, 1, 1);
  mutable Lazy<QMatrix4x4> m_matrix;
  // parent/children
  std::vector<std::unique_ptr<Object3D>> m_children;
  Object3D* m_parent = nullptr;
};

inline Object3D::Object3D()
{
  m_matrix.dirty = false;
}

inline bool Object3D::visible() const
{
  return m_visible;
}

inline void Object3D::setVisible(bool visible)
{
  m_visible = visible;
}

inline const QVector3D& Object3D::position() const
{
  return m_position;
}

inline void Object3D::setPosition(const QVector3D& pos)
{
  if (m_position != pos)
  {
    m_position = pos;
    m_matrix.dirty = true;
  }
}

inline const EulerAngles& Object3D::rotation() const
{
  return m_rotation;
}

inline void Object3D::setRotation(const EulerAngles& angles)
{
  if (m_rotation != angles)
  {
    m_rotation = angles;
    m_matrix.dirty = true;
  }
}

inline QQuaternion Object3D::quaternion() const
{
  return m_rotation.toQuaternion();
}

inline const QVector3D& Object3D::scale() const
{
  return m_scale;
}

inline void Object3D::setScale(const QVector3D& scale)
{
  if (m_scale != scale)
  {
    m_scale = scale;
    m_matrix.dirty = true;
  }
}

inline const QMatrix4x4& Object3D::matrix() const
{
  if (m_matrix.dirty)
  {
    QMatrix4x4 m;
    m.translate(position());
    m.rotate(quaternion());
    m.scale(scale());
    m_matrix.update(m);
  }

  return m_matrix.value();
}

inline Object3D* Object3D::parent() const
{
  return m_parent;
}

inline std::vector<Object3D*> Object3D::children() const
{
  std::vector<Object3D*> result;
  result.reserve(m_children.size());
  for (const auto& ptr : m_children)
  {
    result.push_back(ptr.get());
  }

  return result;
}

inline int Object3D::childCount() const
{
  return m_children.size();
}

inline Object3D* Object3D::childAt(int index) const
{
  return index >= 0 && index < childCount() ? m_children[index].get() : nullptr;
}

inline void Object3D::add(std::unique_ptr<Object3D> child)
{
  if (!child)
  {
    return;
  }

  if (child->parent() && child->parent() != this)
  {
    return;
  }

  child->m_parent = this;
  m_children.push_back(std::move(child));
}

inline std::unique_ptr<Object3D> Object3D::takeChildAt(int index)
{
  if (index < 0 || index >= m_children.size())
  {
    return nullptr;
  }

  std::unique_ptr<Object3D> result = std::move(m_children[index]);
  result->m_parent = nullptr;
  m_children.erase(m_children.begin() + index);
  return result;
}

inline std::unique_ptr<Object3D> Object3D::takeChild(const Object3D* child)
{
  return takeChildAt(indexOf(child));
}

inline int Object3D::indexOf(const Object3D* child) const
{
  auto it = std::find_if(m_children.begin(), m_children.end(), [child](const auto& e) {
    return e.get() == child;
  });

  return it != m_children.end() ? int(std::distance(m_children.begin(), it)) : -1;
}

inline void Object3D::clear()
{
  m_children.clear();
}

class Group : public Object3D
{
public:
  using Object3D::Object3D;
  ~Group() = default;
};
