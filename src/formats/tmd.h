// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef TMD_H
#define TMD_H

#include "typedefs.h"

#include "buffer.h"

#include <array>
#include <cassert>
#include <filesystem>
#include <vector>

typedef struct
{
  u32 id;
  u32 flags;
  u32 num_objects;
} tmd_header_t;

typedef struct
{
  u32 vertex_offset;
  u32 vertex_count;
  u32 normal_offset;
  u32 normal_count;
  u32 primitive_offset;
  u32 primitive_count;
  s32 scale;
} tmd_object_header_t;

typedef struct
{
  u8 olen;
  u8 ilen;
  u8 flag; // MSB [00000, GRD, FCE, LGT] LSB, see TMD_Flag
  u8 mode; // MSB [TMD_Code (3 bits), options (5 bits)] LSB
} tmd_primitive_header_t;

enum TMD_Flag {
  TMD_Flag_LGT = 1, // if set, light source calculation is disabled
  TMD_Flag_FCE = 2, // if set, the polygon is double faced
  TMD_Flag_GRD = 4, // if set, the polygon is gradated
};

enum TMD_Code {
  TMD_Code_INVALID = 0,
  TMD_Code_POLYGON = 1,
  TMD_Code_LINE = 2,
  TMD_Code_SPRITE = 3,
};

enum TMD_ModeOption {
  TMD_ModeOption_TGE = 1, // brightness
  TMD_ModeOption_ABE = 2, // translucency
  TMD_ModeOption_TME = 4, // texture
  TMD_ModeOption_QUAD = 8,
  TMD_ModeOption_IIP = 16, // gouraud shading
};

typedef struct
{
  s16 x;
  s16 y;
  s16 z;
  s16 zero;
} tmd_vertex_t;

typedef struct
{
  s16 x;
  s16 y;
  s16 z;
  s16 zero;
} tmd_normal_t;

typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
} tmd_color_t;

typedef struct
{
  uint8_t u;
  uint8_t v;
} tmd_uv_coord_t;

typedef struct
{
  uint16_t w;
  uint16_t h;
} tmd_sprite_size_t;

typedef struct
{
  tmd_primitive_header_t header;
} tmd_primitive_packet_t;

// returns the size, in bytes, of a primitive packet
inline size_t get_primitive_packet_size(const tmd_primitive_packet_t* primitive)
{
  return sizeof(tmd_primitive_header_t) + primitive->header.ilen * sizeof(uint32_t);
}

inline TMD_Code extract_code_from_mode(uint8_t mode)
{
  return static_cast<TMD_Code>(mode >> 5);
}

inline TMD_Code get_primitive_code(const tmd_primitive_packet_t* primitive)
{
  return extract_code_from_mode(primitive->header.mode);
}

struct TMD_TextureInfo
{
  uint8_t page : 5;
  uint8_t mixtureRate : 2;
  uint8_t colorMode : 2;
};

inline int get_textureinfo_bpp(const TMD_TextureInfo& textureInfo)
{
  switch (textureInfo.colorMode)
  {
  case 0:
    return 4;
  case 1:
    return 8;
  case 2:
    return 16;
  default:
    assert(false);
    return -1;
  }
}

struct TMD_CLUT_Info
{
  uint16_t clutX : 6;
  uint16_t clutY : 9;
};

struct TMD_FlagBitfield
{
  uint8_t isLightSourceDisabled : 1;
  uint8_t isDoubleFaced : 1;
  uint8_t isGradated : 1;
  uint8_t unused : 5;
};

struct TMD_ModeBitfield
{
  uint8_t hasBrightness : 1;
  uint8_t hasTranslucency : 1;
  uint8_t hasTexture : 1;
  uint8_t isQuad : 1;
  uint8_t isGouraud : 1;
  uint8_t code : 3;
};

struct TMD_ModeSpriteBitfield
{
  uint8_t zero : 1;
  uint8_t hasTranslucency : 1;
  uint8_t one : 1;
  uint8_t size : 2;
  uint8_t code : 3;
};

class TMD_Object;

class TMD_Primitive
{
private:
  std::array<uint16_t, 4> m_vertices;
  std::array<uint16_t, 4> m_normals;
  std::array<tmd_color_t, 4> m_colors;
  std::array<tmd_uv_coord_t, 4> m_uvs;
  TMD_TextureInfo m_textureInfo = {};
  TMD_CLUT_Info m_clutInfo = {};
  tmd_sprite_size_t m_spriteSize = {};
  uint8_t m_flags;
  uint8_t m_mode;
  struct
  {
    uint16_t vertexCount : 3;
    uint16_t normalCount : 3;
    uint16_t colorCount : 3;
  } m_ = {0, 0, 0};

public:
  explicit TMD_Primitive(const tmd_primitive_packet_t* packet)
      : m_flags(packet->header.flag)
      , m_mode(packet->header.mode)
  {
    auto flags = *reinterpret_cast<TMD_FlagBitfield*>(&m_flags);

    Buffer buffer{reinterpret_cast<uint8_t*>(const_cast<tmd_primitive_packet_t*>(packet)),
                  get_primitive_packet_size(packet)};
    buffer.skip(sizeof(tmd_primitive_header_t));

    if (getCode() == TMD_Code_POLYGON)
    {
      assert(packet->header.ilen <= 0x0A);

      auto mode = *reinterpret_cast<TMD_ModeBitfield*>(&m_mode);

      m_.vertexCount = mode.isQuad ? 4 : 3;
      m_.normalCount = flags.isLightSourceDisabled || mode.hasBrightness ? 0
                       : mode.isGouraud                                  ? m_.vertexCount
                                                                         : 1;
      if (flags.isGradated)
        m_.colorCount = m_.vertexCount;
      else if (flags.isLightSourceDisabled)
        m_.colorCount = mode.isGouraud ? m_.vertexCount : 1;
      else if (!mode.hasTexture)
        m_.colorCount = 1;

      if (mode.hasTexture)
      {
        m_uvs[0] = readbuf<tmd_uv_coord_t>(buffer);
        m_clutInfo = readbuf<TMD_CLUT_Info>(buffer);
        m_uvs[1] = readbuf<tmd_uv_coord_t>(buffer);
        m_textureInfo = readbuf<TMD_TextureInfo>(buffer);
        m_uvs[2] = readbuf<tmd_uv_coord_t>(buffer);
        buffer.skip(2);
        if (m_.vertexCount == 4)
        {
          m_uvs[3] = readbuf<tmd_uv_coord_t>(buffer);
          buffer.skip(2);
        }
      }

      for (uint32_t i = 0u; i < m_.colorCount; i++)
      {
        m_colors[i] = readbuf<tmd_color_t>(buffer);
        buffer.skip(1);
      }

      for (uint32_t i = 0u; i < m_.vertexCount; i++)
      {
        if (i < m_.normalCount)
        {
          m_normals[i] = readbuf<uint16_t>(buffer);
        }
        m_vertices[i] = readbuf<uint16_t>(buffer);
      }
    }
    else if (getCode() == TMD_Code_LINE)
    {
      assert(packet->header.ilen == 0x02 || packet->header.ilen == 0x03);

      auto mode = *reinterpret_cast<TMD_ModeBitfield*>(&m_mode);
      assert(m_flags == 0x01); // light calculation disabled

      m_.colorCount = 0;
      m_colors[m_.colorCount++] = readbuf<tmd_color_t>(buffer);
      buffer.skip(1);

      // the gouraud flag is to be interpreted as gradation for lines
      const bool isGradated = mode.isGouraud;
      if (isGradated)
      {
        m_colors[m_.colorCount++] = readbuf<tmd_color_t>(buffer);
        buffer.skip(1);
      }

      m_.normalCount = 0;
      m_.vertexCount = 2;
      m_vertices[0] = readbuf<uint16_t>(buffer);
      m_vertices[1] = readbuf<uint16_t>(buffer);
    }
    else
    {
      assert(getCode() == TMD_Code_SPRITE);
      assert(packet->header.ilen == 0x02 || packet->header.ilen == 0x03);

      m_.vertexCount = 1;
      m_vertices[0] = readbuf<uint16_t>(buffer);
      m_textureInfo = readbuf<TMD_TextureInfo>(buffer);
      m_uvs[0] = readbuf<tmd_uv_coord_t>(buffer);
      m_clutInfo = readbuf<TMD_CLUT_Info>(buffer);

      auto mode = *reinterpret_cast<TMD_ModeSpriteBitfield*>(&m_mode);

      if (mode.size == 0b01)
      {
        m_spriteSize.w = m_spriteSize.h = 1;
      }
      else if (mode.size == 0b10)
      {
        m_spriteSize.w = m_spriteSize.h = 8;
      }
      else if (mode.size == 0b11)
      {
        m_spriteSize.w = m_spriteSize.h = 16;
      }
      else
      {
        assert(mode.size == 0);
        assert(packet->header.ilen == 0x03);
        m_spriteSize = readbuf<tmd_sprite_size_t>(buffer);
      }
    }
  }

  uint8_t flags() const { return m_flags; }
  uint8_t mode() const { return m_mode; }
  TMD_Code getCode() const { return extract_code_from_mode(m_mode); }

  int vertexCount() const { return m_.vertexCount; }
  int normalCount() const { return m_.normalCount; }
  int colorCount() const { return m_.colorCount; }

  bool hasTexture() const
  {
    return getCode() == TMD_Code_POLYGON
           && reinterpret_cast<const TMD_ModeBitfield*>(&m_mode)->hasTexture;
  }

  TMD_TextureInfo textureInfo() const { return m_textureInfo; }
  TMD_CLUT_Info clutInfo() const { return m_clutInfo; }

  uint16_t* vertexBuf() { return &m_vertices[0]; }
  uint16_t* normals() { return &m_normals[0]; }
  tmd_color_t* colors() { return &m_colors[0]; }

  const uint16_t* vertexBuf() const { return &m_vertices[0]; }
  const uint16_t* normals() const { return &m_normals[0]; }
  const tmd_color_t* colors() const { return &m_colors[0]; }
  const tmd_uv_coord_t* uvs() const { return &m_uvs[0]; }
};

class TMD_PrimitiveList
{
public:
  int count() const;
  tmd_primitive_packet_t* at(int n);
  const tmd_primitive_packet_t* at(int n) const;

  void append(const tmd_primitive_packet_t* primitive);

  tmd_primitive_packet_t& operator[](int n);

private:
  std::vector<u32> m_packets_data;
  std::vector<size_t> m_primitive_offsets;
};

class TMD_Object
{
public:
  s32 scale() const;
  void setScale(s32 scale);

  std::vector<tmd_vertex_t>& vertices();
  const std::vector<tmd_vertex_t>& vertices() const;

  std::vector<tmd_normal_t>& normals();
  const std::vector<tmd_normal_t>& normals() const;

  TMD_PrimitiveList& primitives();
  const TMD_PrimitiveList& primitives() const;

private:
  s32 m_scale;
  std::vector<tmd_vertex_t> m_vertices;
  std::vector<tmd_normal_t> m_normals;
  TMD_PrimitiveList m_primitives;
};

class TMD_Model
{
public:
  std::vector<TMD_Object>& objects();
  const std::vector<TMD_Object>& objects() const;

private:
  std::vector<TMD_Object> m_objects;
};

class TMD_Reader
{
public:
  bool readModel(const std::filesystem::path& filePath, TMD_Model& outputModel);
  bool readModel(Buffer& buffer, TMD_Model& outputModel);
};

// TMD_PrimitiveList

inline int TMD_PrimitiveList::count() const
{
  return (int) m_primitive_offsets.size();
}

inline tmd_primitive_packet_t* TMD_PrimitiveList::at(int n)
{
  size_t offset = m_primitive_offsets.at(n);
  return reinterpret_cast<tmd_primitive_packet_t*>(m_packets_data.data() + offset);
}

inline const tmd_primitive_packet_t* TMD_PrimitiveList::at(int n) const
{
  size_t offset = m_primitive_offsets.at(n);
  return reinterpret_cast<const tmd_primitive_packet_t*>(m_packets_data.data() + offset);
}

inline void TMD_PrimitiveList::append(const tmd_primitive_packet_t* primitive)
{
  m_primitive_offsets.push_back(m_packets_data.size());
  const auto* packet_data = reinterpret_cast<const uint32_t*>(primitive);

  static_assert(sizeof(tmd_primitive_header_t) == sizeof(uint32_t), "packet header must be 4 bytes");
  m_packets_data.insert(m_packets_data.end(),
                        packet_data,
                        packet_data + (primitive->header.ilen + 1));
}

inline tmd_primitive_packet_t& TMD_PrimitiveList::operator[](int n)
{
  return *reinterpret_cast<tmd_primitive_packet_t*>(m_packets_data.data() + n);
}

// TMD_Object

inline s32 TMD_Object::scale() const
{
  return m_scale;
}

inline void TMD_Object::setScale(s32 scale)
{
  m_scale = scale;
}

inline std::vector<tmd_vertex_t>& TMD_Object::vertices()
{
  return m_vertices;
}

inline const std::vector<tmd_vertex_t>& TMD_Object::vertices() const
{
  return m_vertices;
}

inline std::vector<tmd_normal_t>& TMD_Object::normals()
{
  return m_normals;
}

inline const std::vector<tmd_normal_t>& TMD_Object::normals() const
{
  return m_normals;
}

inline TMD_PrimitiveList& TMD_Object::primitives()
{
  return m_primitives;
}

inline const TMD_PrimitiveList& TMD_Object::primitives() const
{
  return m_primitives;
}

// TMD_Model

inline std::vector<TMD_Object>& TMD_Model::objects()
{
  return m_objects;
}

inline const std::vector<TMD_Object>& TMD_Model::objects() const
{
  return m_objects;
}

#endif // TMD_H
