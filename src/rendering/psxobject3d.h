// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "object3d.h"

#include "color.h"

#include <QImage>

#include <memory>

struct PSX_Texture : public std::enable_shared_from_this<PSX_Texture>
{
  QImage image;
  int revision = 0;
};

class PSX_Material
{
public:
  virtual ~PSX_Material() = default;

  bool vertexColors = false;
  bool lighting = false;
  RgbColor color = RgbColor(255, 255, 255);
  std::shared_ptr<PSX_Texture> map;
};

class PSX_Object3D : public Object3D
{
public:
  std::vector<QVector3D> vertices;
  std::vector<RgbColor> colors;
  std::vector<QVector2D> uv;
  std::vector<QVector3D> normals;

  std::vector<std::shared_ptr<PSX_Material>> materials;

  enum PrimitiveType { Line, Triangle, Quad, Sprite };

  struct PrimitiveInfo
  {
    int index;
    int count;
    PrimitiveType type;
    int materialIndex;
  };

  std::vector<PrimitiveInfo> primitives;
};
