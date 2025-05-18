// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "gamedata.h"

#include <QWidget>

class QListWidget;

class CharacterViewer;

class CharactersViewer : public QWidget
{
  Q_OBJECT
public:
  CharactersViewer(const GameData& gameData, QWidget* parent = nullptr);

protected Q_SLOTS:
  void onSelectedCharacterChanged();
  void onSelectedAnimationChanged();

private:
  void fillAnimationList();

private:
  GameData m_gameData;
  QListWidget* m_characterList;
  CharacterViewer* m_viewer;
  // right column:
  QListWidget* m_animationList;
};
