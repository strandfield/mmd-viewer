// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "tmd.h"

#include "datastream.h"
#include "readfile.h"

#include <cassert>
#include <cmath>

bool TMD_Reader::readModel(const std::filesystem::path& filePath, TMD_Model& outputModel)
{
  if (!std::filesystem::is_regular_file(filePath))
  {
    return false;
  }

  std::vector<uint8_t> bytes = read_all(filePath);
  Buffer buffer{bytes.data(), bytes.size()};
  return readModel(buffer, outputModel);
}

bool TMD_Reader::readModel(Buffer& buffer, TMD_Model& outputModel)
{
  const auto header = readbuf<tmd_header_t>(buffer);
  if (header.id != 0x41)
  {
    return false;
  }

  const int64_t objheaders_offset = buffer.pos();

  auto objectheaders = std::vector<tmd_object_header_t>(header.num_objects);
  for (tmd_object_header_t& objheader : objectheaders)
  {
    objheader = readbuf<tmd_object_header_t>(buffer);
  }

  outputModel.objects().clear();

  for (const tmd_object_header_t& objheader : objectheaders)
  {
    TMD_Object& object = outputModel.objects().emplace_back();
    object.setScale(objheader.scale);

    const auto* vertex_ptr = reinterpret_cast<tmd_vertex_t*>(buffer.data() + objheaders_offset
                                                             + objheader.vertex_offset);
    object.vertices().assign(vertex_ptr, vertex_ptr + objheader.vertex_count);

    const auto* normals_ptr = reinterpret_cast<tmd_normal_t*>(buffer.data() + objheaders_offset
                                                              + objheader.normal_offset);
    object.normals().assign(normals_ptr, normals_ptr + objheader.normal_count);

    buffer.seek(objheaders_offset + objheader.primitive_offset);
    for (uint32_t i(0); i < objheader.primitive_count; ++i)
    {
      const auto* primitive = reinterpret_cast<tmd_primitive_packet_t*>(buffer.data() + buffer.pos());
      object.primitives().append(primitive);
      buffer.skip(get_primitive_packet_size(primitive));
    }
  }

  return true;
}
