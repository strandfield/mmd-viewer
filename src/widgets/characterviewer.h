// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "formats/mmd.h"
#include "gamereader.h"

#include <QWidget>

class AnimationPlayer;
class CharacterModel;
class SceneViewer;

class CharacterViewer : public QWidget
{
  Q_OBJECT
public:
  explicit CharacterViewer(QWidget* parent = nullptr);
  CharacterViewer(const CharacterEntry& characterEntry,
                  const MMD_File& mmd,
                  QWidget* parent = nullptr);

  const QString& sourceDir() const;
  void setSourceDir(const QString& sourceDir);

  void reset(const CharacterEntry& characterEntry);

  CharacterModel* model() const;

  int animationCount() const;
  void playAnimation(int index);

private:
  void init();

private:
  QString m_sourceDir;
  SceneViewer* m_viewer;
  CharacterModel* m_model;
  AnimationPlayer* m_player = nullptr;
};
