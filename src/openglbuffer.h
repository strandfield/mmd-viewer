// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

/**
 * @file openglbuffer.h
 * @brief provides helper functions to manipulate OpenGL buffers
 */

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

#include <memory>
#include <vector>

/**
 * @brief specify the data to upload to the GPU
 */
struct BufferData
{
  const GLvoid* ptr = nullptr; ///< pointer to the first byte
  GLsizei size = 0;            ///< number of bytes
};

/**
 * @brief stores vertex specifications for a buffer
 *
 * @sa glVertexAttribPointer().
 */
struct BufferSpecs
{
  QOpenGLBuffer::UsagePattern usage = QOpenGLBuffer::StaticDraw; ///< usage pattern
  GLuint index;                                                  ///< index in the vao
  GLint tuplesize = 1;                                           ///< number of values per point
  GLenum type = GL_FLOAT;                                        ///< type of each value
  GLboolean normalized = GL_FALSE;                               ///< whether non-float input should be normalized
  GLsizei stride = 0;                                            ///< offset in the buffer between two tuples
  const GLvoid* offset = nullptr;                                ///< offset of the first tuple in the buffer
};

/**
 * @brief helper class for building BufferSpecs
 *
 * This class allows the use of method chaining (a.k.a. fluent interface) to
 * build a complex BufferSpecs in a single line.
 */
class BufferSpecsBuilder
{
public:
  BufferSpecs specs;

public:
  BufferSpecsBuilder() = default;

  BufferSpecsBuilder& index(GLuint val) { specs.index = val; return *this; }
  BufferSpecsBuilder& tuplesize(GLuint val) { specs.tuplesize = val; return *this; }
  BufferSpecsBuilder& type(GLenum val) { specs.type = val; return *this; }
  BufferSpecsBuilder& normalized(GLboolean val) { specs.normalized = val; return *this; }
  BufferSpecsBuilder& stride(GLsizei val) { specs.stride = val; return *this; }
  BufferSpecsBuilder& offset(const GLvoid* val) { specs.offset = val; return *this; }

  operator BufferSpecs() const { return specs; }
};

/**
 * @brief builds a BufferData struct from the whole content of a vector
 * @param v  the vector
 *
 * @note although this function takes the input vector as a const reference
 * (because the data is not modified), care must be taken not to pass a
 * temporary as the resulting BufferData holds a pointer to the vector
 * internal data().
 */
template<typename T>
BufferData buffer_data_from_vector(const std::vector<T>& v)
{
  BufferData result;
  result.ptr = v.data();
  result.size = static_cast<GLsizei>(v.size() * sizeof(T));
  return result;
}

/**
 * @brief setup a GL buffer
 * @param ptr    unique pointer that will take ownership of the QOpenGLBuffer
 * @param gl     pointer to the OpenGL functions
 * @param data   the buffer data
 * @param specs  specifications for the buffer
 *
 * The @a specs are used to bind the buffer to the currently bound VAO.
 */
inline void setup_buffer(std::unique_ptr<QOpenGLBuffer>& ptr,
                         QOpenGLFunctions* gl,
                         BufferData data,
                         BufferSpecs specs)
{
  ptr = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
  auto* vbo = ptr.get();

  vbo->create();
  assert(vbo->isCreated());

  vbo->bind();
  vbo->setUsagePattern(specs.usage);
  vbo->allocate(data.ptr, data.size);

  gl->glVertexAttribPointer(
    specs.index, specs.tuplesize, specs.type, specs.normalized, specs.stride, specs.offset);

  vbo->release();

  gl->glEnableVertexAttribArray(specs.index);
}

inline void setup_index_buffer(std::unique_ptr<QOpenGLBuffer>& ptr,
                               QOpenGLFunctions* gl,
                               BufferData data,
                               QOpenGLBuffer::UsagePattern usage = QOpenGLBuffer::StaticDraw)
{
  ptr = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
  auto* vbo = ptr.get();

  vbo->create();
  assert(vbo->isCreated());

  vbo->bind();
  vbo->setUsagePattern(usage);
  vbo->allocate(data.ptr, data.size);
}

//inline void setup_instanced_buffer(std::unique_ptr<QOpenGLBuffer>& ptr,
//                                   QOpenGLFunctions* gl,
//                                   BufferData data,
//                                   BufferSpecs specs)
//{
//    ptr = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
//    auto* vbo = ptr.get();
//
//    vbo->create();
//    assert(vbo->isCreated());
//
//    vbo->bind();
//    vbo->setUsagePattern(specs.usage);
//    vbo->allocate(data.ptr, data.size);
//
//    gl->glVertexAttribPointer(
//      specs.index, specs.tuplesize, specs.type, specs.normalized, specs.stride, specs.offset);
//
//    gl->glVertexAttribDivisor(specs.index, 1);
//
//    vbo->release();
//
//    gl->glEnableVertexAttribArray(specs.index);
//}

//inline void setup_instanced_matrix_buffer(std::unique_ptr<QOpenGLBuffer>& ptr,
//                                          QOpenGLFunctions* gl,
//                                          BufferData data,
//                                          int index,
//                                          QOpenGLBuffer::UsagePattern usage
//                                          = QOpenGLBuffer::DynamicDraw)
//{
//    ptr = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
//    auto* vbo = ptr.get();
//
//    vbo->create();
//    assert(vbo->isCreated());
//
//    vbo->bind();
//    vbo->setUsagePattern(usage);
//    vbo->allocate(data.ptr, data.size);
//
//    constexpr size_t vec4size = 4 * sizeof(float);
//
//    gl->glVertexAttribPointer(index + 0, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, nullptr);
//    gl->glVertexAttribPointer(index + 1, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (GLvoid*)(vec4size));
//    gl->glVertexAttribPointer(
//      index + 2, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (GLvoid*)(vec4size * 2));
//    gl->glVertexAttribPointer(
//      index + 3, 4, GL_FLOAT, GL_FALSE, 4 * vec4size, (GLvoid*)(vec4size * 3));
//
//    constexpr bool instanced = true;
//
//    if (instanced)
//    {
//        gl->glVertexAttribDivisor(index + 0, 1);
//        gl->glVertexAttribDivisor(index + 1, 1);
//        gl->glVertexAttribDivisor(index + 2, 1);
//        gl->glVertexAttribDivisor(index + 3, 1);
//    }
//
//    vbo->release();
//
//    gl->glEnableVertexAttribArray(index);
//    gl->glEnableVertexAttribArray(index + 1);
//    gl->glEnableVertexAttribArray(index + 2);
//    gl->glEnableVertexAttribArray(index + 3);
//}

/**
 * @brief change the data of a buffer
 * @param ptr   pointer to the QOpenGLBuffer
 * @param data  the new buffer data
 */
inline void update_buffer(std::unique_ptr<QOpenGLBuffer>& ptr, BufferData data)
{
  auto* vbo = ptr.get();
  vbo->bind();
  vbo->allocate(data.ptr, data.size);
  vbo->release();
}
