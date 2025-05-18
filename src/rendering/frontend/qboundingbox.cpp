// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "qboundingbox.h"

QBoundingBox::QBoundingBox(QObject* parent) : QObject(parent)
{

}

QBoundingBox::QBoundingBox(const AABB& bbox, QObject* parent) : QObject(parent),
  m_aabb(bbox)
{

}

const AABB& QBoundingBox::aabb() const
{
  return m_aabb;
}

void QBoundingBox::setAABB(const AABB& bbox)
{
  if (m_aabb != bbox)
  {
    m_aabb = bbox;
    Q_EMIT bboxChanged();
  }
}

bool QBoundingBox::isNull() const
{
  return aabb() == AABB();
}

QVector3D QBoundingBox::center() const
{
  return aabb().center();
}

float QBoundingBox::length() const
{
  return aabb().max.x() - aabb().min.x();
}

float QBoundingBox::width() const
{
  return aabb().max.y() - aabb().min.y();
}

float QBoundingBox::height() const
{
  return aabb().max.z() - aabb().min.z();
}

QVector3D QBoundingBox::diagonal() const
{
  return { length(), width(), height() };
}
