// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QFile>
#include <QString>

#include <string>

#include "gamedata.h"

#include "formats/tim.h"

#include "buffer.h"
#include "readfile.h"

class GameReader
{
public:
  GameData result;

public:
  void readPSX_EXE(const QString& filepath)
  {
    QFile file{filepath};
    if (!file.open(QIODevice::ReadOnly))
    {
      return;
    }

    const GameInfo gameinfo = SLUS_DATA;
    result.info = gameinfo;

    QByteArray bytes = file.readAll();

    this->result.sourceDir = std::filesystem::path(filepath.toStdString()).parent_path();
    auto alltims = read_all(this->result.sourceDir / ALLTIM_PATH);
    constexpr size_t ALL_TIMS_STRIDE = 0x4800;
    Buffer tims{alltims.data(), alltims.size()};

    this->result.characters.clear();

    using DigimonFileName = char[8];
    DigimonFileName* names = reinterpret_cast<DigimonFileName*>(bytes.data() + gameinfo.nameOffset);
    CharacterInfo* info = reinterpret_cast<CharacterInfo*>(bytes.data()
                                                           + gameinfo.characterDataOffset);
    uint32_t* skelOffset = reinterpret_cast<uint32_t*>(bytes.data() + gameinfo.skelOffset);

    constexpr int DIGIMON_COUNT = 180;
    for (int i(0); i < DIGIMON_COUNT; ++i)
    {
      SkeletonNodeRel* skeletonOffset = reinterpret_cast<SkeletonNodeRel*>(
          bytes.data() + skelOffset[i] - 0x80090000);
      int32_t boneCount = info[i].boneCount;

      CharacterEntry entry;
      entry.index = i;
      entry.filename = std::string(names[i]);

      for (int32_t j = 0; j < boneCount; j++)
        entry.skeleton.push_back(skeletonOffset[j]);

      tims.seek(i * ALL_TIMS_STRIDE);
      entry.texture = TimImage(&tims);

      this->result.characters.push_back(entry);
    }
  }
};
