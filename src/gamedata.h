// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "formats/tim.h"

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

constexpr const char* ALLTIM_PATH = "CHDAT/ALLTIM.TIM";

struct GameInfo
{
  std::string_view exeName;
  uint32_t nameOffset;
  uint32_t characterDataOffset;
  uint32_t skelOffset;
  bool isPAL;
};

constexpr GameInfo SLUS_DATA = {
    .exeName = "SLUS_010.32",
    .nameOffset = 0xa3b44,
    .characterDataOffset = 0x9ceb4,
    .skelOffset = 0x8ce60,
    .isPAL = false,
};

struct CharacterInfo
{
  char name[20]; // this field is not present in the PAL versions of the game
  int32_t boneCount;
  int16_t radius;
  int16_t height;
  uint8_t type;
  uint8_t level;
  uint8_t special[3];
  uint8_t dropItem;
  uint8_t dropChance;
  int8_t moves[16];
  uint8_t padding;
};

struct SkeletonNodeRel
{
  uint8_t object;
  uint8_t parent;
};

struct CharacterEntry
{
  int index;
  std::string filename;
  std::vector<SkeletonNodeRel> skeleton;
  TimImage texture;
};

struct GameData
{
  std::filesystem::path sourceDir;
  GameInfo info;
  std::vector<CharacterEntry> characters;
};
