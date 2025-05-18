// Copyright (C) 2025 Vincent Chambrin
// This file is part of the 'mmd-viewer' project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "rendering/psxobject3d.h"

#include "formats/tim.h"
#include "formats/tmd.h"

#include "converters/tim2image.h"

#include <QVector3D>

#include <map>

inline QVector3D convert(tmd_vertex_t vertex)
{
  return QVector3D(vertex.x, vertex.y, vertex.z);
}

inline QVector3D convert(tmd_normal_t n)
{
  constexpr int decimalBits = 12;
  const float scale = 1.0f / (1 << decimalBits);
  const auto ret = -1 * QVector3D(n.x * scale, n.y * scale, n.z * scale);
  return ret;
}

inline RgbColor convert(tmd_color_t c)
{
  return RgbColor(c.r, c.g, c.b);
}

class PSX_TextureCache
{
private:
  std::vector<TimImage> m_tims;

  struct SearchKey
  {
    int texturePage = -1;
    int bpp = -1;
    int clutY = -1;
    auto operator<=>(const SearchKey&) const = default;
  };

  std::map<SearchKey, std::shared_ptr<PSX_Texture>> m_textures;

public:
  const std::vector<TimImage>& tims() const { return m_tims; }

  void setTIMs(std::vector<TimImage> images)
  {
    m_tims = std::move(images);
    m_textures.clear();
  }

  std::shared_ptr<PSX_Texture> getTexture(int page, int bpp, int clutX, int clutY)
  {
    SearchKey key;
    key.texturePage = page;
    key.bpp = bpp;
    key.clutY = clutY;

    auto it = m_textures.find(key);
    if (it != m_textures.end())
    {
      return it->second;
    }

    QImage image = createTextureImage(page, clutX, clutY);

    if (image.isNull())
    {
      return nullptr;
    }

    // image.save(QString("texture-%1-%2-%3.png")
    //                .arg(QString::number(page), QString::number(clutX), QString::number(clutY)));

    auto texture = std::make_shared<PSX_Texture>();
    texture->image = image;
    m_textures[key] = texture;
    return texture;
  }

private:
  static int getTexturePageFromVRAMCoords(const TimImage& img)
  {
    constexpr int VRAM_TEXTURE_PAGE_NATIVE_WIDTH = 64;
    constexpr int VRAM_TEXTURE_PAGE_NATIVE_HEIGHT = 256;
    const int page_x = img.getPixelX() / VRAM_TEXTURE_PAGE_NATIVE_WIDTH;
    const int page_y = img.getPixelY() / VRAM_TEXTURE_PAGE_NATIVE_HEIGHT;
    return page_x + page_y * 16;
  }

  QImage createTextureImage(int texturePage, int clutX, int clutY)
  {
    auto it = std::find_if(m_tims.rbegin(), m_tims.rend(), [texturePage](const TimImage& img) {
      return getTexturePageFromVRAMCoords(img) == texturePage;
    });

    if (it == m_tims.rend())
    {
      return QImage();
    }

    const TimImage& tim = *it;
    TimImage::Image timimg = tim.generateImage(clutX, clutY);
    return tim2image(timimg);
  }
};

class PSX_MaterialTracker
{
private:
  std::vector<std::shared_ptr<PSX_Material>>& m_materials;

  struct SearchKey
  {
    int texturePage = -1;
    int bpp = -1;
    int clutY = -1;
    bool vertexColors = false;
    bool light = false;

    auto operator<=>(const SearchKey&) const = default;
  };

  PSX_TextureCache& m_textures;
  std::map<SearchKey, int> m_materialsmap;

public:
  PSX_MaterialTracker(PSX_TextureCache& textures, std::vector<std::shared_ptr<PSX_Material>>& target)
      : m_materials(target)
      , m_textures(textures)
  {}

  int getMaterialIndex(const TMD_Primitive& primitive)
  {
    SearchKey key;

    if (primitive.hasTexture())
    {
      key.bpp = get_textureinfo_bpp(primitive.textureInfo());
      key.texturePage = primitive.textureInfo().page;
      key.clutY = primitive.clutInfo().clutY;
      key.light = primitive.normalCount() > 0;

      auto it = m_materialsmap.find(key);

      if (it != m_materialsmap.end())
      {
        return it->second;
      }

      std::shared_ptr<PSX_Texture> texture = m_textures.getTexture(key.texturePage,
                                                                   key.bpp,
                                                                   primitive.clutInfo().clutX * 16,
                                                                   key.clutY);

      if (texture)
      {
        auto material = std::make_shared<PSX_Material>();
        material->lighting = key.light;
        material->map = texture;

        const int material_index = m_materials.size();
        m_materials.push_back(material);
        m_materialsmap[key] = material_index;
        return material_index;
      }
    }

    key.light = primitive.normalCount() > 0;
    key.vertexColors = primitive.colorCount() == primitive.vertexCount();

    auto it = m_materialsmap.find(key);

    if (it != m_materialsmap.end())
    {
      return it->second;
    }

    auto material = std::make_shared<PSX_Material>();
    material->lighting = key.light;
    material->vertexColors = key.vertexColors;

    if (primitive.colorCount() == 1)
    {
      material->color = ::convert(*primitive.colors());
    }
    else
    {
      material->color = RgbColor(127, 0, 127);
    }

    const int material_index = m_materials.size();
    m_materials.push_back(material);
    m_materialsmap[key] = material_index;
    return material_index;
  }
};

class TMD_TexCoordsConverter
{
public:
  float m_width;
  float m_height;

public:
  explicit TMD_TexCoordsConverter(const QImage& tex)
      : m_width(tex.width())
      , m_height(tex.height())
  {}

  explicit TMD_TexCoordsConverter(PSX_Texture* texture)
  {
    if (texture)
    {
      m_width = texture->image.width();
      m_height = texture->image.height();
    }
    else
    {
      constexpr int VRAM_ACCESSIBLE_TEXTURE_WIDTH =
          256; // (texture page width) * 16bits / 4-bit per pixel
      constexpr int VRAM_ACCESSIBLE_TEXTURE_HEIGHT = 256;

      m_width = VRAM_ACCESSIBLE_TEXTURE_WIDTH;
      m_height = VRAM_ACCESSIBLE_TEXTURE_HEIGHT;
    }
  }

  TMD_TexCoordsConverter(float textureWidth, float textureHeight)
      : m_width(textureWidth)
      , m_height(textureHeight)
  {}

  QVector2D convert(tmd_uv_coord_t uv) const
  {
    return QVector2D(uv.u / m_width + 0.0001f, (m_height - uv.v) / m_height + 0.0001f);
  }
};

class TMD_ModelConverter
{
private:
  PSX_TextureCache m_textures;

public:
  PSX_TextureCache& textures() { return m_textures; }
  void setTIMs(std::vector<TimImage> images) { textures().setTIMs(std::move(images)); }

  std::unique_ptr<Group> convertModel(const TMD_Model& model)
  {
    auto group = std::make_unique<Group>();

    for (const TMD_Object& obj : model.objects())
    {
      std::unique_ptr<Object3D> obj3d = convertObject(obj);
      if (obj3d)
      {
        group->add(std::move(obj3d));
      }
    }

    return group;
  }

  std::unique_ptr<Object3D> convertObject(const TMD_Object& object)
  {
    const TMD_PrimitiveList& primitives = object.primitives();

    if (primitives.count() == 0)
    {
      return {};
    }

    auto result = std::make_unique<PSX_Object3D>();
    result->primitives.reserve(primitives.count());

    PSX_MaterialTracker tracker{m_textures, result->materials};

    for (int i(0); i < primitives.count(); ++i)
    {
      TMD_Primitive primitive{primitives.at(i)};

      if (!addPrimitive(*result, tracker, object, primitive))
      {
        continue;
      }
    }

    return result;
  }

private:
  bool addPrimitive(PSX_Object3D& data,
                    PSX_MaterialTracker& materialTracker,
                    const TMD_Object& tmdObj,
                    const TMD_Primitive& primitive)
  {
    PSX_Object3D::PrimitiveInfo element;

    if (primitive.getCode() == TMD_Code_POLYGON)
    {
      element.type = primitive.vertexCount() == 4 ? PSX_Object3D::Quad : PSX_Object3D::Triangle;
      element.index = data.vertices.size();
      element.count = element.type == PSX_Object3D::Quad ? 6 : 3;

      element.materialIndex = materialTracker.getMaterialIndex(primitive);

      assert(primitive.colorCount() > 0 || primitive.hasTexture());

      TMD_TexCoordsConverter uvconv{data.materials[element.materialIndex]->map.get()};

      append_triangles(data, tmdObj, primitive, uvconv);

      data.primitives.push_back(element);

      return true;
    }
    else if (primitive.getCode() == TMD_Code_LINE)
    {
      element.type = PSX_Object3D::Line;
      element.index = data.vertices.size();
      element.count = 2;

      element.materialIndex = materialTracker.getMaterialIndex(primitive);

      assert(primitive.colorCount() > 0 || primitive.hasTexture());

      TMD_TexCoordsConverter uvconv{data.materials[element.materialIndex]->map.get()};

      append_line(data, tmdObj, primitive, uvconv);

      data.primitives.push_back(element);

      return true;
    }
    else
    {
      return false;
    }

    return false;
  }

  void append_triangles(PSX_Object3D& data,
                        const TMD_Object& tmdObj,
                        const TMD_Primitive& primitive,
                        TMD_TexCoordsConverter& uvconv)
  {
    const QVector3D default_normal = QVector3D(-1, -1, -1).normalized();
    const QVector2D default_uv = QVector2D(0, 0);
    const RgbColor default_color = RgbColor(127, 127, 127);

    const size_t offset = data.vertices.size();

    constexpr int i00 = 2;
    constexpr int i01 = 1;
    constexpr int i02 = 0;
    constexpr int i10 = 1;
    constexpr int i11 = 2;
    constexpr int i12 = 3;

    if (offset > 0)
    {
      // fill attributes with dummy elements if neeeded
      {
        if (primitive.normalCount() > 0 && data.normals.empty())
        {
          data.normals.resize(offset, default_normal);
        }

        if (primitive.colorCount() > 0 && data.colors.empty())
        {
          data.colors.resize(offset, default_color);
        }

        if (primitive.hasTexture() && data.uv.empty())
        {
          data.uv.resize(offset, default_uv);
        }
      }

      const size_t newvertex_count = offset + (primitive.vertexCount() == 4 ? 6 : 3);
      {
        if (!data.normals.empty() && primitive.normalCount() == 0)
        {
          data.normals.resize(newvertex_count, default_normal);
        }

        if (primitive.colorCount() == 0 && !data.colors.empty())
        {
          data.colors.resize(newvertex_count, default_color);
        }

        if (!primitive.hasTexture() && !data.uv.empty())
        {
          data.uv.resize(newvertex_count, default_uv);
        }
      }
    }

    if (primitive.normalCount() > 0)
    {
      const tmd_normal_t* normals = tmdObj.normals().data();
      const uint16_t* indices = primitive.normals();
      const size_t last_index = primitive.normalCount() - 1;
      data.normals.push_back(convert(normals[indices[std::min<size_t>(i00, last_index)]]));
      data.normals.push_back(convert(normals[indices[std::min<size_t>(i01, last_index)]]));
      data.normals.push_back(convert(normals[indices[std::min<size_t>(i02, last_index)]]));
    }

    // vertices
    {
      const tmd_vertex_t* vertices = tmdObj.vertices().data();
      const uint16_t* indices = primitive.vertexBuf();
      data.vertices.push_back(convert(vertices[indices[i00]]));
      data.vertices.push_back(convert(vertices[indices[i01]]));
      data.vertices.push_back(convert(vertices[indices[i02]]));
    }

    if (primitive.colorCount() > 0)
    {
      const tmd_color_t* colors = primitive.colors();
      const size_t last_index = primitive.colorCount() - 1;
      data.colors.push_back(convert(colors[std::min<size_t>(i00, last_index)]));
      data.colors.push_back(convert(colors[std::min<size_t>(i01, last_index)]));
      data.colors.push_back(convert(colors[std::min<size_t>(i02, last_index)]));
    }

    if (primitive.hasTexture())
    {
      const tmd_uv_coord_t* uvs = primitive.uvs();

      data.uv.push_back(uvconv.convert(uvs[i00]));
      data.uv.push_back(uvconv.convert(uvs[i01]));
      data.uv.push_back(uvconv.convert(uvs[i02]));
    }

    if (primitive.vertexCount() == 4)
    {
      if (primitive.normalCount() > 0)
      {
        const tmd_normal_t* normals = tmdObj.normals().data();
        const uint16_t* indices = primitive.normals();
        const size_t last_index = primitive.normalCount() - 1;
        data.normals.push_back(convert(normals[indices[std::min<size_t>(i10, last_index)]]));
        data.normals.push_back(convert(normals[indices[std::min<size_t>(i11, last_index)]]));
        data.normals.push_back(convert(normals[indices[std::min<size_t>(i12, last_index)]]));
      }

      // vertices
      {
        const tmd_vertex_t* vertices = tmdObj.vertices().data();
        const uint16_t* indices = primitive.vertexBuf();
        data.vertices.push_back(convert(vertices[indices[i10]]));
        data.vertices.push_back(convert(vertices[indices[i11]]));
        data.vertices.push_back(convert(vertices[indices[i12]]));
      }

      if (primitive.colorCount() > 0)
      {
        const tmd_color_t* colors = primitive.colors();
        const size_t last_index = primitive.colorCount() - 1;
        data.colors.push_back(convert(colors[std::min<size_t>(i10, last_index)]));
        data.colors.push_back(convert(colors[std::min<size_t>(i11, last_index)]));
        data.colors.push_back(convert(colors[std::min<size_t>(i12, last_index)]));
      }

      if (primitive.hasTexture())
      {
        const tmd_uv_coord_t* uvs = primitive.uvs();

        data.uv.push_back(uvconv.convert(uvs[i10]));
        data.uv.push_back(uvconv.convert(uvs[i11]));
        data.uv.push_back(uvconv.convert(uvs[i12]));
      }
    }
  }

  void append_line(PSX_Object3D& data,
                   const TMD_Object& tmdObj,
                   const TMD_Primitive& primitive,
                   TMD_TexCoordsConverter& uvconv)
  {
    const QVector3D default_normal = QVector3D(-1, -1, -1).normalized();
    const QVector2D default_uv = QVector2D(0, 0);
    const RgbColor default_color = RgbColor(127, 127, 127);

    const size_t offset = data.vertices.size();

    if (offset > 0)
    {
      // fill attributes with dummy elements if neeeded
      {
        if (primitive.normalCount() > 0 && data.normals.empty())
        {
          data.normals.resize(offset, default_normal);
        }

        if (primitive.colorCount() > 0 && data.colors.empty())
        {
          data.colors.resize(offset, default_color);
        }

        if (primitive.hasTexture() && data.uv.empty())
        {
          data.uv.resize(offset, default_uv);
        }
      }

      const size_t newvertex_count = offset + 2;
      {
        if (!data.normals.empty() && primitive.normalCount() == 0)
        {
          data.normals.resize(newvertex_count, default_normal);
        }

        if (primitive.colorCount() == 0 && !data.colors.empty())
        {
          data.colors.resize(newvertex_count, default_color);
        }

        if (!primitive.hasTexture() && !data.uv.empty())
        {
          data.uv.resize(newvertex_count, default_uv);
        }
      }
    }

    assert(primitive.normalCount() == 0);
    assert(!primitive.hasTexture());

    // vertices
    {
      const tmd_vertex_t* vertices = tmdObj.vertices().data();
      const uint16_t* indices = primitive.vertexBuf();
      data.vertices.push_back(convert(vertices[indices[0]]));
      data.vertices.push_back(convert(vertices[indices[1]]));
    }

    assert(primitive.colorCount() > 0);
    {
      const tmd_color_t* colors = primitive.colors();
      const size_t last_index = primitive.colorCount() - 1;
      data.colors.push_back(convert(colors[std::min<size_t>(0, last_index)]));
      data.colors.push_back(convert(colors[std::min<size_t>(1, last_index)]));
    }
  }
};
