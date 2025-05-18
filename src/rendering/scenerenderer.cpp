// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#include "scenerenderer.h"

#include <algorithm>

static std::unique_ptr<QOpenGLTexture> createTextureFromImage(const QImage& image)
{
  auto texture = std::make_unique<QOpenGLTexture>(image.mirrored());
  texture->setWrapMode(QOpenGLTexture::ClampToEdge);
  return texture;
}

QOpenGLTexture* OpenGLTextureManager::getTextureFor(PSX_Texture& psxTexture)
{
  auto it = m_textures.find(&psxTexture);
  if (it != m_textures.end())
  {
    Value& value = it->second;
    if (value.revision != psxTexture.revision)
    {
      value.texture = createTextureFromImage(psxTexture.image);
      value.revision = psxTexture.revision;
    }
    return value.texture.get();
  }

  PSX_Texture_Entry e;
  e.key = &psxTexture;
  e.weakptr = psxTexture.shared_from_this();
  m_entries.push_back(e);

  Value& value = m_textures[&psxTexture];
  value.texture = createTextureFromImage(psxTexture.image);
  value.revision = psxTexture.revision;
  return value.texture.get();
}

void OpenGLTextureManager::deleteUnreachableTextures()
{
  auto it = std::partition(m_entries.begin(), m_entries.end(), [](const PSX_Texture_Entry& e) {
    return !e.weakptr.expired();
  });

  std::for_each(it, m_entries.end(), [this](PSX_Texture_Entry& e) { m_textures.erase(e.key); });

  m_entries.erase(it, m_entries.end());
}

bool SceneRenderer::setupVAOAndVBO()
{
  if (!m_vao.create())
  {
    return false;
  }

  m_vao.bind();

  // fill buffers and init vao
  {
    {
      BufferSpecs specs = BufferSpecsBuilder().index(0).tuplesize(3).type(GL_FLOAT);
      setup_buffer(m_buffers.vertex, this, BufferData(), specs);
    }

    {
      BufferSpecs specs = BufferSpecsBuilder().index(2).tuplesize(3).type(GL_UNSIGNED_BYTE);
      setup_buffer(m_buffers.color, this, BufferData(), specs);
    }

    {
      BufferSpecs specs = BufferSpecsBuilder().index(3).tuplesize(2).type(GL_FLOAT);
      setup_buffer(m_buffers.uv, this, BufferData(), specs);
    }

    {
      BufferSpecs specs = BufferSpecsBuilder().index(4).tuplesize(3).type(GL_FLOAT);
      setup_buffer(m_buffers.normal, this, BufferData(), specs);
    }
  }

  return true;
}

void SceneRenderer::recursiveRender(Object3D& object, QMatrix4x4 modelTransform)
{
  modelTransform *= object.matrix();

  if (auto* psxobj = dynamic_cast<PSX_Object3D*>(&object))
  {
    render(*psxobj, modelTransform);
  }

  for (Object3D* child : object.children())
  {
    recursiveRender(*child, modelTransform);
  }
}

void SceneRenderer::render(PSX_Object3D& object, const QMatrix4x4& modelTransform)
{
  if (object.vertices.empty())
  {
    return;
  }

  if (!m_vao.isCreated())
  {
    if (!setupVAOAndVBO())
    {
      qDebug() << "could no create vao";
      return;
    }
  }
  else
  {
    m_vao.bind();
  }

  //update buffers
  {
    {
      update_buffer(m_buffers.vertex, buffer_data_from_vector(object.vertices));
    }

    if (!object.colors.empty())
    {
      update_buffer(m_buffers.color, buffer_data_from_vector(object.colors));
    }

    if (!object.uv.empty())
    {
      update_buffer(m_buffers.uv, buffer_data_from_vector(object.uv));
    }

    if (!object.normals.empty())
    {
      update_buffer(m_buffers.normal, buffer_data_from_vector(object.normals));
    }
  }

  QOpenGLShaderProgram* active_program = nullptr;

  for (const PSX_Object3D::PrimitiveInfo& primitive : object.primitives)
  {
    if (primitive.type == PSX_Object3D::Sprite)
    {
      // TODO: handle sprites
      continue;
    }

    const PSX_Material& material = *object.materials[primitive.materialIndex];
    QOpenGLShaderProgram* shader_program = m_shaders.getProgram(object, material);

    if (!shader_program)
    {
      continue;
    }

    if (shader_program != active_program)
    {
      active_program = shader_program;
      active_program->bind();
    }

    shader_program->setUniformValue("model_matrix", modelTransform);
    shader_program->setUniformValue("view_matrix", viewMatrix);
    shader_program->setUniformValue("projection_matrix", projectionMatrix);

    shader_program->setUniformValue("material_color", QColor(material.color));

    if (material.map)
    {
      QOpenGLTexture* texture = m_textures.getTextureFor(*material.map);
      texture->bind();
      shader_program->setUniformValue("texture_diffuse", 0);
    }

    if (material.lighting)
    {
      shader_program->setUniformValue("light.direction", QVector3D(-1, 1, -1));
      shader_program->setUniformValue("light.ambient", QVector3D(0.7, 0.7, 0.7));
      shader_program->setUniformValue("light.diffuse", QVector3D(0.3, 0.3, 0.3));
    }

    GLenum mode = GL_TRIANGLES;
    if (primitive.type == PSX_Object3D::Line)
    {
      mode = GL_LINES;
    }

    glDrawArrays(mode, primitive.index, primitive.count);
  }

  if (active_program)
  {
    active_program->release();
  }

  m_vao.release();
}
