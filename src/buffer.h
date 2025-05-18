// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <cstdint>
#include <cstring>
#include <iterator>

class Buffer
{
private:
  uint8_t* m_data;
  size_t m_size;
  uint8_t* m_ptr;

public:
  Buffer(uint8_t* data, size_t size)
      : m_data(data)
      , m_size(size)
      , m_ptr(m_data)
  {}

  uint8_t* data() const { return m_data; }
  int64_t size() const { return m_size; }
  int64_t pos() const { return std::distance(m_data, m_ptr); }
  void seek(int64_t pos) { m_ptr = std::next(m_data, pos); }
  void skip(int64_t bytes) { seek(pos() + bytes); }
  int64_t bytesAvailable() const { return size() - pos(); }
  bool atEnd() const { return pos() == size(); }

  int64_t peek(uint8_t* data, int64_t maxSize) const
  {
    int64_t r = std::min(size() - pos(), maxSize);
    std::memcpy(data, m_ptr, r);
    return r;
  }

  int64_t read(uint8_t* data, int64_t maxSize)
  {
    int64_t r = std::min(size() - pos(), maxSize);
    std::memcpy(data, m_ptr, r);
    m_ptr += r;
    return r;
  }
};

template<typename T>
T peekbuf(const Buffer& buffer)
{
  T result;
  buffer.peek(reinterpret_cast<uint8_t*>(&result), sizeof(T));
  return result;
}

template<typename T>
T readbuf(Buffer& buffer)
{
  T result;
  buffer.read(reinterpret_cast<uint8_t*>(&result), sizeof(T));
  return result;
}
