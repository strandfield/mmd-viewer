// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "openglbuffer.h"
#include "psxobject3d.h"
#include "ubershader.h"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

class PSX_UberShader : public UberShader
{
public:
  PSX_UberShader()
      : UberShader(":/shaders/psxmodel.vert", ":/shaders/psxmodel.frag")
  {}

  struct Config
  {
    bool has_colors = false;
    bool has_uv = false;
    bool has_normals = false;
    bool vertexColors = false;
    bool hasTexture = false;
    bool lighting = false;
  };

  QOpenGLShaderProgram* getProgram(Config conf)
  {
    glsl::PreprocessorDefines defines;

    if (conf.has_colors)
    {
      defines.emplace_back("MESH_HAS_COLORS");
    }

    if (conf.has_uv)
    {
      defines.emplace_back("MESH_HAS_UV");
    }

    if (conf.has_normals)
    {
      defines.emplace_back("MESH_HAS_NORMALS");
    }

    if (conf.hasTexture)
    {
      defines.emplace_back("MATERIAL_TEXTURE");
    }

    if (conf.lighting)
    {
      defines.emplace_back("LIGHTING_ON");
    }

    auto sp = UberShader::getProgram(defines, {});
    return sp.get();
  }

  QOpenGLShaderProgram* getProgram(const PSX_Object3D& data, const PSX_Material& material)
  {
    Config conf;
    conf.has_colors = !data.colors.empty();
    conf.has_uv = !data.uv.empty();
    conf.has_normals = !data.normals.empty();
    conf.vertexColors = material.vertexColors;
    conf.hasTexture = material.map != nullptr;
    conf.lighting = material.lighting;
    return getProgram(conf);
  }
};

class OpenGLTextureManager
{
public:
  QOpenGLTexture* getTextureFor(PSX_Texture& psxTexture);

  void deleteUnreachableTextures();

private:
  using Key = PSX_Texture*;
  struct Value
  {
    int revision;
    std::unique_ptr<QOpenGLTexture> texture;
  };
  std::map<Key, Value> m_textures;

  struct PSX_Texture_Entry
  {
    Key key;
    std::weak_ptr<PSX_Texture> weakptr;
  };
  std::vector<PSX_Texture_Entry> m_entries;
};

class SceneRenderer : public QOpenGLFunctions
{
private:
  QOpenGLContext* m_context;
  PSX_UberShader m_shaders;

public:
  QMatrix4x4 projectionMatrix;
  QMatrix4x4 viewMatrix;

private:
  OpenGLTextureManager m_textures;

  QOpenGLVertexArrayObject m_vao;
  struct
  {
    std::unique_ptr<QOpenGLBuffer> vertex;
    std::unique_ptr<QOpenGLBuffer> index;
    std::unique_ptr<QOpenGLBuffer> color;
    std::unique_ptr<QOpenGLBuffer> uv;
    std::unique_ptr<QOpenGLBuffer> normal;
  } m_buffers;

public:
  explicit SceneRenderer(QOpenGLContext* ctx)
      : QOpenGLFunctions(ctx)
      , m_context(ctx)
  {}

  void render(Object3D& model)
  {
    glEnable(GL_CULL_FACE);

    QMatrix4x4 model_matrix;
    {
      // TMD y-down/z-depth, so we need to apply a transform to be z-up.
      model_matrix(1, 1) = 0;
      model_matrix(2, 2) = 0;
      model_matrix(0, 0) = 1;
      model_matrix(1, 2) = 1;
      model_matrix(2, 1) = -1;
    }

    recursiveRender(model, model_matrix);

    m_textures.deleteUnreachableTextures();
  }

private:
  bool setupVAOAndVBO();
  void recursiveRender(Object3D& object, QMatrix4x4 modelTransform);
  void render(PSX_Object3D& object, const QMatrix4x4& modelTransform);
};
