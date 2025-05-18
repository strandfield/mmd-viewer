// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef MMD_H
#define MMD_H

#include "tmd.h"

#include <array>
#include <cassert>
#include <filesystem>
#include <variant>
#include <vector>

struct MMD_FileHeader
{
  uint32_t tmdOffset;
  uint32_t animationsOffset;
};

class MMD_Animation
{
public:
  struct Position
  {
    int16_t scaleX = 0x1000;
    int16_t scaleY = 0x1000;
    int16_t scaleZ = 0x1000;
    int16_t posX = 0;
    int16_t posY = 0;
    int16_t posZ = 0;
    int16_t rotX = 0;
    int16_t rotY = 0;
    int16_t rotZ = 0;
  };

  enum class Axis {
    SCALE_X,
    SCALE_Y,
    SCALE_Z,

    ROT_X,
    ROT_Y,
    ROT_Z,

    POS_X,
    POS_Y,
    POS_Z,
  };

  struct KeyframeEntry
  {
    // TODO: revoir comment exposer cette information,
    // en l'état l'enum Axis n'est pas cohérente avec ce bitfield
    // uint16_t enabledAxis;
    uint8_t affectedNode;
    uint16_t scale;
    std::vector<std::pair<Axis, float>> values;
  };

  struct KeyframeInstruction
  {
    uint32_t timecode;
    std::vector<KeyframeEntry> entries;
  };

  struct LoopStartInstruction
  {
    uint32_t loopCount;
  };

  struct LoopEndInstruction
  {
    uint32_t timecode;
    uint32_t newTime;
  };

  struct PlaySoundInstruction
  {
    uint32_t timecode;
    uint8_t vabId;
    uint8_t soundId;
  };

  struct TextureInstruction
  {
    uint32_t timecode;
    uint8_t srcX;
    uint8_t srcY;
    uint8_t width;
    uint8_t height;
    uint8_t destX;
    uint8_t destY;
  };

  using Instruction = std::variant<KeyframeInstruction,
                                   LoopStartInstruction,
                                   LoopEndInstruction,
                                   PlaySoundInstruction,
                                   TextureInstruction>;

  uint32_t frameCount = 0;
  uint32_t id;
  std::vector<Position> initialPositions;
  std::vector<Instruction> instructions;

  explicit MMD_Animation(uint32_t id = -1)
      : id(id)
  {}
};

class MMD_Animations
{
private:
  std::vector<uint8_t> m_animationData;

public:
  MMD_Animations() = default;
  explicit MMD_Animations(Buffer& buffer);

  int count() const;
  std::vector<MMD_Animation> decode(size_t boneCount) const;
};

class MMD_File
{
public:
  MMD_FileHeader header;
  TMD_Model tmd;
  MMD_Animations animations;

  bool open(Buffer* buffer);
  bool open(const std::filesystem::path& filePath);
  bool open(const std::filesystem::path& gameDirectory,
            int characterId,
            const std::string& filename);
};

#endif // MMD_H
