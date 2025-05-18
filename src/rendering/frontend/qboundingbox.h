// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "math/aabb.h"

#include <QObject>

class QBoundingBox : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool isNull READ isNull NOTIFY bboxChanged)
  Q_PROPERTY(QVector3D center READ center NOTIFY bboxChanged)
  Q_PROPERTY(QVector3D diagonal READ diagonal NOTIFY bboxChanged)
  Q_PROPERTY(float length READ length NOTIFY bboxChanged)
  Q_PROPERTY(float width READ width NOTIFY bboxChanged)
  Q_PROPERTY(float height READ height NOTIFY bboxChanged)
public:
  explicit QBoundingBox(QObject* parent = nullptr);
  explicit QBoundingBox(const AABB& bbox, QObject* parent = nullptr);

  const AABB& aabb() const;
  void setAABB(const AABB& bbox);

  bool isNull() const;

  QVector3D center() const;

  float length() const;
  float width() const;
  float height() const;
  QVector3D diagonal() const;

Q_SIGNALS:
  void bboxChanged();

private:
  AABB m_aabb;
};
