#pragma once

#include "game.h"
#include "io/sdlreader.h"
#include "util/helpers.h"

#include <array>
#include <stdexcept>
#include <vector>

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>

namespace loader
{
namespace
{
constexpr const uint16_t TextureIndexMaskTr4 = 0x7FFF;          // in some custom levels we need to use 0x7FFF flag
constexpr const uint16_t TextureIndexMask = 0x0FFF;
//constexpr const uint16_t TR_TEXTURE_SHAPE_MASK = 0x7000;          // still not used
constexpr const uint16_t TextureFlippedMask = 0x8000;
}

struct ByteColor
{
    uint8_t r, g, b, a;

    static ByteColor readTr1(io::SDLReader& reader)
    {
        return read(reader, false);
    }

    static ByteColor readTr2(io::SDLReader& reader)
    {
        return read(reader, true);
    }

private:
    static ByteColor read(io::SDLReader& reader, bool withAlpha)
    {
        ByteColor colour;
        colour.r = reader.readU8() << 2;
        colour.g = reader.readU8() << 2;
        colour.b = reader.readU8() << 2;
        if(withAlpha)
            colour.a = reader.readU8() << 2;
        else
            colour.a = 0;
        return colour;
    }
};

struct FloatColor
{
    float r, g, b, a;
};

struct Vertex
{
    float x, y, z;

    static Vertex read16(io::SDLReader& reader)
    {
        Vertex vertex;
        // read vertex and change coordinate system
        vertex.x = static_cast<float>(reader.readI16());
        vertex.y = static_cast<float>(-reader.readI16());
        vertex.z = static_cast<float>(-reader.readI16());
        return vertex;
    }

    static Vertex read32(io::SDLReader& reader)
    {
        Vertex vertex;
        // read vertex and change coordinate system
        vertex.x = static_cast<float>(reader.readI32());
        vertex.y = static_cast<float>(-reader.readI32());
        vertex.z = static_cast<float>(-reader.readI32());
        return vertex;
    }

    static Vertex readF(io::SDLReader& reader)
    {
        Vertex vertex;
        vertex.x = reader.readF();
        vertex.y = -reader.readF();
        vertex.z = -reader.readF();
        return vertex;
    }
};

struct Triangle
{
    uint16_t vertices[3];    ///< index into the appropriate list of vertices.
    uint16_t texture;        /**< \brief object-texture index or colour index.
                               * If the triangle is textured, then this is an index into the object-texture list.
                               * If it's not textured, then the low 8 bit contain the index into the 256 colour palette
                               * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                               */
    uint16_t lighting;       /**< \brief transparency flag & strength of the hilight (TR4-TR5).
                               * bit0 if set, then alpha channel = intensity (see attribute in tr2_object_texture).<br>
                               * bit1-7 is the strength of the hilight.
                               */

    static Triangle readTr1(io::SDLReader& reader)
    {
        return read(reader, false);
    }

    static Triangle readTr4(io::SDLReader& reader)
    {
        return read(reader, true);
    }

private:
    static Triangle read(io::SDLReader& reader, bool withLighting)
    {
        Triangle meshface;
        meshface.vertices[0] = reader.readU16();
        meshface.vertices[1] = reader.readU16();
        meshface.vertices[2] = reader.readU16();
        meshface.texture = reader.readU16();
        if(withLighting)
            meshface.lighting = reader.readU16();
        else
            meshface.lighting = 0;
        return meshface;
    }
};

struct QuadFace
{
    uint16_t vertices[4];    ///< index into the appropriate list of vertices.
    uint16_t texture;        /**< \brief object-texture index or colour index.
                               * If the rectangle is textured, then this is an index into the object-texture list.
                               * If it's not textured, then the low 8 bit contain the index into the 256 colour palette
                               * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                               */
    uint16_t lighting;       /**< \brief transparency flag & strength of the hilight (TR4-TR5).
                               *
                               * In TR4, objects can exhibit some kind of light reflection when seen from some particular angles.
                               * - bit0 if set, then alpha channel = intensity (see attribute in tr2_object_texture).
                               * - bit1-7 is the strength of the hilight.
                               */

    static QuadFace readTr1(io::SDLReader& reader)
    {
        return read(reader, false);
    }

    static QuadFace readTr4(io::SDLReader& reader)
    {
        return read(reader, true);
    }

private:
    static QuadFace read(io::SDLReader& reader, bool withLighting)
    {
        QuadFace meshface;
        meshface.vertices[0] = reader.readU16();
        meshface.vertices[1] = reader.readU16();
        meshface.vertices[2] = reader.readU16();
        meshface.vertices[3] = reader.readU16();
        meshface.texture = reader.readU16();
        if(withLighting)
            meshface.lighting = reader.readU16();
        else
            meshface.lighting = 0;
        return meshface;
    }
};

struct ByteTexture
{
    uint8_t pixels[256][256];

    static std::unique_ptr<ByteTexture> read(io::SDLReader& reader)
    {
        std::unique_ptr<ByteTexture> textile{ new ByteTexture() };
        reader.readBytes(reinterpret_cast<uint8_t*>(textile->pixels), 256 * 256);
        return textile;
    }
};

/** \brief 16-bit texture.
  *
  * Each pixel is a colour with the following format.<br>
  * - 1-bit transparency (0 ::= transparent, 1 ::= opaque) (0x8000)
  * - 5-bit red channel (0x7c00)
  * - 5-bit green channel (0x03e0)
  * - 5-bit blue channel (0x001f)
  */
struct WordTexture
{
    uint16_t pixels[256][256];

    static std::unique_ptr<WordTexture> read(io::SDLReader& reader)
    {
        std::unique_ptr<WordTexture> texture{ new WordTexture() };

        for(int i = 0; i < 256; i++)
        {
            for(int j = 0; j < 256; j++)
                texture->pixels[i][j] = reader.readU16();
        }

        return texture;
    }
};

struct DWordTexture
{
    uint32_t pixels[256][256];

    static std::unique_ptr<DWordTexture> read(io::SDLReader& reader)
    {
        std::unique_ptr<DWordTexture> textile{ new DWordTexture() };

        for(int i = 0; i < 256; i++)
        {
            for(int j = 0; j < 256; j++)
            {
                auto tmp = reader.readU32();
                const auto a = tmp & 0xff00ff00;
                const auto b = tmp & 0x00ff0000;
                const auto c = tmp & 0x000000ff;
                tmp = a | (b >> 16) | (c << 16);
                textile->pixels[i][j] = tmp;
            }
        }

        return textile;
    }
};

struct Portal
{
    uint16_t adjoining_room;     ///< \brief which room this portal leads to.
    Vertex normal;         /**< \brief which way the portal faces.
                                   * the normal points away from the adjacent room->
                                   * to be seen through, it must point toward the viewpoint.
                                   */
    Vertex vertices[4];    /**< \brief the corners of this portal.
                                   * the right-hand rule applies with respect to the normal.
                                   * if the right-hand-rule is not followed, the portal will
                                   * contain visual artifacts instead of a viewport to
                                   * Adjoiningroom->
                                   */

    static Portal read(io::SDLReader& reader)
    {
        Portal portal;
        portal.adjoining_room = reader.readU16();
        portal.normal = Vertex::read16(reader);
        portal.vertices[0] = Vertex::read16(reader);
        portal.vertices[1] = Vertex::read16(reader);
        portal.vertices[2] = Vertex::read16(reader);
        portal.vertices[3] = Vertex::read16(reader);
        if(util::fuzzyOne(portal.normal.x) && util::fuzzyZero(portal.normal.y) && util::fuzzyZero(portal.normal.z))
            return portal;
        if(util::fuzzyOne(-portal.normal.x) && util::fuzzyZero(portal.normal.y) && util::fuzzyZero(portal.normal.z))
            return portal;
        if(util::fuzzyZero(portal.normal.x) && util::fuzzyOne(portal.normal.y) && util::fuzzyZero(portal.normal.z))
            return portal;
        if(util::fuzzyZero(portal.normal.x) && util::fuzzyOne(-portal.normal.y) && util::fuzzyZero(portal.normal.z))
            return portal;
        if(util::fuzzyZero(portal.normal.x) && util::fuzzyZero(portal.normal.y) && util::fuzzyOne(portal.normal.z))
            return portal;
        if(util::fuzzyZero(portal.normal.x) && util::fuzzyZero(portal.normal.y) && util::fuzzyOne(-portal.normal.z))
            return portal;
        // std::cerr << "read_tr_room_portal: normal not on world axis");
        return portal;
    }
};

struct Sector
{
    uint16_t fd_index;     // Index into FloorData[]
    uint16_t box_index;    // Index into Boxes[]/Zones[] (-1 if none)
    uint8_t room_below;    // The number of the room below this one (-1 or 255 if none)
    int8_t floor;          // Absolute height of floor (multiply by 256 for world coordinates)
    uint8_t room_above;    // The number of the room above this one (-1 or 255 if none)
    int8_t ceiling;        // Absolute height of ceiling (multiply by 256 for world coordinates)

    static Sector read(io::SDLReader& reader)
    {
        Sector sector;
        sector.fd_index = reader.readU16();
        sector.box_index = reader.readU16();
        sector.room_below = reader.readU8();
        sector.floor = reader.readI8();
        sector.room_above = reader.readU8();
        sector.ceiling = reader.readI8();
        return sector;
    }
};

/*
* lights
*/
enum class LightType : uint8_t
{
    Null,
    Point,
    Spotlight,
    Sun,
    Shadow
};

struct Light
{
    Vertex position;           // world coords
    ByteColor color;         // three bytes rgb values
    float intensity;            // Calculated intensity
    uint16_t intensity1;        // Light intensity
    uint16_t intensity2;        // Almost always equal to Intensity1 [absent from TR1 data files]
    uint32_t fade1;             // Falloff value 1
    uint32_t fade2;             // Falloff value 2 [absent from TR1 data files]
    uint8_t light_type;         // same as D3D (i.e. 2 is for spotlight)
    uint8_t unknown;            // always 0xff?
    float r_inner;
    float r_outer;
    float length;
    float cutoff;
    Vertex dir;           // direction
    Vertex pos2;          // world coords
    Vertex dir2;          // direction

    LightType getLightType() const
    {
        switch(light_type)
        {
            case 0:
                return LightType::Sun;
            case 1:
                return LightType::Point;
            case 2:
                return LightType::Spotlight;
            case 3:
                return LightType::Shadow;
            default:
                return LightType::Null;
        }
    }

    /** \brief reads a room light definition.
      *
      * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * intensity2 and fade2 are introduced in TR2 and are set to intensity1 and fade1 for TR1.
      */
    static Light readTr1(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::read32(reader);
        // read and make consistent
        light.intensity1 = (8191 - reader.readU16()) << 2;
        light.fade1 = reader.readU32();
        // only in TR2
        light.intensity2 = light.intensity1;

        light.intensity = light.intensity1;
        light.intensity /= 4096.0f;

        if(light.intensity > 1.0f)
            light.intensity = 1.0f;

        light.fade2 = light.fade1;

        light.r_outer = static_cast<float>(light.fade1);
        light.r_inner = static_cast<float>(light.fade1 / 2);

        light.light_type = 1; // Point light

                                 // all white
        light.color.r = 0xff;
        light.color.g = 0xff;
        light.color.b = 0xff;
        return light;
    }

    static Light readTr2(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::read32(reader);
        light.intensity1 = reader.readU16();
        light.intensity2 = reader.readU16();
        light.fade1 = reader.readU32();
        light.fade2 = reader.readU32();

        light.intensity = light.intensity1;
        light.intensity /= 4096.0f;

        if(light.intensity > 1.0f)
            light.intensity = 1.0f;

        light.r_outer = static_cast<float>(light.fade1);
        light.r_inner = static_cast<float>(light.fade1 / 2);

        light.light_type = 1; // Point light

                                 // all white
        light.color.r = 0xff;
        light.color.g = 0xff;
        light.color.b = 0xff;
        return light;
    }

    static Light readTr3(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::read32(reader);
        light.color.r = reader.readU8();
        light.color.g = reader.readU8();
        light.color.b = reader.readU8();
        light.color.a = reader.readU8();
        light.fade1 = reader.readU32();
        light.fade2 = reader.readU32();

        light.intensity = 1.0f;

        light.r_outer = static_cast<float>(light.fade1);
        light.r_inner = static_cast<float>(light.fade1) / 2.0f;

        light.light_type = 1; // Point light
        return light;
    }

    static Light readTr4(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::read32(reader);
        light.color = ByteColor::readTr1(reader);
        light.light_type = reader.readU8();
        light.unknown = reader.readU8();
        light.intensity1 = reader.readU8();
        light.intensity = light.intensity1;
        light.intensity /= 32;
        light.r_inner = reader.readF();
        light.r_outer = reader.readF();
        light.length = reader.readF();
        light.cutoff = reader.readF();
        light.dir = Vertex::readF(reader);
        return light;
    }

    static Light readTr5(io::SDLReader& reader)
    {
        Light light;
        light.position = Vertex::readF(reader);
        //read_tr_colour(src, light.color);
        light.color.r = static_cast<uint8_t>(reader.readF() * 255);    // r
        light.color.g = static_cast<uint8_t>(reader.readF() * 255);    // g
        light.color.b = static_cast<uint8_t>(reader.readF() * 255);    // b
        light.color.a = static_cast<uint8_t>(reader.readF() * 255);    // a
        light.intensity = 1.0f;
        /*
        if ((temp != 0) && (temp != 0xCDCDCDCD))
        BOOST_THROW_EXCEPTION( TR_ReadError("read_tr5_room_light: seperator1 has wrong value") );
        */
        light.r_inner = reader.readF();
        light.r_outer = reader.readF();
        reader.readF();    // rad_input
        reader.readF();    // rad_output
        reader.readF();    // range
        light.dir = Vertex::readF(reader);
        light.pos2 = Vertex::read32(reader);
        light.dir2 = Vertex::read32(reader);
        light.light_type = reader.readU8();

        auto temp = reader.readU8();
        if(temp != 0xCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator2 has wrong value";

        temp = reader.readU8();
        if(temp != 0xCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator3 has wrong value";

        temp = reader.readU8();
        if(temp != 0xCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: seperator4 has wrong value";

        return light;
    }
};

struct Sprite
{
    int16_t vertex;                 // offset into vertex list
    int16_t texture;                // offset into sprite texture list

    /// \brief reads a room sprite definition.
    static Sprite read(io::SDLReader& reader)
    {
        Sprite room_sprite;
        room_sprite.vertex = reader.readI16();
        room_sprite.texture = reader.readI16();
        return room_sprite;
    }
};

/** \brief Room layer (TR5).
  */
struct Layer
{
    uint16_t num_vertices;
    uint16_t unknown_l1;
    uint16_t unknown_l2;
    uint16_t num_rectangles;
    uint16_t num_triangles;
    uint16_t unknown_l3;
    uint16_t unknown_l4;
    //  The following 6 floats define the bounding box for the layer
    float bounding_box_x1;
    float bounding_box_y1;
    float bounding_box_z1;
    float bounding_box_x2;
    float bounding_box_y2;
    float bounding_box_z2;
    int16_t unknown_l6a;
    int16_t unknown_l6b;
    int16_t unknown_l7a;
    int16_t unknown_l7b;
    int16_t unknown_l8a;
    int16_t unknown_l8b;

    static Layer read(io::SDLReader& reader)
    {
        Layer layer;
        layer.num_vertices = reader.readU16();
        layer.unknown_l1 = reader.readU16();
        layer.unknown_l2 = reader.readU16();
        layer.num_rectangles = reader.readU16();
        layer.num_triangles = reader.readU16();
        layer.unknown_l3 = reader.readU16();
        layer.unknown_l4 = reader.readU16();
        if(reader.readU16() != 0)
            BOOST_LOG_TRIVIAL(warning) << "Room Layer: filler2 has wrong value";

        layer.bounding_box_x1 = reader.readF();
        layer.bounding_box_y1 = -reader.readF();
        layer.bounding_box_z1 = -reader.readF();
        layer.bounding_box_x2 = reader.readF();
        layer.bounding_box_y2 = -reader.readF();
        layer.bounding_box_z2 = -reader.readF();
        if(reader.readU32() != 0)
            BOOST_LOG_TRIVIAL(warning) << "Room Layer: filler3 has wrong value";

        layer.unknown_l6a = reader.readI16();
        layer.unknown_l6b = reader.readI16();
        layer.unknown_l7a = reader.readI16();
        layer.unknown_l7b = reader.readI16();
        layer.unknown_l8a = reader.readI16();
        layer.unknown_l8b = reader.readI16();
        return layer;
    }
};

struct RoomVertex
{
    Vertex vertex;    // where this vertex lies (relative to tr2_room_info::x/z)
    int16_t lighting1;
    uint16_t attributes;    // A set of flags for special rendering effects [absent from TR1 data files]
    // 0x8000 something to do with water surface
    // 0x4000 under water lighting modulation and
    // movement if viewed from above water surface
    // 0x2000 water/quicksand surface movement
    // 0x0010 "normal"
    int16_t lighting2;      // Almost always equal to Lighting1 [absent from TR1 data files]
    // TR5 -->
    Vertex normal;
    FloatColor colour;

    /** \brief reads a room vertex definition.
      *
      * lighting1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * lighting2 is introduced in TR2 and is set to lighting1 for TR1.
      * attributes is introduced in TR2 and is set 0 for TR1.
      * All other values are introduced in TR5 and get set to appropiate values.
      */
    static RoomVertex readTr1(io::SDLReader& reader)
    {
        RoomVertex room_vertex;
        room_vertex.vertex = Vertex::read16(reader);
        // read and make consistent
        room_vertex.lighting1 = (8191 - reader.readI16()) << 2;
        // only in TR2
        room_vertex.lighting2 = room_vertex.lighting1;
        room_vertex.attributes = 0;
        // only in TR5
        room_vertex.normal.x = 0;
        room_vertex.normal.y = 0;
        room_vertex.normal.z = 0;
        room_vertex.colour.r = room_vertex.lighting1 / 32768.0f;
        room_vertex.colour.g = room_vertex.lighting1 / 32768.0f;
        room_vertex.colour.b = room_vertex.lighting1 / 32768.0f;
        room_vertex.colour.a = 1.0f;
        return room_vertex;
    }

    static RoomVertex readTr2(io::SDLReader& reader)
    {
        RoomVertex room_vertex;
        room_vertex.vertex = Vertex::read16(reader);
        // read and make consistent
        room_vertex.lighting1 = (8191 - reader.readI16()) << 2;
        room_vertex.attributes = reader.readU16();
        room_vertex.lighting2 = (8191 - reader.readI16()) << 2;
        // only in TR5
        room_vertex.normal.x = 0;
        room_vertex.normal.y = 0;
        room_vertex.normal.z = 0;
        room_vertex.colour.r = room_vertex.lighting2 / 32768.0f;
        room_vertex.colour.g = room_vertex.lighting2 / 32768.0f;
        room_vertex.colour.b = room_vertex.lighting2 / 32768.0f;
        room_vertex.colour.a = 1.0f;
        return room_vertex;
    }

    static RoomVertex readTr3(io::SDLReader& reader)
    {
        RoomVertex room_vertex;
        room_vertex.vertex = Vertex::read16(reader);
        // read and make consistent
        room_vertex.lighting1 = reader.readI16();
        room_vertex.attributes = reader.readU16();
        room_vertex.lighting2 = reader.readI16();
        // only in TR5
        room_vertex.normal.x = 0;
        room_vertex.normal.y = 0;
        room_vertex.normal.z = 0;

        room_vertex.colour.r = ((room_vertex.lighting2 & 0x7C00) >> 10) / 62.0f;
        room_vertex.colour.g = ((room_vertex.lighting2 & 0x03E0) >> 5) / 62.0f;
        room_vertex.colour.b = (room_vertex.lighting2 & 0x001F) / 62.0f;
        room_vertex.colour.a = 1.0f;
        return room_vertex;
    }

    static RoomVertex readTr4(io::SDLReader& reader)
    {
        RoomVertex room_vertex;
        room_vertex.vertex = Vertex::read16(reader);
        // read and make consistent
        room_vertex.lighting1 = reader.readI16();
        room_vertex.attributes = reader.readU16();
        room_vertex.lighting2 = reader.readI16();
        // only in TR5
        room_vertex.normal.x = 0;
        room_vertex.normal.y = 0;
        room_vertex.normal.z = 0;

        room_vertex.colour.r = ((room_vertex.lighting2 & 0x7C00) >> 10) / 31.0f;
        room_vertex.colour.g = ((room_vertex.lighting2 & 0x03E0) >> 5) / 31.0f;
        room_vertex.colour.b = (room_vertex.lighting2 & 0x001F) / 31.0f;
        room_vertex.colour.a = 1.0f;
        return room_vertex;
    }

    static RoomVertex readTr5(io::SDLReader& reader)
    {
        RoomVertex vert;
        vert.vertex = Vertex::readF(reader);
        vert.normal = Vertex::readF(reader);
        vert.colour.b = reader.readU8() / 255.0f;
        vert.colour.g = reader.readU8() / 255.0f;
        vert.colour.r = reader.readU8() / 255.0f;
        vert.colour.a = reader.readU8() / 255.0f;
        return vert;
    }
};

struct RoomStaticMesh
{
    Vertex position;       // world coords
    float rotation;         // high two bits (0xC000) indicate steps of
    // 90 degrees (e.g. (Rotation >> 14) * 90)
    int16_t intensity1;     // Constant lighting; -1 means use mesh lighting
    int16_t intensity2;     // Like Intensity 1, and almost always the same value [absent from TR1 data files]
    uint16_t object_id;     // which StaticMesh item to draw
    FloatColor tint;      // extracted from intensity

    /** \brief reads a room staticmesh definition.
      *
      * rotation gets converted to float and scaled appropiatly.
      * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * intensity2 is introduced in TR2 and is set to intensity1 for TR1.
      */
    static RoomStaticMesh readTr1(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = Vertex::read32(reader);
        room_static_mesh.rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        room_static_mesh.intensity1 = reader.readI16();
        room_static_mesh.object_id = reader.readU16();
        // make consistent
        if(room_static_mesh.intensity1 >= 0)
            room_static_mesh.intensity1 = (8191 - room_static_mesh.intensity1) << 2;
        // only in TR2
        room_static_mesh.intensity2 = room_static_mesh.intensity1;

        room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2 / 16384.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr2(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = Vertex::read32(reader);
        room_static_mesh.rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        room_static_mesh.intensity1 = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.object_id = reader.readU16();
        // make consistent
        if(room_static_mesh.intensity1 >= 0)
            room_static_mesh.intensity1 = (8191 - room_static_mesh.intensity1) << 2;
        if(room_static_mesh.intensity2 >= 0)
            room_static_mesh.intensity2 = (8191 - room_static_mesh.intensity2) << 2;

        room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2 / 16384.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr3(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = Vertex::read32(reader);
        room_static_mesh.rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        room_static_mesh.intensity1 = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.object_id = reader.readU16();

        room_static_mesh.tint.r = (room_static_mesh.intensity1 & 0x001F) / 62.0f;

        room_static_mesh.tint.g = ((room_static_mesh.intensity1 & 0x03E0) >> 5) / 62.0f;

        room_static_mesh.tint.b = ((room_static_mesh.intensity1 & 0x7C00) >> 10) / 62.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }

    static RoomStaticMesh readTr4(io::SDLReader& reader)
    {
        RoomStaticMesh room_static_mesh;
        room_static_mesh.position = Vertex::read32(reader);
        room_static_mesh.rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        room_static_mesh.intensity1 = reader.readI16();
        room_static_mesh.intensity2 = reader.readI16();
        room_static_mesh.object_id = reader.readU16();

        room_static_mesh.tint.r = (room_static_mesh.intensity1 & 0x001F) / 31.0f;

        room_static_mesh.tint.g = ((room_static_mesh.intensity1 & 0x03E0) >> 5) / 31.0f;

        room_static_mesh.tint.b = ((room_static_mesh.intensity1 & 0x7C00) >> 10) / 31.0f;
        room_static_mesh.tint.a = 1.0f;
        return room_static_mesh;
    }
};

// In TR3-5, there were 5 reverb / echo effect flags for each
// room, but they were never used in PC versions - however, level
// files still contain this info, so we now can re-use these flags
// to assign reverb/echo presets to each room->
// Also, underwater environment can be considered as additional
// reverb flag, so overall amount is 6.

enum class ReverbType : uint8_t
{
    Outside,         // EFX_REVERB_PRESET_CITY
    SmallRoom,       // EFX_REVERB_PRESET_LIVINGROOM
    MediumRoom,      // EFX_REVERB_PRESET_WOODEN_LONGPASSAGE
    LargeRoom,       // EFX_REVERB_PRESET_DOME_TOMB
    Pipe,            // EFX_REVERB_PRESET_PIPE_LARGE
    Water,           // EFX_REVERB_PRESET_UNDERWATER
    Sentinel
};

/** \brief room->
  */
struct Room
{
    Vertex offset;            ///< \brief offset of room (world coordinates).
    float y_bottom;                 ///< \brief indicates lowest point in room->
    float y_top;                    ///< \brief indicates highest point in room->
    std::vector<Layer> layers;       // [NumStaticMeshes]list of static meshes
    std::vector<RoomVertex> vertices;    // [NumVertices] list of vertices (relative coordinates)
    std::vector<QuadFace> rectangles;        // [NumRectangles] list of textured rectangles
    std::vector<Triangle> triangles;         // [NumTriangles] list of textured triangles
    std::vector<Sprite> sprites;      // [NumSprites] list of sprites
    std::vector<Portal> portals;      // [NumPortals] list of visibility portals
    uint16_t num_zsectors;          // "width" of sector list
    uint16_t num_xsectors;          // "height" of sector list
    std::vector<Sector> sector_list;  // [NumXsectors * NumZsectors] list of sectors
    // in this room
    int16_t intensity1;             // This and the next one only affect externally-lit objects
    int16_t intensity2;             // Almost always the same value as AmbientIntensity1 [absent from TR1 data files]
    int16_t light_mode;             // (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
    std::vector<Light> lights;       // [NumLights] list of point lights
    std::vector<RoomStaticMesh> static_meshes;    // [NumStaticMeshes]list of static meshes
    int16_t alternate_room;         // number of the room that this room can alternate
    int8_t  alternate_group;        // number of group which is used to switch alternate rooms
    // with (e.g. empty/filled with water is implemented as an empty room that alternates with a full room)

    uint16_t flags;
    // Flag bits:
    // 0x0001 - room is filled with water,
    // 0x0020 - Lara's ponytail gets blown by the wind;
    // TR1 has only the water flag and the extra unknown flag 0x0100.
    // TR3 most likely has flags for "is raining", "is snowing", "water is cold", and "is
    // filled by quicksand", among others.

    uint8_t water_scheme;
    // Water scheme is used with various room options, for example, R and M room flags in TRLE.
    // Also, it specifies lighting scheme, when 0x4000 vertex attribute is set.

    ReverbType reverb_info;

    // Reverb info is used in TR3-5 and contains index that specifies reverb type.
    // 0 - Outside, 1 - Small room, 2 - Medium room, 3 - Large room, 4 - Pipe.

    FloatColor light_colour;    // Present in TR5 only

    // TR5 only:

    float room_x;
    float room_z;
    float room_y_bottom;
    float room_y_top;

    uint32_t unknown_r1;
    uint32_t unknown_r2;
    uint32_t unknown_r3;
    uint16_t unknown_r4a;
    uint16_t unknown_r4b;
    uint32_t unknown_r5;
    uint32_t unknown_r6;

    /** \brief reads a room definition.
      *
      * intensity1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * intensity2 is introduced in TR2 and is set to intensity1 for TR1.
      * light_mode is only in TR2 and is set 0 for TR1.
      * light_colour is only in TR3-4 and gets set appropiatly.
      */
    static std::unique_ptr<Room> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Room> room{ new Room() };

        // read and change coordinate system
        room->offset.x = static_cast<float>(reader.readI32());
        room->offset.y = 0;
        room->offset.z = static_cast<float>(-reader.readI32());
        room->y_bottom = static_cast<float>(-reader.readI32());
        room->y_top = static_cast<float>(-reader.readI32());

        auto num_data_words = reader.readU32();

        auto position = reader.tell();

        room->vertices.resize(reader.readU16());
        for(size_t i = 0; i < room->vertices.size(); i++)
            room->vertices[i] = RoomVertex::readTr1(reader);

        room->rectangles.resize(reader.readU16());
        for(size_t i = 0; i < room->rectangles.size(); i++)
            room->rectangles[i] = QuadFace::readTr1(reader);

        room->triangles.resize(reader.readU16());
        for(size_t i = 0; i < room->triangles.size(); i++)
            room->triangles[i] = Triangle::readTr1(reader);

        room->sprites.resize(reader.readU16());
        for(size_t i = 0; i < room->sprites.size(); i++)
            room->sprites[i] = Sprite::read(reader);

        // set to the right position in case that there is some unused data
        reader.seek(position + num_data_words * 2);

        room->portals.resize(reader.readU16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader);

        room->num_zsectors = reader.readU16();
        room->num_xsectors = reader.readU16();
        room->sector_list.resize(room->num_zsectors * room->num_xsectors);
        for(uint32_t i = 0; i < static_cast<uint32_t>(room->num_zsectors * room->num_xsectors); i++)
            room->sector_list[i] = Sector::read(reader);

        // read and make consistent
        room->intensity1 = (8191 - reader.readI16()) << 2;
        // only in TR2-TR4
        room->intensity2 = room->intensity1;
        // only in TR2
        room->light_mode = 0;

        room->lights.resize(reader.readU16());
        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr1(reader);

        room->static_meshes.resize(reader.readU16());
        for(size_t i = 0; i < room->static_meshes.size(); i++)
            room->static_meshes[i] = RoomStaticMesh::readTr1(reader);

        room->alternate_room = reader.readI16();
        room->alternate_group = 0;   // Doesn't exist in TR1-3

        room->flags = reader.readU16();
        room->reverb_info = ReverbType::MediumRoom;

        room->light_colour.r = room->intensity1 / 32767.0f;
        room->light_colour.g = room->intensity1 / 32767.0f;
        room->light_colour.b = room->intensity1 / 32767.0f;
        room->light_colour.a = 1.0f;
        return room;
    }

    static std::unique_ptr<Room> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Room> room{ new Room() };
        // read and change coordinate system
        room->offset.x = static_cast<float>(reader.readI32());
        room->offset.y = 0;
        room->offset.z = static_cast<float>(-reader.readI32());
        room->y_bottom = static_cast<float>(-reader.readI32());
        room->y_top = static_cast<float>(-reader.readI32());

        auto num_data_words = reader.readU32();

        auto position = reader.tell();

        room->vertices.resize(reader.readU16());
        for(size_t i = 0; i < room->vertices.size(); i++)
            room->vertices[i] = RoomVertex::readTr2(reader);

        room->rectangles.resize(reader.readU16());
        for(size_t i = 0; i < room->rectangles.size(); i++)
            room->rectangles[i] = QuadFace::readTr1(reader);

        room->triangles.resize(reader.readU16());
        for(size_t i = 0; i < room->triangles.size(); i++)
            room->triangles[i] = Triangle::readTr1(reader);

        room->sprites.resize(reader.readU16());
        for(size_t i = 0; i < room->sprites.size(); i++)
            room->sprites[i] = Sprite::read(reader);

        // set to the right position in case that there is some unused data
        reader.seek(position + num_data_words * 2);

        room->portals.resize(reader.readU16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader);

        room->num_zsectors = reader.readU16();
        room->num_xsectors = reader.readU16();
        room->sector_list.resize(room->num_zsectors * room->num_xsectors);
        for(size_t i = 0; i < static_cast<uint32_t>(room->num_zsectors * room->num_xsectors); i++)
            room->sector_list[i] = Sector::read(reader);

        // read and make consistent
        room->intensity1 = (8191 - reader.readI16()) << 2;
        room->intensity2 = (8191 - reader.readI16()) << 2;
        room->light_mode = reader.readI16();

        room->lights.resize(reader.readU16());
        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr2(reader);

        room->static_meshes.resize(reader.readU16());
        for(size_t i = 0; i < room->static_meshes.size(); i++)
            room->static_meshes[i] = RoomStaticMesh::readTr2(reader);

        room->alternate_room = reader.readI16();
        room->alternate_group = 0;   // Doesn't exist in TR1-3

        room->flags = reader.readU16();

        if(room->flags & 0x0020)
        {
            room->reverb_info = ReverbType::Outside;
        }
        else
        {
            room->reverb_info = ReverbType::MediumRoom;
        }

        room->light_colour.r = room->intensity1 / 16384.0f;
        room->light_colour.g = room->intensity1 / 16384.0f;
        room->light_colour.b = room->intensity1 / 16384.0f;
        room->light_colour.a = 1.0f;
        return room;
    }

    static std::unique_ptr<Room> readTr3(io::SDLReader& reader)
    {
        std::unique_ptr<Room> room{ new Room() };

        // read and change coordinate system
        room->offset.x = static_cast<float>(reader.readI32());
        room->offset.y = 0;
        room->offset.z = static_cast<float>(-reader.readI32());
        room->y_bottom = static_cast<float>(-reader.readI32());
        room->y_top = static_cast<float>(-reader.readI32());

        auto num_data_words = reader.readU32();

        auto position = reader.tell();

        room->vertices.resize(reader.readU16());
        for(size_t i = 0; i < room->vertices.size(); i++)
            room->vertices[i] = RoomVertex::readTr3(reader);

        room->rectangles.resize(reader.readU16());
        for(size_t i = 0; i < room->rectangles.size(); i++)
            room->rectangles[i] = QuadFace::readTr1(reader);

        room->triangles.resize(reader.readU16());
        for(size_t i = 0; i < room->triangles.size(); i++)
            room->triangles[i] = Triangle::readTr1(reader);

        room->sprites.resize(reader.readU16());
        for(size_t i = 0; i < room->sprites.size(); i++)
            room->sprites[i] = Sprite::read(reader);

        // set to the right position in case that there is some unused data
        reader.seek(position + num_data_words * 2);

        room->portals.resize(reader.readU16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader);

        room->num_zsectors = reader.readU16();
        room->num_xsectors = reader.readU16();
        room->sector_list.resize(room->num_zsectors * room->num_xsectors);
        for(size_t i = 0; i < static_cast<uint32_t>(room->num_zsectors * room->num_xsectors); i++)
            room->sector_list[i] = Sector::read(reader);

        room->intensity1 = reader.readI16();
        room->intensity2 = reader.readI16();

        // only in TR2
        room->light_mode = 0;

        room->lights.resize(reader.readU16());
        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr3(reader);

        room->static_meshes.resize(reader.readU16());
        for(size_t i = 0; i < room->static_meshes.size(); i++)
            room->static_meshes[i] = RoomStaticMesh::readTr3(reader);

        room->alternate_room = reader.readI16();
        room->alternate_group = 0;   // Doesn't exist in TR1-3

        room->flags = reader.readU16();

        if(room->flags & 0x0080)
        {
            room->flags |= 0x0002;   // Move quicksand flag to another bit to avoid confusion with NL flag.
            room->flags ^= 0x0080;
        }

        // Only in TR3-5

        room->water_scheme = reader.readU8();
        room->reverb_info = static_cast<ReverbType>(reader.readU8());

        reader.skip(1);   // Alternate_group override?

        room->light_colour.r = room->intensity1 / 65534.0f;
        room->light_colour.g = room->intensity1 / 65534.0f;
        room->light_colour.b = room->intensity1 / 65534.0f;
        room->light_colour.a = 1.0f;
        return room;
    }

    static std::unique_ptr<Room> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<Room> room{ new Room() };
        // read and change coordinate system
        room->offset.x = static_cast<float>(reader.readI32());
        room->offset.y = 0;
        room->offset.z = static_cast<float>(-reader.readI32());
        room->y_bottom = static_cast<float>(-reader.readI32());
        room->y_top = static_cast<float>(-reader.readI32());

        auto num_data_words = reader.readU32();

        auto position = reader.tell();

        room->vertices.resize(reader.readU16());
        for(size_t i = 0; i < room->vertices.size(); i++)
            room->vertices[i] = RoomVertex::readTr4(reader);

        room->rectangles.resize(reader.readU16());
        for(size_t i = 0; i < room->rectangles.size(); i++)
            room->rectangles[i] = QuadFace::readTr1(reader);

        room->triangles.resize(reader.readU16());
        for(size_t i = 0; i < room->triangles.size(); i++)
            room->triangles[i] = Triangle::readTr1(reader);

        room->sprites.resize(reader.readU16());
        for(size_t i = 0; i < room->sprites.size(); i++)
            room->sprites[i] = Sprite::read(reader);

        // set to the right position in case that there is some unused data
        reader.seek(position + num_data_words * 2);

        room->portals.resize(reader.readU16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader);

        room->num_zsectors = reader.readU16();
        room->num_xsectors = reader.readU16();
        room->sector_list.resize(room->num_zsectors * room->num_xsectors);
        for(size_t i = 0; i < static_cast<uint32_t>(room->num_zsectors * room->num_xsectors); i++)
            room->sector_list[i] = Sector::read(reader);

        room->intensity1 = reader.readI16();
        room->intensity2 = reader.readI16();

        // only in TR2
        room->light_mode = 0;

        room->lights.resize(reader.readU16());
        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr4(reader);

        room->static_meshes.resize(reader.readU16());
        for(size_t i = 0; i < room->static_meshes.size(); i++)
            room->static_meshes[i] = RoomStaticMesh::readTr4(reader);

        room->alternate_room = reader.readI16();
        room->flags = reader.readU16();

        // Only in TR3-5

        room->water_scheme = reader.readU8();
        room->reverb_info = static_cast<ReverbType>(reader.readU8());

        // Only in TR4-5

        room->alternate_group = reader.readU8();

        room->light_colour.r = (room->intensity2 & 0x00FF) / 255.0f;
        room->light_colour.g = ((room->intensity1 & 0xFF00) >> 8) / 255.0f;
        room->light_colour.b = (room->intensity1 & 0x00FF) / 255.0f;
        room->light_colour.a = ((room->intensity2 & 0xFF00) >> 8) / 255.0f;
        return room;
    }

    static std::unique_ptr<Room> readTr5(io::SDLReader& reader)
    {
        if(reader.readU32() != 0x414C4558)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: 'XELA' not found";

        const auto room_data_size = reader.readU32();
        const auto position = reader.tell();
        const auto endPos = position + room_data_size;

        std::unique_ptr<Room> room{ new Room() };
        room->intensity1 = 32767;
        room->intensity2 = 32767;
        room->light_mode = 0;

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator1 has wrong value";

        /*portal_offset = */reader.readI32();             // StartPortalOffset?   // endSDOffset
        auto sector_data_offset = reader.readU32();    // StartSDOffset
        auto temp = reader.readU32();
        if(temp != 0 && temp != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator2 has wrong value";

        auto static_meshes_offset = reader.readU32();     // endPortalOffset
                                                        // static_meshes_offset or room_layer_offset
                                                        // read and change coordinate system
        room->offset.x = static_cast<float>(reader.readI32());
        room->offset.y = static_cast<float>(reader.readU32());
        room->offset.z = static_cast<float>(-reader.readI32());
        room->y_bottom = static_cast<float>(-reader.readI32());
        room->y_top = static_cast<float>(-reader.readI32());

        room->num_zsectors = reader.readU16();
        room->num_xsectors = reader.readU16();

        room->light_colour.b = reader.readU8() / 255.0f;
        room->light_colour.g = reader.readU8() / 255.0f;
        room->light_colour.r = reader.readU8() / 255.0f;
        room->light_colour.a = reader.readU8() / 255.0f;
        room->light_colour.a = 1.0f;

        room->lights.resize(reader.readU16());
        if(room->lights.size() > 512)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: lights.size() > 512";

        room->static_meshes.resize(reader.readU16());
        if(room->static_meshes.size() > 512)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: static_meshes.size() > 512";

        room->reverb_info = static_cast<ReverbType>(reader.readU8());
        room->alternate_group = reader.readU8();
        room->water_scheme = static_cast<uint8_t>(reader.readU16());

        if(reader.readU32() != 0x00007FFF)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: filler1 has wrong value";

        if(reader.readU32() != 0x00007FFF)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: filler2 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator4 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator5 has wrong value";

        if(reader.readU32() != 0xFFFFFFFF)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator6 has wrong value";

        room->alternate_room = reader.readI16();

        room->flags = reader.readU16();

        room->unknown_r1 = reader.readU32();
        room->unknown_r2 = reader.readU32();
        room->unknown_r3 = reader.readU32();

        temp = reader.readU32();
        if(temp != 0 && temp != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator7 has wrong value";

        room->unknown_r4a = reader.readU16();
        room->unknown_r4b = reader.readU16();

        room->room_x = reader.readF();
        room->unknown_r5 = reader.readU32();
        room->room_z = -reader.readF();

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator8 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator9 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator10 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator11 has wrong value";

        temp = reader.readU32();
        if(temp != 0 && temp != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator12 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator13 has wrong value";

        auto num_triangles = reader.readU32();
        if(num_triangles == 0xCDCDCDCD)
            num_triangles = 0;
        if(num_triangles > 512)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: triangles.size() > 512";
        room->triangles.resize(num_triangles);

        auto num_rectangles = reader.readU32();
        if(num_rectangles == 0xCDCDCDCD)
            num_rectangles = 0;
        if(num_rectangles > 1024)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: rectangles.size() > 1024";
        room->rectangles.resize(num_rectangles);

        if(reader.readU32() != 0)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator14 has wrong value";

        /*light_size = */reader.readU32();
        auto numL2 = reader.readU32();
        if(numL2 != room->lights.size())
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: numLights2 != lights.size()"));

        room->unknown_r6 = reader.readU32();
        room->room_y_top = -reader.readF();
        room->room_y_bottom = -reader.readF();

        room->layers.resize(reader.readU32());

        auto layer_offset = reader.readU32();
        auto vertices_offset = reader.readU32();
        auto poly_offset = reader.readU32();
        auto poly_offset2 = reader.readU32();
        if(poly_offset != poly_offset2)
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: poly_offset != poly_offset2"));

        auto vertices_size = reader.readU32();
        if(vertices_size % 28 != 0)
            BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: vertices_size has wrong value"));

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator15 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator16 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator17 has wrong value";

        if(reader.readU32() != 0xCDCDCDCD)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Room: seperator18 has wrong value";

        for(size_t i = 0; i < room->lights.size(); i++)
            room->lights[i] = Light::readTr5(reader);

        reader.seek(position + 208 + sector_data_offset);

        room->sector_list.resize(room->num_zsectors * room->num_xsectors);
        for(size_t i = 0; i < static_cast<uint32_t>(room->num_zsectors * room->num_xsectors); i++)
            room->sector_list[i] = Sector::read(reader);

        room->portals.resize(reader.readI16());
        for(size_t i = 0; i < room->portals.size(); i++)
            room->portals[i] = Portal::read(reader);

        reader.seek(position + 208 + static_meshes_offset);

        for(size_t i = 0; i < room->static_meshes.size(); i++)
            room->static_meshes[i] = RoomStaticMesh::readTr4(reader);

        reader.seek(position + 208 + layer_offset);

        for(size_t i = 0; i < room->layers.size(); i++)
            room->layers[i] = Layer::read(reader);

        reader.seek(position + 208 + poly_offset);

        {
            uint32_t vertex_index = 0;
            uint32_t rectangle_index = 0;
            uint32_t triangle_index = 0;

            for(size_t i = 0; i < room->layers.size(); i++)
            {
                uint32_t j;

                for(j = 0; j < room->layers[i].num_rectangles; j++)
                {
                    room->rectangles[rectangle_index] = QuadFace::readTr4(reader);
                    room->rectangles[rectangle_index].vertices[0] += vertex_index;
                    room->rectangles[rectangle_index].vertices[1] += vertex_index;
                    room->rectangles[rectangle_index].vertices[2] += vertex_index;
                    room->rectangles[rectangle_index].vertices[3] += vertex_index;
                    rectangle_index++;
                }
                for(j = 0; j < room->layers[i].num_triangles; j++)
                {
                    room->triangles[triangle_index] = Triangle::readTr4(reader);
                    room->triangles[triangle_index].vertices[0] += vertex_index;
                    room->triangles[triangle_index].vertices[1] += vertex_index;
                    room->triangles[triangle_index].vertices[2] += vertex_index;
                    triangle_index++;
                }
                vertex_index += room->layers[i].num_vertices;
            }
        }

        reader.seek(position + 208 + vertices_offset);

        {
            uint32_t vertex_index = 0;
            room->vertices.resize(vertices_size / 28);
            //int temp1 = room_data_size - (208 + vertices_offset + vertices_size);
            for(size_t i = 0; i < room->layers.size(); i++)
            {
                uint32_t j;

                for(j = 0; j < room->layers[i].num_vertices; j++)
                    room->vertices[vertex_index++] = RoomVertex::readTr5(reader);
            }
        }

        reader.seek(endPos);

        return room;
    }
};

struct Mesh
{
    Vertex centre;                // This is usually close to the mesh's centroid, and appears to be the center of a sphere used for collision testing.
    int32_t collision_size;             // This appears to be the radius of that aforementioned collisional sphere.
    std::vector<Vertex> vertices;             //[NumVertices]; // list of vertices (relative coordinates)
    std::vector<Vertex> normals;              //[NumNormals]; // list of normals (if NumNormals is positive)
    std::vector<int16_t> lights;                    //[-NumNormals]; // list of light values (if NumNormals is negative)
    std::vector<QuadFace> textured_rectangles;   //[NumTexturedRectangles]; // list of textured rectangles
    std::vector<Triangle> textured_triangles;    //[NumTexturedTriangles]; // list of textured triangles
    // the rest is not present in TR4
    std::vector<QuadFace> coloured_rectangles;   //[NumColouredRectangles]; // list of coloured rectangles
    std::vector<Triangle> coloured_triangles;    //[NumColouredTriangles]; // list of coloured triangles

    /** \brief reads mesh definition.
      *
      * The read num_normals value is positive when normals are available and negative when light
      * values are available. The values get set appropiatly.
      */
    static std::unique_ptr<Mesh> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Mesh> mesh{ new Mesh() };
        mesh->centre = Vertex::read16(reader);
        mesh->collision_size = reader.readI32();

        mesh->vertices.resize(reader.readI16());
        for(size_t i = 0; i < mesh->vertices.size(); i++)
            mesh->vertices[i] = Vertex::read16(reader);

        auto num_normals = reader.readI16();
        if(num_normals >= 0)
        {
            mesh->normals.resize(num_normals);
            for(size_t i = 0; i < mesh->normals.size(); i++)
                mesh->normals[i] = Vertex::read16(reader);
        }
        else
        {
            mesh->lights.resize(-num_normals);
            for(size_t i = 0; i < mesh->lights.size(); i++)
                mesh->lights[i] = reader.readI16();
        }

        mesh->textured_rectangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->textured_rectangles.size(); i++)
            mesh->textured_rectangles[i] = QuadFace::readTr1(reader);

        mesh->textured_triangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->textured_triangles.size(); i++)
            mesh->textured_triangles[i] = Triangle::readTr1(reader);

        mesh->coloured_rectangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->coloured_rectangles.size(); i++)
            mesh->coloured_rectangles[i] = QuadFace::readTr1(reader);

        mesh->coloured_triangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->coloured_triangles.size(); i++)
            mesh->coloured_triangles[i] = Triangle::readTr1(reader);
        return mesh;
    }

    static std::unique_ptr<Mesh> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<Mesh> mesh{ new Mesh() };
        mesh->centre = Vertex::read16(reader);
        mesh->collision_size = reader.readI32();

        mesh->vertices.resize(reader.readI16());
        for(size_t i = 0; i < mesh->vertices.size(); i++)
            mesh->vertices[i] = Vertex::read16(reader);

        auto num_normals = reader.readI16();
        if(num_normals >= 0)
        {
            mesh->normals.resize(num_normals);
            for(size_t i = 0; i < mesh->normals.size(); i++)
                mesh->normals[i] = Vertex::read16(reader);
        }
        else
        {
            mesh->lights.resize(-num_normals);
            for(size_t i = 0; i < mesh->lights.size(); i++)
                mesh->lights[i] = reader.readI16();
        }

        mesh->textured_rectangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->textured_rectangles.size(); i++)
            mesh->textured_rectangles[i] = QuadFace::readTr4(reader);

        mesh->textured_triangles.resize(reader.readI16());
        for(size_t i = 0; i < mesh->textured_triangles.size(); i++)
            mesh->textured_triangles[i] = Triangle::readTr4(reader);
        return mesh;
    }
};

struct StaticMesh
{
    uint32_t object_id;             // Object Identifier (matched in Items[])
    uint16_t mesh;                  // mesh (offset into MeshPointers[])
    Vertex visibility_box[2];
    Vertex collision_box[2];
    uint16_t flags;                 // Meaning uncertain; it is usually 2, and is 3 for objects Lara can travel through,
    // like TR2's skeletons and underwater vegetation

    static std::unique_ptr<StaticMesh> read(io::SDLReader& reader)
    {
        std::unique_ptr<StaticMesh> mesh{ new StaticMesh() };
        mesh->object_id = reader.readU32();
        mesh->mesh = reader.readU16();

        mesh->visibility_box[0].x = static_cast<float>(reader.readI16());
        mesh->visibility_box[1].x = static_cast<float>(reader.readI16());
        mesh->visibility_box[0].y = static_cast<float>(-reader.readI16());
        mesh->visibility_box[1].y = static_cast<float>(-reader.readI16());
        mesh->visibility_box[0].z = static_cast<float>(-reader.readI16());
        mesh->visibility_box[1].z = static_cast<float>(-reader.readI16());

        mesh->collision_box[0].x = static_cast<float>(reader.readI16());
        mesh->collision_box[1].x = static_cast<float>(reader.readI16());
        mesh->collision_box[0].y = static_cast<float>(-reader.readI16());
        mesh->collision_box[1].y = static_cast<float>(-reader.readI16());
        mesh->collision_box[0].z = static_cast<float>(-reader.readI16());
        mesh->collision_box[1].z = static_cast<float>(-reader.readI16());

        mesh->flags = reader.readU16();
        return mesh;
    }
};

/** \brief MeshTree.
  *
  * MeshTree[] is actually groups of four bit32s. The first one is a
  * "flags" word;
  *    bit 1 (0x0002) indicates "put the parent mesh on the mesh stack";
  *    bit 0 (0x0001) indicates "take the top mesh off of the mesh stack and use as the parent mesh"
  * when set, otherwise "use the previous mesh are the parent mesh".
  * When both are present, the bit-0 operation is always done before the bit-1 operation; in effect, read the stack but do not change it.
  * The next three bit32s are X, Y, Z offsets of the mesh's origin from the parent mesh's origin.
  */
struct MeshTree
{
    uint32_t flags;
    Vertex offset;
};

/** \brief Frame.
  *
  * Frames indicates how composite meshes are positioned and rotated.
  * They work in conjunction with Animations[] and Bone2[].
  *
  * A given frame has the following format:
  *    short BB1x, BB1y, BB1z           // bounding box (low)
  *    short BB2x, BB2y, BB2z           // bounding box (high)
  *    short OffsetX, OffsetY, OffsetZ  // starting offset for this moveable
  *    (TR1 ONLY: short NumValues       // number of angle sets to follow)
  *    (TR2/3: NumValues is implicitly NumMeshes (from moveable))
  *
  * What follows next is a list of angle sets.  In TR2/3, an angle set can
  * specify either one or three axes of rotation.  If either of the high two
  * bits (0xc000) of the first angle unsigned short are set, it's one axis:
  *  only one  unsigned short,
  *  low 10 bits (0x03ff),
  *  scale is 0x100 == 90 degrees;
  * the high two  bits are interpreted as follows:
  *  0x4000 == X only, 0x8000 == Y only,
  *  0xC000 == Z only.
  *
  * If neither of the high bits are set, it's a three-axis rotation.  The next
  * 10 bits (0x3ff0) are the X rotation, the next 10 (including the following
  * unsigned short) (0x000f, 0xfc00) are the Y rotation,
  * the next 10 (0x03ff) are the Z rotation, same scale as
  * before (0x100 == 90 degrees).
  *
  * Rotations are performed in Y, X, Z order.
  * TR1 ONLY: All angle sets are two words and interpreted like the two-word
  * sets in TR2/3, EXCEPT that the word order is reversed.
  *
  */
  /*typedef struct {
      tr5_vertex_t bbox_low;
      tr5_vertex_t bbox_high;
      tr5_vertex_t offset;
      tr5_vertex_array_t rotations;
      int32_t byte_offset;
  } tr_frame_t;
  typedef prtl::array < tr_frame_t > tr_frame_array_t;*/

struct AnimatedModel
{
    uint32_t object_id;         // Item Identifier (matched in Items[])
    uint16_t num_meshes;        // number of meshes in this object
    uint16_t firstMesh;     // starting mesh (offset into MeshPointers[])
    uint32_t mesh_tree_index;   // offset into MeshTree[]
    uint32_t poseDataOffset;      // byte offset into Frames[] (divide by 2 for Frames[i])
    uint16_t animation_index;   // offset into Animations[]

    /** \brief reads a moveable definition.
      *
      * some sanity checks get done which throw a exception on failure.
      * frame_offset needs to be corrected later in TR_Level::read_tr_level.
      */
    static std::unique_ptr<AnimatedModel> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<AnimatedModel> moveable{ new AnimatedModel() };
        moveable->object_id = reader.readU32();
        moveable->num_meshes = reader.readU16();
        moveable->firstMesh = reader.readU16();
        moveable->mesh_tree_index = reader.readU32();
        moveable->poseDataOffset = reader.readU32();
        moveable->animation_index = reader.readU16();
        return moveable;
    }

    static std::unique_ptr<AnimatedModel> readTr5(io::SDLReader& reader)
    {
        std::unique_ptr<AnimatedModel> moveable = readTr1(reader);
        if(reader.readU16() != 0xFFEF)
            BOOST_LOG_TRIVIAL(warning) << "TR5 Moveable: filler has wrong value";
        return moveable;
    }
};

struct Item
{
    int16_t object_id;     // Object Identifier (matched in Moveables[], or SpriteSequences[], as appropriate)
    int16_t room;          // which room contains this item
    Vertex position;      // world coords
    float rotation;        // ((0xc000 >> 14) * 90) degrees
    int16_t intensity1;    // (constant lighting; -1 means use mesh lighting)
    int16_t intensity2;    // Like Intensity1, and almost always with the same value. [absent from TR1 data files]
    int16_t ocb;           // Object code bit - used for altering entity behaviour. Only in TR4-5.
    uint16_t flags;        // 0x0100 indicates "initially invisible", 0x3e00 is Activation Mask
    // 0x3e00 indicates "open" or "activated";  these can be XORed with
    // related FloorData::FDlist fields (e.g. for switches)

    uint16_t getActivationMask() const
    {
        return (flags & 0x3e00) >> 9;
    }

    bool isInitiallyInvisible() const
    {
        return (flags & 0x0100) != 0;
    }

    /// \brief reads an item definition.
    static std::unique_ptr<Item> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ new Item() };
        item->object_id = reader.readI16();
        item->room = reader.readI16();
        item->position = Vertex::read32(reader);
        item->rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        item->intensity1 = reader.readU16();
        if(item->intensity1 >= 0)
            item->intensity1 = (8191 - item->intensity1) << 2;
        item->intensity2 = item->intensity1;
        item->ocb = 0;   // Not present in TR1!
        item->flags = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ new Item() };
        item->object_id = reader.readI16();
        item->room = reader.readI16();
        item->position = Vertex::read32(reader);
        item->rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        item->intensity1 = reader.readU16();
        if(item->intensity1 >= 0)
            item->intensity1 = (8191 - item->intensity1) << 2;
        item->intensity2 = reader.readU16();
        if(item->intensity2 >= 0)
            item->intensity2 = (8191 - item->intensity2) << 2;
        item->ocb = 0;   // Not present in TR2!
        item->flags = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr3(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ new Item() };
        item->object_id = reader.readI16();
        item->room = reader.readI16();
        item->position = Vertex::read32(reader);
        item->rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        item->intensity1 = reader.readU16();
        item->intensity2 = reader.readU16();
        item->ocb = 0;   // Not present in TR3!
        item->flags = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ new Item() };
        item->object_id = reader.readI16();
        item->room = reader.readI16();
        item->position = Vertex::read32(reader);
        item->rotation = static_cast<float>(reader.readU16()) / 16384.0f * -90;
        item->intensity1 = reader.readU16();
        item->intensity2 = item->intensity1;
        item->ocb = reader.readU16();
        item->flags = reader.readU16();
        return item;
    }
};

struct SpriteTexture
{
    uint16_t        tile;
    int16_t         x0;        // tex coords
    int16_t         y0;        //
    int16_t         x1;        //
    int16_t         y1;        //

    int16_t         left_side;
    int16_t         top_side;
    int16_t         right_side;
    int16_t         bottom_side;

    /** \brief reads sprite texture definition.
      *
      * some sanity checks get done and if they fail an exception gets thrown.
      */
    static std::unique_ptr<SpriteTexture> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<SpriteTexture> sprite_texture{ new SpriteTexture() };

        sprite_texture->tile = reader.readU16();
        if(sprite_texture->tile > 64)
            BOOST_LOG_TRIVIAL(warning) << "TR1 Sprite Texture: tile > 64";

        int tx = reader.readU8();
        int ty = reader.readU8();
        int tw = reader.readU16();
        int th = reader.readU16();
        int tleft = reader.readI16();
        int ttop = reader.readI16();
        int tright = reader.readI16();
        int tbottom = reader.readI16();

        float w = tw / 256.0f;
        float h = th / 256.0f;
        sprite_texture->x0 = tx;
        sprite_texture->y0 = ty;
        sprite_texture->x1 = static_cast<int16_t>(sprite_texture->x0 + w);
        sprite_texture->y1 = static_cast<int16_t>(sprite_texture->y0 + h);

        sprite_texture->left_side = tleft;
        sprite_texture->right_side = tright;
        sprite_texture->top_side = -tbottom;
        sprite_texture->bottom_side = -ttop;
        return sprite_texture;
    }

    static std::unique_ptr<SpriteTexture> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<SpriteTexture> sprite_texture{ new SpriteTexture() };
        sprite_texture->tile = reader.readU16();
        if(sprite_texture->tile > 128)
            BOOST_LOG_TRIVIAL(warning) << "TR4 Sprite Texture: tile > 128";

        int tx = reader.readU8();
        int ty = reader.readU8();
        int tw = reader.readU16();
        int th = reader.readU16();
        int tleft = reader.readI16();
        int ttop = reader.readI16();
        int tright = reader.readI16();
        int tbottom = reader.readI16();

        sprite_texture->x0 = tleft;
        sprite_texture->x1 = tright;
        sprite_texture->y0 = tbottom;
        sprite_texture->y1 = ttop;

        sprite_texture->left_side = tx;
        sprite_texture->right_side = tx + tw / 256;
        sprite_texture->bottom_side = ty;
        sprite_texture->top_side = ty + th / 256;
        return sprite_texture;
    }
};

struct SpriteSequence
{
    int32_t object_id;     // Item identifier (matched in Items[])
    int16_t length;        // negative of "how many sprites are in this sequence"
    int16_t offset;        // where (in sprite texture list) this sequence starts

    /** \brief reads sprite sequence definition.
      *
      * length is negative when read and thus gets negated.
      */
    static std::unique_ptr<SpriteSequence> read(io::SDLReader& reader)
    {
        std::unique_ptr<SpriteSequence> sprite_sequence{ new SpriteSequence() };
        sprite_sequence->object_id = reader.readI32();
        sprite_sequence->length = -reader.readI16();
        sprite_sequence->offset = reader.readI16();
        return sprite_sequence;
    }
};

/** \brief animation->
  *
  * This describes each individual animation; these may be looped by specifying
  * the next animation to be itself. In TR2 and TR3, one must be careful when
  * parsing frames using the FrameSize value as the size of each frame, since
  * an animation's frame range may extend into the next animation's frame range,
  * and that may have a different FrameSize value.
  */
struct Animation
{
    uint32_t poseDataOffset;      // byte offset into Frames[] (divide by 2 for Frames[i])
    uint8_t stretchFactor;      // Slowdown factor of this animation
    uint8_t poseDataSize;         // number of bit16's in Frames[] used by this animation
    uint16_t state_id;

    int32_t   speed;
    int32_t   accel;

    int32_t   speed_lateral;      // new in TR4 -->
    int32_t   accel_lateral;      // lateral speed and acceleration.

    uint16_t firstFrame;           // first frame in this animation
    uint16_t lastFrame;             // last frame in this animation (numframes = (End - Start) + 1)
    uint16_t next_animation;
    uint16_t nextFrame;

    uint16_t num_state_changes;
    uint16_t state_change_offset;   // offset into StateChanges[]
    uint16_t num_anim_commands;     // How many of them to use.
    uint16_t anim_command;          // offset into AnimCommand[]

    /// \brief reads an animation definition.
    static std::unique_ptr<Animation> readTr1(io::SDLReader& reader)
    {
        return read(reader, false);
    }

    static std::unique_ptr<Animation> readTr4(io::SDLReader& reader)
    {
        return read(reader, true);
    }

private:
    static std::unique_ptr<Animation> read(io::SDLReader& reader, bool withLateral)
    {
        std::unique_ptr<Animation> animation{ new Animation() };
        animation->poseDataOffset = reader.readU32();
        animation->stretchFactor = reader.readU8();
        animation->poseDataSize = reader.readU8();
        animation->state_id = reader.readU16();

        animation->speed = reader.readI32();
        animation->accel = reader.readI32();
        if(withLateral)
        {
            animation->speed_lateral = reader.readI32();
            animation->accel_lateral = reader.readI32();
        }
        else
        {
            animation->speed_lateral = 0;
            animation->accel_lateral = 0;
        }

        animation->firstFrame = reader.readU16();
        animation->lastFrame = reader.readU16();
        animation->next_animation = reader.readU16();
        animation->nextFrame = reader.readU16();

        animation->num_state_changes = reader.readU16();
        animation->state_change_offset = reader.readU16();
        animation->num_anim_commands = reader.readU16();
        animation->anim_command = reader.readU16();
        return animation;
    }
};

/** \brief State Change.
  *
  * Each one contains the state to change to and which animation dispatches
  * to use; there may be more than one, with each separate one covering a different
  * range of frames.
  */
struct StateChange
{
    uint16_t state_id;
    uint16_t num_anim_dispatches;       // number of ranges (seems to always be 1..5)
    uint16_t anim_dispatch;             // Offset into AnimDispatches[]

    /// \brief reads an animation state change.
    static std::unique_ptr<StateChange> read(io::SDLReader& reader)
    {
        std::unique_ptr<StateChange> state_change{ new StateChange() };
        state_change->state_id = reader.readU16();
        state_change->num_anim_dispatches = reader.readU16();
        state_change->anim_dispatch = reader.readU16();
        return state_change;
    }
};

/** \brief Animation Dispatch.
  *
  * This specifies the next animation and frame to use; these are associated
  * with some range of frames. This makes possible such specificity as one
  * animation for left foot forward and another animation for right foot forward.
  */
struct AnimDispatch
{
    int16_t low;                // Lowest frame that uses this range
    int16_t high;               // Highest frame (+1?) that uses this range
    int16_t next_animation;     // Animation to dispatch to
    int16_t nextFrame;         // Frame offset to dispatch to

    /// \brief reads an animation dispatch.
    static std::unique_ptr<AnimDispatch> read(io::SDLReader& reader)
    {
        std::unique_ptr<AnimDispatch> anim_dispatch{ new AnimDispatch() };
        anim_dispatch->low = reader.readI16();
        anim_dispatch->high = reader.readI16();
        anim_dispatch->next_animation = reader.readI16();
        anim_dispatch->nextFrame = reader.readI16();
        return anim_dispatch;
    }
};

struct Box
{
    uint32_t zmin;          // sectors (* 1024 units)
    uint32_t zmax;
    uint32_t xmin;
    uint32_t xmax;
    int16_t true_floor;     // Y value (no scaling)
    int16_t overlap_index;  // index into Overlaps[]. The high bit is sometimes set; this
    // occurs in front of swinging doors and the like.

    static std::unique_ptr<Box> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Box> box{ new Box() };
        box->zmax = -reader.readI32();
        box->zmin = -reader.readI32();
        box->xmin = reader.readI32();
        box->xmax = reader.readI32();
        box->true_floor = -reader.readI16();
        box->overlap_index = reader.readI16();
        return box;
    }

    static std::unique_ptr<Box> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Box> box{ new Box() };
        box->zmax = -1024 * reader.readU8();
        box->zmin = -1024 * reader.readU8();
        box->xmin = 1024 * reader.readU8();
        box->xmax = 1024 * reader.readU8();
        box->true_floor = -reader.readI16();
        box->overlap_index = reader.readI16();
        return box;
    }
};

struct Zone
{
    uint16_t flyZoneNormal;
    std::vector<uint16_t> groundZonesNormal;
    uint16_t flyZoneAlternate;
    std::vector<uint16_t> groundZonesAlternate;

    static std::unique_ptr<Zone> readTr1(io::SDLReader& reader)
    {
        return read(reader, 2);
    }

    static std::unique_ptr<Zone> readTr2(io::SDLReader& reader)
    {
        return read(reader, 4);
    }

private:
    static std::unique_ptr<Zone> read(io::SDLReader& reader, int n)
    {
        std::unique_ptr<Zone> zone{ new Zone() };
        zone->flyZoneNormal = reader.readU16();
        for(int i = 0; i < n; ++i)
            zone->groundZonesNormal.emplace_back(reader.readU16());
        zone->flyZoneAlternate = reader.readU16();
        for(int i = 0; i < n; ++i)
            zone->groundZonesAlternate.emplace_back(reader.readU16());
        return zone;
    }
};

/** \brief SoundSource.
  *
  * This structure contains the details of continuous-sound sources. Although
  * a SoundSource object has a position, it has no room membership; the sound
  * seems to propagate omnidirectionally for about 10 horizontal-grid sizes
  * without regard for the presence of walls.
  */
struct SoundSource
{
    int32_t x;              // absolute X position of sound source (world coordinates)
    int32_t y;              // absolute Y position of sound source (world coordinates)
    int32_t z;              // absolute Z position of sound source (world coordinates)
    uint16_t sound_id;      // internal sound index
    uint16_t flags;         // 0x40, 0x80, or 0xc0

    static std::unique_ptr<SoundSource> read(io::SDLReader& reader)
    {
        std::unique_ptr<SoundSource> sound_source{ new SoundSource() };
        sound_source->x = reader.readI32();
        sound_source->y = reader.readI32();
        sound_source->z = reader.readI32();

        sound_source->sound_id = reader.readU16();
        sound_source->flags = reader.readU16();
        return sound_source;
    }
};

// Looped field is located at offset 6 of SoundDetail structure and
// combined with SampleIndexes value. This field is responsible for
// looping behaviour of each sound.
// L flag sets sound to continous looped state, while W flag waits
// for any sound with similar ID to finish, and only then plays it
// again. R flag rewinds sound, if sound with similar ID is being
// sent to sources.
enum class LoopType
{
    None,
    Forward,
    PingPong,
    Wait
};

/** \brief SoundDetails.
 *
 * SoundDetails (also called SampleInfos in native TR sources) are properties
 * for each sound index from SoundMap. It contains all crucial information
 * that is needed to play certain sample, except offset to raw wave buffer,
 * which is unnecessary, as it is managed internally by DirectSound.
 */
struct SoundDetails
{
    size_t sample;                       // Index into SampleIndices -- NOT USED IN TR4-5!!!
    uint16_t volume;                     // Global sample value
    uint16_t sound_range;                // Sound range
    uint16_t chance;                     // Chance to play
    int16_t pitch;                       // Pitch shift
    uint8_t num_samples_and_flags_1;     // Bits 0-1: Looped flag, bits 2-5: num samples, bits 6-7: UNUSED
    uint8_t flags_2;                     // Bit 4: UNKNOWN, bit 5: Randomize pitch, bit 6: randomize volume
                                         // All other bits in flags_2 are unused.

    LoopType getLoopType(Engine engine) const
    {
        if(engine == Engine::TR1)
        {
            switch(num_samples_and_flags_1 & 3)
            {
                case 1:
                    return LoopType::PingPong;
                case 2:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
            }
        }
        else if(engine == Engine::TR2)
        {
            switch(num_samples_and_flags_1 & 3)
            {
                case 1:
                    return LoopType::PingPong;
                case 3:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
            }
        }
        else
        {
            switch(num_samples_and_flags_1 & 3)
            {
                case 1:
                    return LoopType::Wait;
                case 2:
                    return LoopType::PingPong;
                case 3:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
            }
        }
    }

    uint8_t getSampleCount() const
    {
        return (num_samples_and_flags_1 >> 2) & 0x0f;
    }

    bool useRandomPitch() const
    {
        return (flags_2 & 0x20) != 0;
    }

    bool useRandomVolume() const
    {
        return (flags_2 & 0x40) != 0;
    }

    // Default range and pitch values are required for compatibility with
    // TR1 and TR2 levels, as there is no such parameters in SoundDetails
    // structures.

    static constexpr const int DefaultRange = 8;
    static constexpr const float DefaultPitch = 1.0f;       // 0.0 - only noise

    static std::unique_ptr<SoundDetails> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<SoundDetails> sound_details{ new SoundDetails() };
        sound_details->sample = reader.readU16();
        sound_details->volume = reader.readU16();
        sound_details->chance = reader.readU16();
        sound_details->num_samples_and_flags_1 = reader.readU8();
        sound_details->flags_2 = reader.readU8();
        sound_details->sound_range = DefaultRange;
        sound_details->pitch = static_cast<int16_t>(DefaultPitch);
        return sound_details;
    }

    static std::unique_ptr<SoundDetails> readTr3(io::SDLReader& reader)
    {
        std::unique_ptr<SoundDetails> sound_details{ new SoundDetails() };
        sound_details->sample = reader.readU16();
        sound_details->volume = reader.readU8();
        sound_details->sound_range = reader.readU8();
        sound_details->chance = static_cast<uint16_t>(reader.readU8());
        sound_details->pitch = reader.readI8();
        sound_details->num_samples_and_flags_1 = reader.readU8();
        sound_details->flags_2 = reader.readU8();
        return sound_details;
    }
};

/** \brief Object Texture Vertex.
  *
  * It specifies a vertex location in textile coordinates.
  * The Xpixel and Ypixel are the actual coordinates of the vertex's pixel.
  * The Xcoordinate and Ycoordinate values depend on where the other vertices
  * are in the object texture. And if the object texture is used to specify
  * a triangle, then the fourth vertex's values will all be zero.
  */
struct ObjectTextureVertex
{
    int8_t xcoordinate;     // 1 if Xpixel is the low value, -1 if Xpixel is the high value in the object texture
    uint8_t xpixel;
    int8_t ycoordinate;     // 1 if Ypixel is the low value, -1 if Ypixel is the high value in the object texture
    uint8_t ypixel;

    /// \brief reads object texture vertex definition.
    static ObjectTextureVertex readTr1(io::SDLReader& reader)
    {
        ObjectTextureVertex vert;
        vert.xcoordinate = reader.readI8();
        vert.xpixel = reader.readU8();
        vert.ycoordinate = reader.readI8();
        vert.ypixel = reader.readU8();
        return vert;
    }

    static ObjectTextureVertex readTr4(io::SDLReader& reader)
    {
        ObjectTextureVertex vert;
        vert.xcoordinate = reader.readI8();
        vert.xpixel = reader.readU8();
        vert.ycoordinate = reader.readI8();
        vert.ypixel = reader.readU8();
        if(vert.xcoordinate == 0)
            vert.xcoordinate = 1;
        if(vert.ycoordinate == 0)
            vert.ycoordinate = 1;
        return vert;
    }
};

/** \brief Object Texture.
  *
  * These, thee contents of ObjectTextures[], are used for specifying texture
  * mapping for the world geometry and for mesh objects.
  */
enum class BlendingMode : uint16_t
{
    Opaque,
    Transparent,
    Multiply,
    SimpleShade,
    TransparentIgnoreZ,
    InvertSrc,
    Wireframe,
    TransparentAlpha,
    InvertDst,
    Screen,
    Hide,
    AnimatedTexture
};

struct ObjectTexture
{
    BlendingMode transparency_flags;    // 0 means that a texture is all-opaque, and that transparency
    // information is ignored.
    // 1 means that transparency information is used. In 8-bit colour,
    // index 0 is the transparent colour, while in 16-bit colour, the
    // top bit (0x8000) is the alpha channel (1 = opaque, 0 = transparent).
    // 2 (only in TR3) means that the opacity (alpha) is equal to the intensity;
    // the brighter the colour, the more opaque it is. The intensity is probably calculated
    // as the maximum of the individual color values.
    uint16_t tileAndFlag;                     // index into textile list
    uint16_t flags;                             // TR4
    ObjectTextureVertex vertices[4];      // the four corners of the texture
    uint32_t unknown1;                          // TR4
    uint32_t unknown2;                          // TR4
    uint32_t x_size;                            // TR4
    uint32_t y_size;                            // TR4

    /** \brief reads object texture definition.
      *
      * some sanity checks get done and if they fail an exception gets thrown.
      * all values introduced in TR4 get set appropiatly.
      */
    static std::unique_ptr<ObjectTexture> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<ObjectTexture> object_texture{ new ObjectTexture() };
        object_texture->transparency_flags = static_cast<BlendingMode>(reader.readU16());
        object_texture->tileAndFlag = reader.readU16();
        if(object_texture->tileAndFlag > 64)
            BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag > 64";

        if((object_texture->tileAndFlag & (1 << 15)) != 0)
            BOOST_LOG_TRIVIAL(warning) << "TR1 Object Texture: tileAndFlag is flagged";

        // only in TR4
        object_texture->flags = 0;
        object_texture->vertices[0] = ObjectTextureVertex::readTr1(reader);
        object_texture->vertices[1] = ObjectTextureVertex::readTr1(reader);
        object_texture->vertices[2] = ObjectTextureVertex::readTr1(reader);
        object_texture->vertices[3] = ObjectTextureVertex::readTr1(reader);
        // only in TR4
        object_texture->unknown1 = 0;
        object_texture->unknown2 = 0;
        object_texture->x_size = 0;
        object_texture->y_size = 0;
        return object_texture;
    }

    static std::unique_ptr<ObjectTexture> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<ObjectTexture> object_texture{ new ObjectTexture() };
        object_texture->transparency_flags = static_cast<BlendingMode>(reader.readU16());
        object_texture->tileAndFlag = reader.readU16();
        if((object_texture->tileAndFlag & 0x7FFF) > 128)
            BOOST_LOG_TRIVIAL(warning) << "TR4 Object Texture: tileAndFlag > 128";

        object_texture->flags = reader.readU16();
        object_texture->vertices[0] = ObjectTextureVertex::readTr4(reader);
        object_texture->vertices[1] = ObjectTextureVertex::readTr4(reader);
        object_texture->vertices[2] = ObjectTextureVertex::readTr4(reader);
        object_texture->vertices[3] = ObjectTextureVertex::readTr4(reader);
        object_texture->unknown1 = reader.readU32();
        object_texture->unknown2 = reader.readU32();
        object_texture->x_size = reader.readU32();
        object_texture->y_size = reader.readU32();
        return object_texture;
    }

    static std::unique_ptr<ObjectTexture> readTr5(io::SDLReader& reader)
    {
        std::unique_ptr<ObjectTexture> object_texture = readTr4(reader);
        if(reader.readU16() != 0)
        {
            BOOST_LOG_TRIVIAL(warning) << "TR5 Object Texture: unexpected value at end of structure";
        }
        return object_texture;
    }
};

struct AnimatedTexture
{
    std::vector<int16_t> texture_ids; // offsets into ObjectTextures[], in animation order.
};       //[NumAnimatedTextures];

struct Camera
{
    int32_t x;
    int32_t y;
    int32_t z;
    int16_t room;
    uint16_t unknown1;    // correlates to Boxes[]? Zones[]?

    static std::unique_ptr<Camera> read(io::SDLReader& reader)
    {
        std::unique_ptr<Camera> camera{ new Camera() };
        camera->x = reader.readI32();
        camera->y = reader.readI32();
        camera->z = reader.readI32();

        camera->room = reader.readI16();
        camera->unknown1 = reader.readU16();
        return camera;
    }
};

struct FlybyCamera
{
    int32_t  cam_x;
    int32_t  cam_y;
    int32_t  cam_z;
    int32_t  target_x;
    int32_t  target_y;
    int32_t  target_z;
    uint8_t  sequence;
    uint8_t  index;
    uint16_t fov;
    uint16_t roll;
    uint16_t timer;
    uint16_t speed;
    uint16_t flags;
    uint32_t room_id;

    static std::unique_ptr<FlybyCamera> read(io::SDLReader& reader)
    {
        std::unique_ptr<FlybyCamera> camera{ new FlybyCamera() };
        camera->cam_x = reader.readI32();
        camera->cam_y = reader.readI32();
        camera->cam_z = reader.readI32();
        camera->target_x = reader.readI32();
        camera->target_y = reader.readI32();
        camera->target_z = reader.readI32();

        camera->sequence = reader.readI8();
        camera->index = reader.readI8();

        camera->fov = reader.readU16();
        camera->roll = reader.readU16();
        camera->timer = reader.readU16();
        camera->speed = reader.readU16();
        camera->flags = reader.readU16();

        camera->room_id = reader.readU32();
        return camera;
    }
};

struct AIObject
{
    uint16_t object_id;    // the objectID from the AI object (AI_FOLLOW is 402)
    uint16_t room;
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t ocb;
    uint16_t flags;        // The trigger flags (button 1-5, first button has value 2)
    int32_t angle;

    static std::unique_ptr<AIObject> read(io::SDLReader& reader)
    {
        std::unique_ptr<AIObject> object{ new AIObject() };
        object->object_id = reader.readU16();
        object->room = reader.readU16();                        // 4

        object->x = reader.readI32();
        object->y = reader.readI32();
        object->z = reader.readI32();                            // 16

        object->ocb = reader.readU16();
        object->flags = reader.readU16();                       // 20
        object->angle = reader.readI32();                        // 24
        return object;
    }
};

struct CinematicFrame
{
    int16_t roty;        // rotation about Y axis, +/- 32767 == +/- 180 degrees
    int16_t rotz;        // rotation about Z axis, +/- 32767 == +/- 180 degrees
    int16_t rotz2;       // seems to work a lot like rotZ;  I haven't yet been able to
    // differentiate them
    int16_t posz;        // camera position relative to something (target? Lara? room
    // origin?).  pos* are _not_ in world coordinates.
    int16_t posy;        // camera position relative to something (see posZ)
    int16_t posx;        // camera position relative to something (see posZ)
    int16_t unknown;     // changing this can cause a runtime error
    int16_t rotx;        // rotation about X axis, +/- 32767 == +/- 180 degrees

    /// \brief reads a cinematic frame
    static std::unique_ptr<CinematicFrame> read(io::SDLReader& reader)
    {
        std::unique_ptr<CinematicFrame> cf{ new CinematicFrame() };
        cf->roty = reader.readI16();         // rotation about Y axis, +/- 32767 == +/- 180 degrees
        cf->rotz = reader.readI16();         // rotation about Z axis, +/- 32767 == +/- 180 degrees
        cf->rotz2 = reader.readI16();        // seems to work a lot like rotZ;  I haven't yet been able to
                                           // differentiate them
        cf->posz = reader.readI16();         // camera position relative to something (target? Lara? room
                                           // origin?).  pos* are _not_ in world coordinates.
        cf->posy = reader.readI16();         // camera position relative to something (see posZ)
        cf->posx = reader.readI16();         // camera position relative to something (see posZ)
        cf->unknown = reader.readI16();      // changing this can cause a runtime error
        cf->rotx = reader.readI16();         // rotation about X axis, +/- 32767 == +/- 180 degrees
        return cf;
    }
};

struct LightMap
{
    std::array<uint8_t, 32 * 256> map;

    /// \brief reads the lightmap.
    static std::unique_ptr<LightMap> read(io::SDLReader& reader)
    {
        std::unique_ptr<LightMap> lightmap{ new LightMap() };
        reader.readBytes(lightmap->map.data(), lightmap->map.size());
        return lightmap;
    }
};

struct Palette
{
    ByteColor color[256];

    /// \brief reads the 256 colour palette values.
    static std::unique_ptr<Palette> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Palette> palette{ new Palette() };
        for(int i = 0; i < 256; i++)
            palette->color[i] = ByteColor::readTr1(reader);
        return palette;
    }

    static std::unique_ptr<Palette> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Palette> palette{ new Palette() };
        for(int i = 0; i < 256; i++)
            palette->color[i] = ByteColor::readTr2(reader);
        return palette;
    }
};

using FloorData = std::vector<uint16_t>;
} // namespace loader
