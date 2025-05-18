// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "charactermodel.h"

#include <QObject>

#include <array>

struct AnimationMomentumData
{
  // momentum along each axis
  // (i.e., MMD_Animation::Axis)
  std::array<float, 9> values;
};

struct AnimationState
{
  int frameNum = 0;
  int timecode = 0; // TODO: should be initialized to 1 ?
  int loopCounter = 0;
  int loopJumpbackIndex = -1;
  int pc = 0;
  std::vector<AnimationMomentumData> momentumData;
};

struct AnimationData
{
  AnimationState state;
  CharacterModel* model = nullptr;
  MMD_Animation animation;
  bool infinite = false;
};

class AnimationPlayer : public QObject
{
  Q_OBJECT

public:
  explicit AnimationPlayer(CharacterModel& model, QObject* parent = nullptr);

  using MomentumData = AnimationMomentumData;

  void playAnimation(int index);
  void playAnimation(const MMD_Animation& animation);

  bool isInfinite() const;

Q_SIGNALS:
  void stepped();
  void finished();

protected Q_SLOTS:
  void onTimerTimeout();

protected:
  void step();
  void applyMomentum();

private:
  QTimer* m_timer;
  AnimationData m_animationData;
};
