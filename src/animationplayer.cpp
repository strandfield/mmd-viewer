// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "animationplayer.h"

#include <QTimer>

#include <set>

static constexpr float axisFactor(MMD_Animation::Axis axis)
{
  switch (axis)
  {
  case MMD_Animation::Axis::SCALE_X:
  case MMD_Animation::Axis::SCALE_Y:
  case MMD_Animation::Axis::SCALE_Z:
    return 1.0f / 4096.0f;
  case MMD_Animation::Axis::ROT_X:
  case MMD_Animation::Axis::ROT_Y:
  case MMD_Animation::Axis::ROT_Z:
    return 360.0f / 4096.0f;
  default:
    return 1.0f;
  }
}

class AnimInstructionExecutor
{
public:
  AnimationData& data;
  AnimationState& state;

public:
  explicit AnimInstructionExecutor(AnimationData& animData)
      : data(animData)
      , state(data.state)
  {}

  void apply(const MMD_Animation::Instruction& instruction) { std::visit(*this, instruction); }

public:
  void operator()(const MMD_Animation::KeyframeInstruction& ins)
  {
    for (const MMD_Animation::KeyframeEntry& e : ins.entries)
    {
      AnimationMomentumData& momentum = this->state.momentumData[e.affectedNode];

      for (const auto& p : e.values)
      {
        MMD_Animation::Axis axis;
        float value;
        std::tie(axis, value) = p;

        momentum.values[static_cast<int>(axis)] = value * axisFactor(axis);
      }
    }
  }

  void operator()(const MMD_Animation::LoopStartInstruction& ins)
  {
    this->state.loopJumpbackIndex = this->state.pc;
    this->state.loopCounter = ins.loopCount;
  }

  void operator()(const MMD_Animation::LoopEndInstruction& ins)
  {
    if (this->state.loopCounter != 255 && this->state.loopCounter != 0)
    {
      this->state.loopCounter -= 1;
      if (this->state.loopCounter == 0)
      {
        return;
      }
    }

    this->state.timecode = ins.newTime;

    // note: because "pc" is incremented after each instruction is executed,
    // the "loop start" instruction will actually be skipped (which is what
    // we want anyway).
    this->state.pc = this->state.loopJumpbackIndex;
  }

  void operator()(const MMD_Animation::TextureInstruction& ins)
  {
    // As far as I understand, we multiply coordinates along
    // the X-axis by 4 because the TIM are 4 bits per pixel and
    // the VRAM is made of 16-bit units.
    const int srcX = ins.srcX * 4;
    const int srcY = ins.srcY;
    const int destX = ins.destX * 4;
    const int destY = ins.destY;
    const int width = ins.width * 4;
    const int height = ins.height;

    std::set<PSX_Material*> done;

    for (Object3D* node : this->data.model->nodes)
    {
      if (auto* psxobj = dynamic_cast<PSX_Object3D*>(node))
      {
        for (std::shared_ptr<PSX_Material> material : psxobj->materials)
        {
          if (material->map)
          {
            if (done.find(material.get()) != done.end())
            {
              continue;
            }

            QImage& image = material->map->image;

            for (int y(0); y < height; ++y)
            {
              for (int x(0); x < width; ++x)
              {
                auto pixel = image.pixel(x + srcX, y + srcY);
                image.setPixel(x + destX, y + destY, pixel);
              }
            }

            ++(material->map->revision);
            done.insert(material.get());
          }
        }
      }
    }
  }

  void operator()(const MMD_Animation::PlaySoundInstruction& ins)
  {
    // qDebug() << "playsound:" << ins.vabId << ins.soundId;
  }
};

void execute(AnimationData& data, const MMD_Animation::Instruction& instruction)
{
  AnimInstructionExecutor executor{data};
  executor.apply(instruction);
}

std::optional<int> get_timecode(const MMD_Animation::Instruction& instruction)
{
  if (std::holds_alternative<MMD_Animation::KeyframeInstruction>(instruction))
  {
    return std::get<MMD_Animation::KeyframeInstruction>(instruction).timecode;
  }
  else if (std::holds_alternative<MMD_Animation::LoopEndInstruction>(instruction))
  {
    return std::get<MMD_Animation::LoopEndInstruction>(instruction).timecode;
  }
  else if (std::holds_alternative<MMD_Animation::PlaySoundInstruction>(instruction))
  {
    return std::get<MMD_Animation::PlaySoundInstruction>(instruction).timecode;
  }
  else if (std::holds_alternative<MMD_Animation::TextureInstruction>(instruction))
  {
    return std::get<MMD_Animation::TextureInstruction>(instruction).timecode;
  }

  return std::nullopt;
}

AnimationPlayer::AnimationPlayer(CharacterModel& model, QObject* parent)
    : QObject(parent)
{
  m_animationData.model = &model;

  m_timer = new QTimer(this);
  m_timer->setSingleShot(false);
  m_timer->setInterval(50);
  connect(m_timer, &QTimer::timeout, this, &AnimationPlayer::onTimerTimeout);
}

void AnimationPlayer::playAnimation(int index)
{
  const std::vector<MMD_Animation>& anims = m_animationData.model->animations;
  if (index < 0 || index >= anims.size())
  {
    return;
  }

  playAnimation(anims[index]);
}

void AnimationPlayer::playAnimation(const MMD_Animation& animation)
{
  m_animationData.model->setupAnimation(animation);

  m_animationData.animation = animation;

  m_animationData.state = AnimationState();
  m_animationData.state.momentumData.resize(m_animationData.model->nodes.size());
  for (MomentumData& momentum : m_animationData.state.momentumData)
  {
    std::fill(momentum.values.begin(), momentum.values.end(), 0.f);
  }

  m_timer->start();
}

void AnimationPlayer::onTimerTimeout()
{
  step();
}

void AnimationPlayer::step()
{
  applyMomentum();

  AnimationState& state = m_animationData.state;
  const MMD_Animation& animation = m_animationData.animation;

  state.frameNum += 1;
  state.timecode += 1;

  // careful: the timecode may be modified by a loop (end) instruction!
  int& timecode = state.timecode;

  while (state.pc < animation.instructions.size()
         && get_timecode(animation.instructions[state.pc]).value_or(timecode) == timecode)
  {
    execute(m_animationData, animation.instructions[state.pc]);
    ++state.pc;
  }

  Q_EMIT stepped();

  if (state.pc == animation.instructions.size())
  {
    m_timer->stop();
    Q_EMIT finished();
  }
}

static QVector3D get_position(const AnimationMomentumData& momentum)
{
  return QVector3D(momentum.values[static_cast<int>(MMD_Animation::Axis::POS_X)],
                   momentum.values[static_cast<int>(MMD_Animation::Axis::POS_Y)],
                   momentum.values[static_cast<int>(MMD_Animation::Axis::POS_Z)]);
}

static QVector3D get_scale(const AnimationMomentumData& momentum)
{
  return QVector3D(momentum.values[static_cast<int>(MMD_Animation::Axis::SCALE_X)],
                   momentum.values[static_cast<int>(MMD_Animation::Axis::SCALE_Y)],
                   momentum.values[static_cast<int>(MMD_Animation::Axis::SCALE_Z)]);
}

static EulerAngles get_rotation(const AnimationMomentumData& momentum)
{
  return EulerAngles(momentum.values[static_cast<int>(MMD_Animation::Axis::ROT_X)],
                     momentum.values[static_cast<int>(MMD_Animation::Axis::ROT_Y)],
                     momentum.values[static_cast<int>(MMD_Animation::Axis::ROT_Z)]);
}

void AnimationPlayer::applyMomentum()
{
  AnimationState& state = m_animationData.state;
  CharacterModel& model = *m_animationData.model;

  for (size_t i(0); i < state.momentumData.size(); ++i)
  {
    MomentumData& momentum = state.momentumData[i];
    Object3D& node = *model.nodes[i];

    node.setPosition(node.position() + get_position(momentum));
    node.setScale(node.scale() + get_scale(momentum));
    node.setRotation(node.rotation() + get_rotation(momentum));
  }
}
