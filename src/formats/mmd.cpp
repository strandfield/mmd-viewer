// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "mmd.h"

#include "datastream.h"
#include "readfile.h"

#include <cassert>
#include <cmath>
#include <format>

#include <iostream>

size_t get_number_of_animations(const Buffer& buffer)
{
  // format for N animations:
  // - N (relative) offsets to the animations (uint32_t)
  // - N animations data (variable-size).
  // since the first offset points to the first animation,
  // and that animation starts just after the last animation
  // offset, we can deduce the number of animations from that
  // first offset.
  // first offset = number of animations offset * sizeof(uint32_t)

  return peekbuf<uint32_t>(buffer) / sizeof(uint32_t);
}

MMD_Animation::KeyframeEntry read_keyframe_entry(Buffer& buffer)
{
  MMD_Animation::KeyframeEntry result;

  const uint16_t instruction = readbuf<uint16_t>(buffer);
  const uint16_t enabledAxis = (instruction & 0x7FC0) >> 6;
  result.affectedNode = instruction & 0x3F;

  // in TS caharacter viewer, "scale" is called "duration" and
  // the "values" aren't divided by "duration".
  result.scale = readbuf<uint16_t>(buffer);

  for (int32_t i = 8; i >= 0; i--)
  {
    if ((enabledAxis & (1 << i)) == 0)
      continue;
    result.values.push_back(std::make_pair(static_cast<MMD_Animation::Axis>(8 - i),
                                           readbuf<int16_t>(buffer) / (float) result.scale));
  }

  return result;
}

bool fill_instruction(Buffer& buffer,
                      uint16_t header,
                      MMD_Animation::KeyframeInstruction& instruction)
{
  instruction.timecode = header & 0x0FFF;

  while (peekbuf<uint16_t>(buffer) & 0x8000)
  {
    instruction.entries.push_back(read_keyframe_entry(buffer));
  }

  return true;
}

bool fill_instruction(Buffer& buffer,
                      uint16_t header,
                      MMD_Animation::LoopStartInstruction& instruction)
{
  instruction.loopCount = header & 0x00FF;
  return true;
}

bool fill_instruction(Buffer& buffer,
                      uint16_t header,
                      MMD_Animation::LoopEndInstruction& instruction)
{
  instruction.timecode = header & 0x0FFF;
  instruction.newTime = readbuf<uint16_t>(buffer);
  return true;
}

bool fill_instruction(Buffer& buffer,
                      uint16_t header,
                      MMD_Animation::PlaySoundInstruction& instruction)
{
  instruction.timecode = header & 0x0FFF;
  instruction.soundId = readbuf<uint8_t>(buffer);
  instruction.vabId = readbuf<uint8_t>(buffer);
  return true;
}

bool fill_instruction(Buffer& buffer,
                      uint16_t header,
                      MMD_Animation::TextureInstruction& instruction)
{
  instruction.timecode = header & 0x0FFF;
  instruction.srcY = readbuf<uint8_t>(buffer);
  instruction.srcX = readbuf<uint8_t>(buffer);
  instruction.height = readbuf<uint8_t>(buffer);
  instruction.width = readbuf<uint8_t>(buffer);
  instruction.destY = readbuf<uint8_t>(buffer);
  instruction.destX = readbuf<uint8_t>(buffer);
  return true;
}

bool fill_animation(Buffer& buffer, size_t boneCount, MMD_Animation& animation)
{
  if (buffer.bytesAvailable() < sizeof(uint16_t))
  {
    return false;
  }

  const auto header = readbuf<uint16_t>(buffer);
  const bool hasScale = header & 0x8000;
  animation.frameCount = header & 0x7FFF;

  animation.initialPositions.emplace_back();

  for (auto i = 1u; i < boneCount; i++)
  {
    const size_t number_of_fiels_to_read = hasScale ? 9 : 6;
    if (buffer.bytesAvailable() < number_of_fiels_to_read * sizeof(int16_t))
    {
      return false;
    }

    MMD_Animation::Position pos;
    if (hasScale)
    {
      pos.scaleX = readbuf<int16_t>(buffer);
      pos.scaleY = readbuf<int16_t>(buffer);
      pos.scaleZ = readbuf<int16_t>(buffer);
    }
    pos.rotX = readbuf<int16_t>(buffer);
    pos.rotY = readbuf<int16_t>(buffer);
    pos.rotZ = readbuf<int16_t>(buffer);
    pos.posX = readbuf<int16_t>(buffer);
    pos.posY = readbuf<int16_t>(buffer);
    pos.posZ = readbuf<int16_t>(buffer);

    animation.initialPositions.push_back(pos);
  }

  for (;;)
  {
    if (buffer.bytesAvailable() < sizeof(uint16_t))
    {
      return false;
    }

    uint16_t instruction_header = readbuf<uint16_t>(buffer);

    if (instruction_header == 0x0000)
    {
      break; // end of animation data
    }

    switch (instruction_header & 0xF000)
    {
    case 0x0000: // keyframe
    {
      MMD_Animation::KeyframeInstruction instruction;
      if (!fill_instruction(buffer, instruction_header, instruction))
      {
        return false;
      }
      animation.instructions.push_back(std::move(instruction));
      break;
    }
    case 0x1000: // loop start
    {
      MMD_Animation::LoopStartInstruction instruction;
      if (!fill_instruction(buffer, instruction_header, instruction))
      {
        return false;
      }
      animation.instructions.push_back(std::move(instruction));
      break;
    }
    case 0x2000: // loop end
    {
      MMD_Animation::LoopEndInstruction instruction;
      if (!fill_instruction(buffer, instruction_header, instruction))
      {
        return false;
      }
      animation.instructions.push_back(std::move(instruction));
      break;
    }
    case 0x3000: // change texture
    {
      MMD_Animation::TextureInstruction instruction;
      if (!fill_instruction(buffer, instruction_header, instruction))
      {
        return false;
      }
      animation.instructions.push_back(std::move(instruction));
      break;
    }
    case 0x4000: // play sound
    {
      MMD_Animation::PlaySoundInstruction instruction;
      if (!fill_instruction(buffer, instruction_header, instruction))
      {
        return false;
      }
      animation.instructions.push_back(std::move(instruction));
      break;
    }
    }
  }

  return true;
}

void read_animations(Buffer& buffer, size_t boneCount, std::vector<MMD_Animation>& output)
{
  assert(buffer.pos() == 0 && buffer.bytesAvailable());

  const size_t anim_count = get_number_of_animations(buffer);

  const auto* offsets = reinterpret_cast<uint32_t*>(buffer.data() + buffer.pos());

  for (size_t i(0); i < anim_count; i++)
  {
    MMD_Animation& animation = output.emplace_back(i);

    const uint32_t a = offsets[i];
    if (a != 0)
    {
      buffer.seek(a);
      const bool ok = fill_animation(buffer, boneCount, animation);
      if (!ok)
      {
        output.clear();
        return;
      }
    }
  }
}

MMD_Animations::MMD_Animations(Buffer& buffer)
    : m_animationData(buffer.data() + buffer.pos(), buffer.data() + buffer.size())
{}

int MMD_Animations::count() const
{
  if (m_animationData.empty())
  {
    return 0;
  }

  Buffer buffer{const_cast<uint8_t*>(m_animationData.data()), m_animationData.size()};
  return get_number_of_animations(buffer);
}

std::vector<MMD_Animation> MMD_Animations::decode(size_t boneCount) const
{
  if (m_animationData.empty())
  {
    return {};
  }

  Buffer buffer{const_cast<uint8_t*>(m_animationData.data()), m_animationData.size()};
  std::vector<MMD_Animation> result;
  read_animations(buffer, boneCount, result);
  return result;
}

bool MMD_File::open(Buffer* buffer)
{
  this->header = readbuf<MMD_FileHeader>(*buffer);

  // read tmd
  if (this->header.tmdOffset < buffer->size())
  {
    Buffer tmd_buffer{buffer->data() + this->header.tmdOffset,
                      size_t(buffer->size() - this->header.tmdOffset)};
    TMD_Reader tmd_reader;
    tmd_reader.readModel(tmd_buffer, this->tmd);
  }
  else
  {
    return false;
  }

  // read animations
  if (this->header.animationsOffset < buffer->size())
  {
    Buffer animations_buffer{buffer->data() + this->header.animationsOffset,
                             size_t(buffer->size() - this->header.animationsOffset)};
    this->animations = MMD_Animations(animations_buffer);
  }
  else
  {
    return false;
  }

  return true;
}

bool MMD_File::open(const std::filesystem::path& filePath)
{
  if (!std::filesystem::is_regular_file(filePath))
  {
    return false;
  }

  std::vector<uint8_t> bytes = read_all(filePath);
  Buffer buffer{bytes.data(), bytes.size()};
  return open(&buffer);
}

bool MMD_File::open(const std::filesystem::path& gameDirectory,
                    int characterId,
                    const std::string& filename)
{
  std::filesystem::path modelPath = gameDirectory
                                    / std::format("CHDAT/MMD{}/{}.MMD", characterId / 30, filename);

  if (!std::filesystem::exists(modelPath))
  {
    std::cout << "File " << modelPath << " does not exist, skipping." << std::endl;
    return false;
  }

  return open(modelPath);
}
