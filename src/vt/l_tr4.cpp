/*
 * Copyright 2002 - Florian Schulze <crow@icculus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This file is part of vt.
 *
 */

#include <SDL2/SDL_endian.h>
#include <zlib.h>
#include "l_main.h"
#include "tr_versions.h"
#include "../system.h"
#include "../audio.h"

#define RCSID "$Id: l_tr4.cpp,v 1.14 2002/09/20 15:59:02 crow Exp $"

void TR_Level::read_tr4_vertex_float(SDL_RWops * const src, tr5_vertex_t & vertex)
{
    vertex.x = read_float(src);
    vertex.y = -read_float(src);
    vertex.z = -read_float(src);
}

void TR_Level::read_tr4_textile32(SDL_RWops * const src, tr4_textile32_t & textile)
{
    for (int i = 0; i < 256; i++) {
        if (SDL_RWread(src, textile.pixels[i], 4, 256) < 256)
            Sys_extError("read_tr4_textile32");

        for (int j = 0; j < 256; j++)
            textile.pixels[i][j] = SDL_SwapLE32((textile.pixels[i][j] & 0xff00ff00) | ((textile.pixels[i][j] & 0x00ff0000) >> 16) | ((textile.pixels[i][j] & 0x000000ff) << 16));
    }
}

void TR_Level::read_tr4_face3(SDL_RWops * const src, tr4_face3_t & meshface)
{
    meshface.vertices[0] = read_bitu16(src);
    meshface.vertices[1] = read_bitu16(src);
    meshface.vertices[2] = read_bitu16(src);
    meshface.texture = read_bitu16(src);
    meshface.lighting = read_bitu16(src);
}

void TR_Level::read_tr4_face4(SDL_RWops * const src, tr4_face4_t & meshface)
{
    meshface.vertices[0] = read_bitu16(src);
    meshface.vertices[1] = read_bitu16(src);
    meshface.vertices[2] = read_bitu16(src);
    meshface.vertices[3] = read_bitu16(src);
    meshface.texture = read_bitu16(src);
    meshface.lighting = read_bitu16(src);
}

void TR_Level::read_tr4_room_light(SDL_RWops * const src, tr5_room_light_t & light)
{
    read_tr_vertex32(src, light.pos);
    read_tr_colour(src, light.color);
    light.light_type = read_bitu8(src);
    light.unknown = read_bitu8(src);
    light.intensity1 = read_bitu8(src);
    light.intensity = light.intensity1;
    light.intensity /= 32;
    light.r_inner = read_float(src);
    light.r_outer = read_float(src);
    light.length = read_float(src);
    light.cutoff = read_float(src);
    read_tr4_vertex_float(src, light.dir);
}

void TR_Level::read_tr4_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex)
{
    read_tr_vertex16(src, room_vertex.vertex);
    // read and make consistent
    room_vertex.lighting1 = read_bit16(src);
    room_vertex.attributes = read_bitu16(src);
    room_vertex.lighting2 = read_bit16(src);
    // only in TR5
    room_vertex.normal.x = 0;
    room_vertex.normal.y = 0;
    room_vertex.normal.z = 0;

    room_vertex.colour.r = ((room_vertex.lighting2 & 0x7C00) >> 10  ) / 31.0f;
     room_vertex.colour.g = ((room_vertex.lighting2 & 0x03E0) >> 5   ) / 31.0f;
     room_vertex.colour.b = ((room_vertex.lighting2 & 0x001F)        ) / 31.0f;
    room_vertex.colour.a = 1.0f;
}

void TR_Level::read_tr4_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh)
{
    read_tr_vertex32(src, room_static_mesh.pos);
    room_static_mesh.rotation = (float)read_bitu16(src) / 16384.0f * -90;
    room_static_mesh.intensity1 = read_bit16(src);
    room_static_mesh.intensity2 = read_bit16(src);
    room_static_mesh.object_id = read_bitu16(src);

    room_static_mesh.tint.r = ((room_static_mesh.intensity1 & 0x001F)        ) / 31.0f;

    room_static_mesh.tint.g = ((room_static_mesh.intensity1 & 0x03E0) >> 5   ) / 31.0f;

    room_static_mesh.tint.b = ((room_static_mesh.intensity1 & 0x7C00) >> 10  ) / 31.0f;
    room_static_mesh.tint.a = 1.0f;
}

void TR_Level::read_tr4_room(SDL_RWops * const src, tr5_room_t & room)
{
    uint32_t num_data_words;
    uint32_t i;
    int64_t pos;

    // read and change coordinate system
    room.offset.x = (float)read_bit32(src);
    room.offset.y = 0;
    room.offset.z = (float)-read_bit32(src);
    room.y_bottom = (float)-read_bit32(src);
    room.y_top = (float)-read_bit32(src);

    num_data_words = read_bitu32(src);

    pos = SDL_RWseek(src, 0, SEEK_CUR);

    room.vertices.resize( read_bitu16(src) );
    for (i = 0; i < room.vertices.size(); i++)
        read_tr4_room_vertex(src, room.vertices[i]);

    room.rectangles.resize( read_bitu16(src) );
    for (i = 0; i < room.rectangles.size(); i++)
        read_tr_face4(src, room.rectangles[i]);

    room.triangles.resize( read_bitu16(src) );
    for (i = 0; i < room.triangles.size(); i++)
        read_tr_face3(src, room.triangles[i]);

    room.sprites.resize( read_bitu16(src) );
    for (i = 0; i < room.sprites.size(); i++)
        read_tr_room_sprite(src, room.sprites[i]);

    // set to the right position in case that there is some unused data
    SDL_RWseek(src, pos + (num_data_words * 2), SEEK_SET);

    room.portals.resize( read_bitu16(src) );
    for (i = 0; i < room.portals.size(); i++)
        read_tr_room_portal(src, room.portals[i]);

    room.num_zsectors = read_bitu16(src);
    room.num_xsectors = read_bitu16(src);
    room.sector_list.resize(room.num_zsectors * room.num_xsectors);
    for (i = 0; i < (uint32_t)(room.num_zsectors * room.num_xsectors); i++)
        read_tr_room_sector(src, room.sector_list[i]);

    room.intensity1 = read_bit16(src);
    room.intensity2 = read_bit16(src);

    // only in TR2
    room.light_mode = 0;

    room.lights.resize( read_bitu16(src) );
    for (i = 0; i < room.lights.size(); i++)
        read_tr4_room_light(src, room.lights[i]);

    room.static_meshes.resize( read_bitu16(src) );
    for (i = 0; i < room.static_meshes.size(); i++)
        read_tr4_room_staticmesh(src, room.static_meshes[i]);

    room.alternate_room = read_bit16(src);
    room.flags = read_bitu16(src);

    // Only in TR3-5

    room.water_scheme = read_bitu8(src);
    room.reverb_info = read_bitu8(src);

    // Only in TR4-5

    room.alternate_group = read_bitu8(src);

    room.light_colour.r = (( room.intensity2 & 0x00FF)       / 255.0f);
    room.light_colour.g = (((room.intensity1 & 0xFF00) >> 8) / 255.0f);
    room.light_colour.b = (( room.intensity1 & 0x00FF)       / 255.0f);
    room.light_colour.a = (((room.intensity2 & 0xFF00) >> 8) / 255.0f);
}

void TR_Level::read_tr4_item(SDL_RWops * const src, tr2_item_t & item)
{
    item.object_id = read_bit16(src);
    item.room = read_bit16(src);
    read_tr_vertex32(src, item.pos);
    item.rotation = (float)read_bitu16(src) / 16384.0f * -90;
    item.intensity1 = read_bitu16(src);
    item.intensity2 = item.intensity1;
    item.ocb = read_bitu16(src);
    item.flags = read_bitu16(src);
}

void TR_Level::read_tr4_object_texture_vert(SDL_RWops * const src, tr4_object_texture_vert_t & vert)
{
    vert.xcoordinate = read_bit8(src);
    vert.xpixel = read_bitu8(src);
    vert.ycoordinate = read_bit8(src);
    vert.ypixel = read_bitu8(src);
    if (vert.xcoordinate == 0)
        vert.xcoordinate = 1;
    if (vert.ycoordinate == 0)
        vert.ycoordinate = 1;
}

void TR_Level::read_tr4_object_texture(SDL_RWops * const src, tr4_object_texture_t & object_texture)
{
    object_texture.transparency_flags = read_bitu16(src);
    object_texture.tile_and_flag = read_bitu16(src);
    if ((object_texture.tile_and_flag & 0x7FFF) > 128)
        Sys_extWarn("object_texture.tile > 128");

    object_texture.flags = read_bitu16(src);
    read_tr4_object_texture_vert(src, object_texture.vertices[0]);
    read_tr4_object_texture_vert(src, object_texture.vertices[1]);
    read_tr4_object_texture_vert(src, object_texture.vertices[2]);
    read_tr4_object_texture_vert(src, object_texture.vertices[3]);
    object_texture.unknown1 = read_bitu32(src);
    object_texture.unknown2 = read_bitu32(src);
    object_texture.x_size = read_bitu32(src);
    object_texture.y_size = read_bitu32(src);
}

 /*
  * tr4 + sprite loading
  */
void TR_Level::read_tr4_sprite_texture(SDL_RWops * const src, tr_sprite_texture_t & sprite_texture)
{
    int tx, ty, tw, th, tleft, tright, ttop, tbottom;

    sprite_texture.tile = read_bitu16(src);
    if (sprite_texture.tile > 128)
        Sys_extWarn("sprite_texture.tile > 128");

    tx = read_bitu8(src);
    ty = read_bitu8(src);
    tw = read_bitu16(src);
    th = read_bitu16(src);
    tleft = read_bit16(src);
    ttop = read_bit16(src);
    tright = read_bit16(src);
    tbottom = read_bit16(src);

    sprite_texture.x0 = tleft;
    sprite_texture.x1 = tright;
    sprite_texture.y0 = tbottom;
    sprite_texture.y1 = ttop;

    sprite_texture.left_side = tx;
    sprite_texture.right_side = tx + tw / (256);
    sprite_texture.bottom_side = ty;
    sprite_texture.top_side = ty + th / (256);
}

void TR_Level::read_tr4_mesh(SDL_RWops * const src, tr4_mesh_t & mesh)
{
    int i;

    read_tr_vertex16(src, mesh.centre);
    mesh.collision_size = read_bit32(src);

    mesh.vertices.resize( read_bit16(src) );
    for (i = 0; i < mesh.vertices.size(); i++)
        read_tr_vertex16(src, mesh.vertices[i]);

    auto num_normals = read_bit16(src);
    if (num_normals >= 0)
    {
        mesh.normals.resize( num_normals );
        for (i = 0; i < mesh.normals.size(); i++)
            read_tr_vertex16(src, mesh.normals[i]);
    }
    else
    {
        mesh.lights.resize( -num_normals );
        for (i = 0; i < mesh.lights.size(); i++)
            mesh.lights[i] = read_bit16(src);
    }

    mesh.textured_rectangles.resize( read_bit16(src) );
    for (i = 0; i < mesh.textured_rectangles.size(); i++)
        read_tr4_face4(src, mesh.textured_rectangles[i]);

    mesh.textured_triangles.resize( read_bit16(src) );
    for (i = 0; i < mesh.textured_triangles.size(); i++)
        read_tr4_face3(src, mesh.textured_triangles[i]);
}

/// \brief reads an animation definition.
void TR_Level::read_tr4_animation(SDL_RWops * const src, tr_animation_t & animation)
{
    animation.frame_offset = read_bitu32(src);
    animation.frame_rate = read_bitu8(src);
    animation.frame_size = read_bitu8(src);
    animation.state_id = read_bitu16(src);

    animation.speed = read_mixfloat(src);
    animation.accel = read_mixfloat(src);
    animation.speed_lateral = read_mixfloat(src);
    animation.accel_lateral = read_mixfloat(src);

    animation.frame_start = read_bitu16(src);
    animation.frame_end = read_bitu16(src);
    animation.next_animation = read_bitu16(src);
    animation.next_frame = read_bitu16(src);

    animation.num_state_changes = read_bitu16(src);
    animation.state_change_offset = read_bitu16(src);
    animation.num_anim_commands = read_bitu16(src);
    animation.anim_command = read_bitu16(src);
}

void TR_Level::read_tr4_level(SDL_RWops * const _src)
{
    SDL_RWops *src = _src;
    uint32_t i;
    uint8_t *uncomp_buffer = NULL;
    uint8_t *comp_buffer = NULL;
    SDL_RWops *newsrc = NULL;

    // Version
    uint32_t file_version = read_bitu32(src);

    if (file_version != 0x00345254 /*&& file_version != 0x63345254*/)           // +TRLE
            Sys_extError("Wrong level version");

    m_numTextiles = 0;
    m_numRoomTextiles = 0;
    m_numObjTextiles = 0;
    m_numBumpTextiles = 0;
    m_numMiscTextiles = 0;
    m_read32BitTextiles = false;

    {
        uint32_t uncomp_size;
        uint32_t comp_size;
        unsigned long size;

        m_numRoomTextiles = read_bitu16(src);
        m_numObjTextiles = read_bitu16(src);
        m_numBumpTextiles = read_bitu16(src);
        m_numMiscTextiles = 2;
        m_numTextiles = m_numRoomTextiles + m_numObjTextiles + m_numBumpTextiles + m_numMiscTextiles;

        uncomp_size = read_bitu32(src);
        if (uncomp_size == 0)
            Sys_extError("read_tr4_level: textiles32 uncomp_size == 0");

        comp_size = read_bitu32(src);
        if (comp_size > 0)
        {
            uncomp_buffer = new uint8_t[uncomp_size];

            m_textile32.resize( m_numTextiles );
            comp_buffer = new uint8_t[comp_size];

            if (SDL_RWread(src, comp_buffer, 1, comp_size) < comp_size)
                Sys_extError("read_tr4_level: textiles32");

            size = uncomp_size;
            if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
                Sys_extError("read_tr4_level: uncompress");

            if (size != uncomp_size)
                Sys_extError("read_tr4_level: uncompress size mismatch");
            delete [] comp_buffer;

            comp_buffer = NULL;
            if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
                Sys_extError("read_tr4_level: SDL_RWFromMem");

            for (i = 0; i < (m_numTextiles - m_numMiscTextiles); i++)
                read_tr4_textile32(newsrc, m_textile32[i]);
            SDL_RWclose(newsrc);
            newsrc = NULL;
            delete [] uncomp_buffer;

            uncomp_buffer = NULL;
            m_read32BitTextiles = true;
        }

        uncomp_size = read_bitu32(src);
        if (uncomp_size == 0)
            Sys_extError("read_tr4_level: textiles16 uncomp_size == 0");

        comp_size = read_bitu32(src);
        if (comp_size > 0)
        {
            if (m_textile32.empty())
            {
                uncomp_buffer = new uint8_t[uncomp_size];

                m_textile16.resize( m_numTextiles );
                comp_buffer = new uint8_t[comp_size];

                if (SDL_RWread(src, comp_buffer, 1, comp_size) < comp_size)
                    Sys_extError("read_tr4_level: textiles16");

                size = uncomp_size;
                if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
                    Sys_extError("read_tr4_level: uncompress");

                if (size != uncomp_size)
                    Sys_extError("read_tr4_level: uncompress size mismatch");
                delete [] comp_buffer;

                comp_buffer = NULL;
                if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
                    Sys_extError("read_tr4_level: SDL_RWFromMem");

                for (i = 0; i < (m_numTextiles - m_numMiscTextiles); i++)
                    read_tr2_textile16(newsrc, m_textile16[i]);
                SDL_RWclose(newsrc);
                newsrc = NULL;
                delete [] uncomp_buffer;

                uncomp_buffer = NULL;
            }
            else
            {
                SDL_RWseek(src, comp_size, SEEK_CUR);
            }
        }

        uncomp_size = read_bitu32(src);
        if (uncomp_size == 0)
            Sys_extError("read_tr4_level: textiles32d uncomp_size == 0");

        comp_size = read_bitu32(src);
        if (comp_size > 0)
        {
            uncomp_buffer = new uint8_t[uncomp_size];

            if ((uncomp_size / (256 * 256 * 4)) > 2)
                Sys_extWarn("read_tr4_level: num_misc_textiles > 2");

            if (m_textile32.empty())
            {
                m_textile32.resize( m_numTextiles );
            }
            comp_buffer = new uint8_t[comp_size];

            if (SDL_RWread(src, comp_buffer, 1, comp_size) < comp_size)
                Sys_extError("read_tr4_level: misc_textiles");

            size = uncomp_size;
            if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
                Sys_extError("read_tr4_level: uncompress");

            if (size != uncomp_size)
                Sys_extError("read_tr4_level: uncompress size mismatch");
            delete [] comp_buffer;

            comp_buffer = NULL;
            if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
                Sys_extError("read_tr4_level: SDL_RWFromMem");

            for (i = (m_numTextiles - m_numMiscTextiles); i < m_numTextiles; i++)
                read_tr4_textile32(newsrc, m_textile32[i]);
            SDL_RWclose(newsrc);
            newsrc = NULL;
            delete [] uncomp_buffer;

            uncomp_buffer = NULL;
        }

        uncomp_size = read_bitu32(src);
        if (uncomp_size == 0)
            Sys_extError("read_tr4_level: packed geometry uncomp_size == 0");

        comp_size = read_bitu32(src);

        if (!comp_size)
            Sys_extError("read_tr4_level: packed geometry");

        uncomp_buffer = new uint8_t[uncomp_size];
        comp_buffer = new uint8_t[comp_size];

        if (SDL_RWread(src, comp_buffer, 1, comp_size) < comp_size)
            Sys_extError("read_tr4_level: packed geometry");

        size = uncomp_size;
        if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
            Sys_extError("read_tr4_level: uncompress");

        if (size != uncomp_size)
            Sys_extError("read_tr4_level: uncompress size mismatch");
        delete [] comp_buffer;

        comp_buffer = NULL;
        if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
            Sys_extError("read_tr4_level: SDL_RWFromMem");
    }

    // Unused
    if (read_bitu32(newsrc) != 0)
        Sys_extWarn("Bad value for 'unused'");

    m_rooms.resize( read_bitu16(newsrc) );
    for (i = 0; i < m_rooms.size(); i++)
        read_tr4_room(newsrc, m_rooms[i]);

    m_floorData.resize( read_bitu32(newsrc) );
    for(i = 0; i < m_floorData.size(); i++)
        m_floorData[i] = read_bitu16(newsrc);

    read_mesh_data(newsrc);

    m_animations.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_animations.size(); i++)
        read_tr4_animation(newsrc, m_animations[i]);

    m_stateChanges.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_stateChanges.size(); i++)
        read_tr_state_changes(newsrc, m_stateChanges[i]);

    m_animDispatches.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_animDispatches.size(); i++)
        read_tr_anim_dispatches(newsrc, m_animDispatches[i]);

    m_animCommands.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_animCommands.size(); i++)
        m_animCommands[i] = read_bit16(newsrc);

    m_meshTreeData.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_meshTreeData.size(); i++)
        m_meshTreeData[i] = read_bitu32(newsrc);                     // 4 bytes

    read_frame_moveable_data(newsrc);

    m_staticMeshes.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_staticMeshes.size(); i++)
        read_tr_staticmesh(newsrc, m_staticMeshes[i]);

    if (read_bit8(newsrc) != 'S')
        Sys_extError("read_tr4_level: 'SPR' not found");

    if (read_bit8(newsrc) != 'P')
        Sys_extError("read_tr4_level: 'SPR' not found");

    if (read_bit8(newsrc) != 'R')
        Sys_extError("read_tr4_level: 'SPR' not found");

    m_spriteTextures.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_spriteTextures.size(); i++)
        read_tr4_sprite_texture(newsrc, m_spriteTextures[i]);

    m_spriteSequences.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_spriteSequences.size(); i++)
        read_tr_sprite_sequence(newsrc, m_spriteSequences[i]);

    m_cameras.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_cameras.size(); i++)
    {
        m_cameras[i].x = read_bit32(newsrc);
        m_cameras[i].y = read_bit32(newsrc);
        m_cameras[i].z = read_bit32(newsrc);

        m_cameras[i].room = read_bit16(newsrc);
        m_cameras[i].unknown1 = read_bitu16(newsrc);
    }
    //SDL_RWseek(newsrc, this->cameras.size() * 16, SEEK_CUR);

    m_flybyCameras.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_flybyCameras.size(); i++)
    {
        m_flybyCameras[i].cam_x = read_bit32(newsrc);
        m_flybyCameras[i].cam_y = read_bit32(newsrc);
        m_flybyCameras[i].cam_z = read_bit32(newsrc);
        m_flybyCameras[i].target_x = read_bit32(newsrc);
        m_flybyCameras[i].target_y = read_bit32(newsrc);
        m_flybyCameras[i].target_z = read_bit32(newsrc);

        m_flybyCameras[i].sequence = read_bit8(newsrc);
        m_flybyCameras[i].index    = read_bit8(newsrc);

        m_flybyCameras[i].fov   = read_bitu16(newsrc);
        m_flybyCameras[i].roll  = read_bitu16(newsrc);
        m_flybyCameras[i].timer = read_bitu16(newsrc);
        m_flybyCameras[i].speed = read_bitu16(newsrc);
        m_flybyCameras[i].flags = read_bitu16(newsrc);

        m_flybyCameras[i].room_id = read_bitu32(newsrc);
    }
    //SDL_RWseek(newsrc, this->flyby_cameras.size() * 40, SEEK_CUR);

    m_soundSources.resize( read_bitu32(newsrc) );
    for(i = 0; i < m_soundSources.size(); i++)
    {
        m_soundSources[i].x = read_bit32(newsrc);
        m_soundSources[i].y = read_bit32(newsrc);
        m_soundSources[i].z = read_bit32(newsrc);

        m_soundSources[i].sound_id = read_bitu16(newsrc);
        m_soundSources[i].flags = read_bitu16(newsrc);
    }

    m_boxes.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_boxes.size(); i++)
        read_tr2_box(newsrc, m_boxes[i]);

    m_overlaps.resize(read_bitu32(newsrc));
    for (i = 0; i < m_overlaps.size(); i++)
        m_overlaps[i] = read_bitu16(newsrc);

    // Zones
    SDL_RWseek(newsrc, m_boxes.size() * 20, SEEK_CUR);

    m_animatedTextures.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_animatedTextures.size(); i++)
    {
        m_animatedTextures[i] = read_bitu16(newsrc);
    }

    m_animatedTexturesUvCount = read_bitu8(newsrc);

    if (read_bit8(newsrc) != 'T')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

    if (read_bit8(newsrc) != 'E')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

    if (read_bit8(newsrc) != 'X')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

    m_objectTextures.resize( read_bitu32(newsrc) );
    for (i = 0; i < m_objectTextures.size(); i++)
        read_tr4_object_texture(newsrc, m_objectTextures[i]);

    m_items.resize(read_bitu32(newsrc));
    for (i = 0; i < m_items.size(); i++)
        read_tr4_item(newsrc, m_items[i]);

    m_aiObjects.resize(read_bitu32(newsrc));
    for(i=0; i < m_aiObjects.size(); i++)
    {
        m_aiObjects[i].object_id = read_bitu16(newsrc);
        m_aiObjects[i].room = read_bitu16(newsrc);                        // 4

        m_aiObjects[i].x = read_bit32(newsrc);
        m_aiObjects[i].y = read_bit32(newsrc);
        m_aiObjects[i].z = read_bit32(newsrc);                            // 16

        m_aiObjects[i].ocb = read_bitu16(newsrc);
        m_aiObjects[i].flags = read_bitu16(newsrc);                       // 20
        m_aiObjects[i].angle = read_bit32(newsrc);                        // 24
    }

    m_demoData.resize( read_bitu16(newsrc) );
    for(i=0; i < m_demoData.size(); i++)
        m_demoData[i] = read_bitu8(newsrc);

    // Soundmap
    m_soundmap.resize(TR_AUDIO_MAP_SIZE_TR4);
    for(i=0; i < m_soundmap.size(); i++)
        m_soundmap[i] = read_bit16(newsrc);

    m_soundDetails.resize(read_bitu32(newsrc));
    for(i=0; i < m_soundDetails.size(); i++)
    {
        m_soundDetails[i].sample = read_bitu16(newsrc);
        m_soundDetails[i].volume = (uint16_t)read_bitu8(newsrc);        // n x 2.6
        m_soundDetails[i].sound_range = (uint16_t)read_bitu8(newsrc);   // n as is
        m_soundDetails[i].chance = (uint16_t)read_bitu8(newsrc);        // If n = 99, n = 0 (max. chance)
        m_soundDetails[i].pitch = (int16_t)read_bit8(newsrc);         // n as is
        m_soundDetails[i].num_samples_and_flags_1 = read_bitu8(newsrc);
        m_soundDetails[i].flags_2 = read_bitu8(newsrc);
    }

    // IMPORTANT NOTE: Sample indices ARE NOT USED in TR4 engine, but are parsed anyway.
    m_sampleIndices.resize( read_bitu32(newsrc) );
    for(i=0; i < m_sampleIndices.size(); i++)
        m_sampleIndices[i] = read_bitu32(newsrc);

    SDL_RWclose(newsrc);
    newsrc = NULL;

    delete [] uncomp_buffer;
    uncomp_buffer = NULL;

    // LOAD SAMPLES

    i = read_bitu32(src);   // Read num samples
    if(i)
    {
        m_samplesCount = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        m_samplesData.resize( (SDL_RWsize(src) - SDL_RWtell(src)) );
        for(i = 0; i < m_samplesData.size(); i++)
            m_samplesData[i] = read_bitu8(src);
    }
}
