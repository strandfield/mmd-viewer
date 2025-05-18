// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "rendering/psxobject3d.h"

#include "gamereader.h"

#include "converters/tmd2object3d.h"
#include "formats/mmd.h"

class CharacterModel : public Object3D
{
public:
  CharacterEntry info;
  MMD_File mmd;
  std::vector<MMD_Animation> animations;
  std::vector<Object3D*> nodes;

public:
  CharacterModel(const CharacterEntry& info, const MMD_File& mmd)
  {
    TMD_ModelConverter converter;
    converter.setTIMs({info.texture});

    this->mmd = mmd;
    this->nodes.reserve(info.skeleton.size());

    for (const SkeletonNodeRel& rel : info.skeleton)
    {
      if (rel.parent == 255 && rel.object == 255)
      {
        this->nodes.push_back(this);
        continue;
      }

      std::unique_ptr<Object3D> obj;

      if (rel.object != 255)
      {
        obj = converter.convertObject(mmd.tmd.objects()[rel.object]);
      }
      else
      {
        obj = std::make_unique<Group>();
      }

      assert(rel.parent != 255);
      {
        this->nodes.push_back(obj.get());
        this->nodes[rel.parent]->add(std::move(obj));
      }
    }

    this->animations = mmd.animations.decode(info.skeleton.size());
  }

  void setupAnimation(const MMD_Animation& animation)
  {
    for (size_t i(0); i < animation.initialPositions.size(); ++i)
    {
      const MMD_Animation::Position& pose = animation.initialPositions[i];
      Object3D& node = *this->nodes[i];
      node.setPosition(QVector3D(pose.posX, pose.posY, pose.posZ));
      node.setScale(QVector3D(pose.scaleX, pose.scaleY, pose.scaleZ) / float(0x1000));
      node.setRotation(
          EulerAngles(QVector3D(pose.rotX, pose.rotY, pose.rotZ) * 360 / float(0x1000)));
    }
  }

  void setupAnimation(int index = 0) { setupAnimation(this->animations.at(index)); }
};
