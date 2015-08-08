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

#include <SDL2/SDL.h>
#include <zlib.h>
#include "l_main.h"
#include "../system.h"
#include "../audio.h"

#define RCSID "$Id: l_tr5.cpp,v 1.14 2002/09/20 15:59:02 crow Exp $"

void TR_Level::read_tr5_room_light(SDL_RWops * const src, tr5_room_light_t & light)
{
    uint32_t temp;

    read_tr4_vertex_float(src, light.pos);
    //read_tr_colour(src, light.color);
    light.color.r = read_float(src) * 255.0;    // r
    light.color.g = read_float(src) * 255.0;    // g
    light.color.b = read_float(src) * 255.0;    // b
    light.color.a = read_float(src) * 255.0;    // a
    light.color.a = 1.0f;
/*
    if ((temp != 0) && (temp != 0xCDCDCDCD))
        throw TR_ReadError("read_tr5_room_light: seperator1 has wrong value");
*/
    light.r_inner = read_float(src);
    light.r_outer = read_float(src);
    read_float(src);    // rad_input
    read_float(src);    // rad_output
    read_float(src);    // range
    read_tr4_vertex_float(src, light.dir);
    read_tr_vertex32(src, light.pos2);
    read_tr_vertex32(src, light.dir2);
    light.light_type = read_bitu8(src);
    temp = read_bitu8(src);
    if (temp != 0xCD)
        Sys_extWarn("read_tr5_room_light: seperator2 has wrong value");

    temp = read_bitu8(src);
    if (temp != 0xCD)
        Sys_extWarn("read_tr5_room_light: seperator3 has wrong value");

    temp = read_bitu8(src);
    if (temp != 0xCD)
        Sys_extWarn("read_tr5_room_light: seperator4 has wrong value");
}

void TR_Level::read_tr5_room_layer(SDL_RWops * const src, tr5_room_layer_t & layer)
{
    layer.num_vertices = read_bitu16(src);
    layer.unknown_l1 = read_bitu16(src);
    layer.unknown_l2 = read_bitu16(src);
    layer.num_rectangles = read_bitu16(src);
    layer.num_triangles = read_bitu16(src);
    layer.unknown_l3 = read_bitu16(src);
    layer.unknown_l4 = read_bitu16(src);
    if (read_bitu16(src) != 0)
        Sys_extWarn("read_tr5_room_layer: filler2 has wrong value");

    layer.bounding_box_x1 = read_float(src);
    layer.bounding_box_y1 = -read_float(src);
    layer.bounding_box_z1 = -read_float(src);
    layer.bounding_box_x2 = read_float(src);
    layer.bounding_box_y2 = -read_float(src);
    layer.bounding_box_z2 = -read_float(src);
    if (read_bitu32(src) != 0)
        Sys_extWarn("read_tr5_room_layer: filler3 has wrong value");

    layer.unknown_l6a = read_bit16(src);
    layer.unknown_l6b = read_bit16(src);
    layer.unknown_l7a = read_bit16(src);
    layer.unknown_l7b = read_bit16(src);
    layer.unknown_l8a = read_bit16(src);
    layer.unknown_l8b = read_bit16(src);
}

void TR_Level::read_tr5_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & vert)
{
    read_tr4_vertex_float(src, vert.vertex);
    read_tr4_vertex_float(src, vert.normal);
    vert.colour.b = read_bitu8(src) / 255.0f;
    vert.colour.g = read_bitu8(src) / 255.0f;
    vert.colour.r = read_bitu8(src) / 255.0f;
    vert.colour.a = read_bitu8(src) / 255.0f;
}

void TR_Level::read_tr5_room(SDL_RWops * const src, tr5_room_t & room)
{
    uint32_t room_data_size;
    //uint32_t portal_offset;
    uint32_t sector_data_offset;
    uint32_t static_meshes_offset;
    uint32_t layer_offset;
    uint32_t vertices_offset;
    uint32_t poly_offset;
    uint32_t poly_offset2;
    uint32_t vertices_size;
    //uint32_t light_size;

    SDL_RWops *newsrc = NULL;
    uint32_t temp;
    uint32_t i;
    uint8_t *buffer;

    if (read_bitu32(src) != 0x414C4558)
        Sys_extError("read_tr5_room: 'XELA' not found");

    room_data_size = read_bitu32(src);
    buffer = new uint8_t[room_data_size];

    if (SDL_RWread(src, buffer, 1, room_data_size) < room_data_size)
        Sys_extError("read_tr5_room: room_data");

    if ((newsrc = SDL_RWFromMem(buffer, room_data_size)) == NULL)
        Sys_extError("read_tr5_room: SDL_RWFromMem");

    room.intensity1 = 32767;
    room.intensity2 = 32767;
    room.light_mode = 0;

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator1 has wrong value");

    /*portal_offset = */read_bit32(newsrc);             // StartPortalOffset?   // endSDOffset
    sector_data_offset = read_bitu32(newsrc);    // StartSDOffset
    temp = read_bitu32(newsrc);
    if ((temp != 0) && (temp != 0xCDCDCDCD))
        Sys_extWarn("read_tr5_room: seperator2 has wrong value");

    static_meshes_offset = read_bitu32(newsrc);     // endPortalOffset
    // static_meshes_offset or room_layer_offset
    // read and change coordinate system
    room.offset.x = (float)read_bit32(newsrc);
    room.offset.y = read_bitu32(newsrc);
    room.offset.z = (float)-read_bit32(newsrc);
    room.y_bottom = (float)-read_bit32(newsrc);
    room.y_top = (float)-read_bit32(newsrc);

    room.num_zsectors = read_bitu16(newsrc);
    room.num_xsectors = read_bitu16(newsrc);

    room.light_colour.b = read_bitu8(newsrc) / 255.0f;
    room.light_colour.g = read_bitu8(newsrc) / 255.0f;
    room.light_colour.r = read_bitu8(newsrc) / 255.0f;
    room.light_colour.a = read_bitu8(newsrc) / 255.0f;

    room.num_lights = read_bitu16(newsrc);
    if (room.num_lights > 512)
        Sys_extWarn("read_tr5_room: num_lights > 512");

    room.num_static_meshes = read_bitu16(newsrc);
    if (room.num_static_meshes > 512)
        Sys_extWarn("read_tr5_room: num_static_meshes > 512");

    room.reverb_info = read_bitu8(newsrc);
    room.alternate_group = read_bitu8(newsrc);
    room.water_scheme = read_bitu16(newsrc);

    if (read_bitu32(newsrc) != 0x00007FFF)
        Sys_extWarn("read_tr5_room: filler1 has wrong value");

    if (read_bitu32(newsrc) != 0x00007FFF)
        Sys_extWarn("read_tr5_room: filler2 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator4 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator5 has wrong value");

    if (read_bitu32(newsrc) != 0xFFFFFFFF)
        Sys_extWarn("read_tr5_room: seperator6 has wrong value");

    room.alternate_room = read_bit16(newsrc);

    room.flags = read_bitu16(newsrc);

    room.unknown_r1 = read_bitu32(newsrc);
    room.unknown_r2 = read_bitu32(newsrc);
    room.unknown_r3 = read_bitu32(newsrc);

    temp = read_bitu32(newsrc);
    if ((temp != 0) && (temp != 0xCDCDCDCD))
        Sys_extWarn("read_tr5_room: seperator7 has wrong value");

    room.unknown_r4a = read_bitu16(newsrc);
    room.unknown_r4b = read_bitu16(newsrc);

    room.room_x = read_float(newsrc);
    room.unknown_r5 = read_bitu32(newsrc);
    room.room_z = -read_float(newsrc);

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator8 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator9 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator10 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator11 has wrong value");

    temp = read_bitu32(newsrc);
    if ((temp != 0) && (temp != 0xCDCDCDCD))
        Sys_extWarn("read_tr5_room: seperator12 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator13 has wrong value");

    room.num_triangles = read_bitu32(newsrc);
    if (room.num_triangles == 0xCDCDCDCD)
            room.num_triangles = 0;
    if (room.num_triangles > 512)
        Sys_extWarn("read_tr5_room: num_triangles > 512");

    room.num_rectangles = read_bitu32(newsrc);
    if (room.num_rectangles == 0xCDCDCDCD)
            room.num_rectangles = 0;
    if (room.num_rectangles > 1024)
        Sys_extWarn("read_tr5_room: num_rectangles > 1024");

    if (read_bitu32(newsrc) != 0)
        Sys_extWarn("read_tr5_room: seperator14 has wrong value");

    /*light_size = */read_bitu32(newsrc);
    if (read_bitu32(newsrc) != room.num_lights)
        Sys_extError("read_tr5_room: room.num_lights2 != room.num_lights");

    room.unknown_r6 = read_bitu32(newsrc);
    room.room_y_top = -read_float(newsrc);
    room.room_y_bottom = -read_float(newsrc);

    room.num_layers = read_bitu32(newsrc);

    /*
       if (room.num_layers != 0) {
       if (room.x != room.room_x)
       throw TR_ReadError("read_tr5_room: x != room_x");
       if (room.z != room.room_z)
       throw TR_ReadError("read_tr5_room: z != room_z");
       if (room.y_top != room.room_y_top)
       throw TR_ReadError("read_tr5_room: y_top != room_y_top");
       if (room.y_bottom != room.room_y_bottom)
       throw TR_ReadError("read_tr5_room: y_bottom != room_y_bottom");
       }
     */

    layer_offset = read_bitu32(newsrc);
    vertices_offset = read_bitu32(newsrc);
    poly_offset = read_bitu32(newsrc);
    poly_offset2 = read_bitu32(newsrc);
    if (poly_offset != poly_offset2)
        Sys_extError("read_tr5_room: poly_offset != poly_offset2");

    vertices_size = read_bitu32(newsrc);
    if ((vertices_size % 28) != 0)
        Sys_extError("read_tr5_room: vertices_size has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator15 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator16 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator17 has wrong value");

    if (read_bitu32(newsrc) != 0xCDCDCDCD)
        Sys_extWarn("read_tr5_room: seperator18 has wrong value");

    room.lights = (tr5_room_light_t*)malloc(room.num_lights * sizeof(tr5_room_light_t));
    for (i = 0; i < room.num_lights; i++)
        read_tr5_room_light(newsrc, room.lights[i]);

    SDL_RWseek(newsrc, 208 + sector_data_offset, SEEK_SET);

    room.sector_list = (tr_room_sector_t*)malloc(room.num_zsectors * room.num_xsectors * sizeof(tr_room_sector_t));
    for (i = 0; i < (uint32_t)(room.num_zsectors * room.num_xsectors); i++)
        read_tr_room_sector(newsrc, room.sector_list[i]);

    /*
       if (room.portal_offset != 0xFFFFFFFF) {
       if (room.portal_offset != (room.sector_data_offset + (room.num_zsectors * room.num_xsectors * 8)))
       throw TR_ReadError("read_tr5_room: portal_offset has wrong value");

       SDL_RWseek(newsrc, 208 + room.portal_offset, SEEK_SET);
       }
     */

    room.num_portals = read_bit16(newsrc);
    room.portals = (tr_room_portal_t*)malloc(room.num_portals * sizeof(tr_room_portal_t));
    for (i = 0; i < room.num_portals; i++)
        read_tr_room_portal(newsrc, room.portals[i]);

    SDL_RWseek(newsrc, 208 + static_meshes_offset, SEEK_SET);

    room.static_meshes = (tr2_room_staticmesh_t*)malloc(room.num_static_meshes * sizeof(tr2_room_staticmesh_t));
    for (i = 0; i < room.num_static_meshes; i++)
        read_tr4_room_staticmesh(newsrc, room.static_meshes[i]);

    SDL_RWseek(newsrc, 208 + layer_offset, SEEK_SET);

    room.layers = (tr5_room_layer_t*)malloc(room.num_layers * sizeof(tr5_room_layer_t));
    for (i = 0; i < room.num_layers; i++)
        read_tr5_room_layer(newsrc, room.layers[i]);

    SDL_RWseek(newsrc, 208 + poly_offset, SEEK_SET);

    {
        uint32_t vertex_index = 0;
        uint32_t rectangle_index = 0;
        uint32_t triangle_index = 0;

        room.rectangles = (tr4_face4_t*)malloc(room.num_rectangles * sizeof(tr4_face4_t));
        room.triangles = (tr4_face3_t*)malloc(room.num_triangles * sizeof(tr4_face3_t));
        for (i = 0; i < room.num_layers; i++) {
            uint32_t j;

            for (j = 0; j < room.layers[i].num_rectangles; j++) {
                read_tr4_face4(newsrc, room.rectangles[rectangle_index]);
                room.rectangles[rectangle_index].vertices[0] += vertex_index;
                room.rectangles[rectangle_index].vertices[1] += vertex_index;
                room.rectangles[rectangle_index].vertices[2] += vertex_index;
                room.rectangles[rectangle_index].vertices[3] += vertex_index;
                rectangle_index++;
            }
            for (j = 0; j < room.layers[i].num_triangles; j++) {
                read_tr4_face3(newsrc, room.triangles[triangle_index]);
                room.triangles[triangle_index].vertices[0] += vertex_index;
                room.triangles[triangle_index].vertices[1] += vertex_index;
                room.triangles[triangle_index].vertices[2] += vertex_index;
                triangle_index++;
            }
            vertex_index += room.layers[i].num_vertices;
        }
    }

    SDL_RWseek(newsrc, 208 + vertices_offset, SEEK_SET);

    {
        uint32_t vertex_index = 0;
        room.num_vertices = vertices_size / 28;
        //int temp1 = room_data_size - (208 + vertices_offset + vertices_size);
        room.vertices = (tr5_room_vertex_t*)calloc(room.num_vertices, sizeof(tr5_room_vertex_t));
        for (i = 0; i < room.num_layers; i++) {
            uint32_t j;

            for (j = 0; j < room.layers[i].num_vertices; j++)
                    read_tr5_room_vertex(newsrc, room.vertices[vertex_index++]);
        }
    }

    SDL_RWseek(newsrc, room_data_size, SEEK_SET);

    SDL_RWclose(newsrc);
    newsrc = NULL;
    delete [] buffer;
}

void TR_Level::read_tr5_moveable(SDL_RWops * const src, tr_moveable_t & moveable)
{
    read_tr_moveable(src, moveable);
    if (read_bitu16(src) != 0xFFEF)
        Sys_extWarn("read_tr5_moveable: filler has wrong value");
}

void TR_Level::read_tr5_level(SDL_RWops * const src)
{
    uint32_t i;
    uint8_t *comp_buffer = NULL;
    uint8_t *uncomp_buffer = NULL;
    SDL_RWops *newsrc = NULL;

    // Version
    uint32_t file_version = read_bitu32(src);

    if (file_version != 0x00345254)
        Sys_extError("Wrong level version");

    this->num_textiles = 0;
    this->num_room_textiles = 0;
    this->num_obj_textiles = 0;
    this->num_bump_textiles = 0;
    this->num_misc_textiles = 0;
    this->read_32bit_textiles = false;

    uint32_t uncomp_size;
    uint32_t comp_size;
    unsigned long size;

    this->num_room_textiles = read_bitu16(src);
    this->num_obj_textiles = read_bitu16(src);
    this->num_bump_textiles = read_bitu16(src);
    this->num_misc_textiles = 3;
    this->num_textiles = this->num_room_textiles + this->num_obj_textiles + this->num_bump_textiles + this->num_misc_textiles;

    uncomp_size = read_bitu32(src);
    if (uncomp_size == 0)
        Sys_extError("read_tr5_level: textiles32 uncomp_size == 0");

    comp_size = read_bitu32(src);
    if (comp_size > 0) {
        uncomp_buffer = new uint8_t[uncomp_size];

        this->textile32.resize( this->num_textiles );
        comp_buffer = new uint8_t[comp_size];

        if (SDL_RWread(src, comp_buffer, 1, comp_size) < comp_size)
            Sys_extError("read_tr5_level: textiles32");

        size = uncomp_size;
        if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
            Sys_extError("read_tr5_level: uncompress");

        if (size != uncomp_size)
            Sys_extError("read_tr5_level: uncompress size mismatch");
        delete [] comp_buffer;

        comp_buffer = NULL;
        if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
            Sys_extError("read_tr5_level: SDL_RWFromMem");

        for (i = 0; i < (this->num_textiles - this->num_misc_textiles); i++)
            read_tr4_textile32(newsrc, this->textile32[i]);
        SDL_RWclose(newsrc);
        newsrc = NULL;
        delete [] uncomp_buffer;

        uncomp_buffer = NULL;
        this->read_32bit_textiles = true;
    }

    uncomp_size = read_bitu32(src);
    if (uncomp_size == 0)
        Sys_extError("read_tr5_level: textiles16 uncomp_size == 0");

    comp_size = read_bitu32(src);
    if (comp_size > 0) {
        if (this->textile32.empty()) {
            uncomp_buffer = new uint8_t[uncomp_size];

            this->textile16_count = this->num_textiles;
            this->textile16 = (tr2_textile16_t*)malloc(this->textile16_count * sizeof(tr2_textile16_t));
            comp_buffer = new uint8_t[comp_size];

            if (SDL_RWread(src, comp_buffer, 1, comp_size) < comp_size)
                Sys_extError("read_tr5_level: textiles16");

            size = uncomp_size;
            if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
                Sys_extError("read_tr5_level: uncompress");

            if (size != uncomp_size)
                Sys_extError("read_tr5_level: uncompress size mismatch");
            delete [] comp_buffer;

            comp_buffer = NULL;
            if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
                Sys_extError("read_tr5_level: SDL_RWFromMem");

            for (i = 0; i < (this->num_textiles - this->num_misc_textiles); i++)
                read_tr2_textile16(newsrc, this->textile16[i]);
            SDL_RWclose(newsrc);
            newsrc = NULL;
            delete [] uncomp_buffer;

            uncomp_buffer = NULL;
        } else {
            SDL_RWseek(src, comp_size, SEEK_CUR);
        }
    }

    uncomp_size = read_bitu32(src);
    if (uncomp_size == 0)
        Sys_extError("read_tr5_level: textiles32d uncomp_size == 0");

    comp_size = read_bitu32(src);
    if (comp_size > 0) {
        uncomp_buffer = new uint8_t[uncomp_size];

        if ((uncomp_size / (256 * 256 * 4)) > 3)
            Sys_extWarn("read_tr5_level: num_misc_textiles > 3");

        if (this->textile32.empty())
        {
            this->textile32.resize( this->num_misc_textiles );
        }

        comp_buffer = new uint8_t[comp_size];

        if (SDL_RWread(src, comp_buffer, 1, comp_size) < comp_size)
            Sys_extError("read_tr5_level: misc_textiles");

        size = uncomp_size;
        if (uncompress(uncomp_buffer, &size, comp_buffer, comp_size) != Z_OK)
            Sys_extError("read_tr5_level: uncompress");

        if (size != uncomp_size)
            Sys_extError("read_tr5_level: uncompress size mismatch");
        delete [] comp_buffer;

        comp_buffer = NULL;
        if ((newsrc = SDL_RWFromMem(uncomp_buffer, uncomp_size)) == NULL)
            Sys_extError("read_tr5_level: SDL_RWFromMem");

        for (i = (this->num_textiles - this->num_misc_textiles); i < this->num_textiles; i++)
            read_tr4_textile32(newsrc, this->textile32[i]);
        SDL_RWclose(newsrc);
        newsrc = NULL;
        delete [] uncomp_buffer;

        uncomp_buffer = NULL;
    }

    // flags?
    /*
       I found 2 flags in the TR5 file format. Directly after the sprite textures are 2 ints as a flag. The first one is the lara type:
       0 Normal
       3 Catsuit
       4 Divesuit
       6 Invisible

       The second one is the weather type (effects all outside rooms):
       0 No weather
       1 Rain
       2 Snow (in title.trc these are red triangles falling from the sky).
     */
    i = read_bitu16(src);
    i = read_bitu16(src);
    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for flags[1]");

    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for flags[2]");

    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for flags[3]");

    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for flags[4]");

    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for flags[5]");

    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for flags[6]");

    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for flags[7]");

    // LevelDataSize1
    read_bitu32(src);
    // LevelDataSize2
    read_bitu32(src);

    // Unused
    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for 'unused'");

    this->rooms.resize( read_bitu32(src) );
    for (i = 0; i < this->rooms.size(); i++)
        read_tr5_room(src, this->rooms[i]);

    this->floor_data.resize( read_bitu32(src) );
    for(i = 0; i < this->floor_data.size(); i++)
        this->floor_data[i] = read_bitu16(src);

    read_mesh_data(src);

    this->animations_count = read_bitu32(src);
    this->animations = (tr_animation_t*)malloc(this->animations_count * sizeof(tr_animation_t));
    for (i = 0; i < this->animations_count; i++)
    {
        read_tr4_animation(src, this->animations[i]);
    }

    this->state_changes_count = read_bitu32(src);
    this->state_changes = (tr_state_change_t*)malloc(this->state_changes_count * sizeof(tr_state_change_t));
    for (i = 0; i < this->state_changes_count; i++)
        read_tr_state_changes(src, this->state_changes[i]);

    this->anim_dispatches_count = read_bitu32(src);
    this->anim_dispatches = (tr_anim_dispatch_t*)malloc(this->anim_dispatches_count * sizeof(tr_anim_dispatch_t));
    for (i = 0; i < this->anim_dispatches_count; i++)
        read_tr_anim_dispatches(src, this->anim_dispatches[i]);

    this->anim_commands_count = read_bitu32(src);
    this->anim_commands = (int16_t*)malloc(this->anim_commands_count * sizeof(int16_t));
    for (i = 0; i < this->anim_commands_count; i++)
        this->anim_commands[i] = read_bit16(src);

    this->mesh_tree_data_size = read_bitu32(src);
    this->mesh_tree_data = (uint32_t*)malloc(this->mesh_tree_data_size * sizeof(uint32_t));
    for (i = 0; i < this->mesh_tree_data_size; i++)
        this->mesh_tree_data[i] = read_bitu32(src);                     // 4 bytes

    read_frame_moveable_data(src);

    this->static_meshes_count = read_bitu32(src);
    this->static_meshes = (tr_staticmesh_t*)malloc(this->static_meshes_count * sizeof(tr_staticmesh_t));
    for (i = 0; i < this->static_meshes_count; i++)
        read_tr_staticmesh(src, this->static_meshes[i]);

    if (read_bit8(src) != 'S')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if (read_bit8(src) != 'P')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if (read_bit8(src) != 'R')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if (read_bit8(src) != 0)
        Sys_extError("read_tr5_level: 'SPR' not found");

    this->sprite_textures.resize( read_bitu32(src) );
    for (i = 0; i < this->sprite_textures.size(); i++)
        read_tr4_sprite_texture(src, this->sprite_textures[i]);

    this->sprite_sequences_count = read_bitu32(src);
    this->sprite_sequences = (tr_sprite_sequence_t*)malloc(this->sprite_sequences_count * sizeof(tr_sprite_sequence_t));
    for (i = 0; i < this->sprite_sequences_count; i++)
        read_tr_sprite_sequence(src, this->sprite_sequences[i]);

    this->cameras_count = read_bitu32(src);
    this->cameras = (tr_camera_t*)malloc(this->cameras_count * sizeof(tr_camera_t));
    for (i = 0; i < this->cameras_count; i++)
    {
        this->cameras[i].x = read_bit32(src);
        this->cameras[i].y = read_bit32(src);
        this->cameras[i].z = read_bit32(src);

        this->cameras[i].room = read_bit16(src);
        this->cameras[i].unknown1 = read_bitu16(src);
    }

    this->flyby_cameras_count = read_bitu32(src);
    this->flyby_cameras = (tr4_flyby_camera_t*)malloc(this->flyby_cameras_count * sizeof(tr4_flyby_camera_t));
    for (i = 0; i < this->flyby_cameras_count; i++)
    {
        this->flyby_cameras[i].cam_x = read_bit32(src);
        this->flyby_cameras[i].cam_y = read_bit32(src);
        this->flyby_cameras[i].cam_z = read_bit32(src);
        this->flyby_cameras[i].target_x = read_bit32(src);
        this->flyby_cameras[i].target_y = read_bit32(src);
        this->flyby_cameras[i].target_z = read_bit32(src);

        this->flyby_cameras[i].sequence = read_bit8(src);
        this->flyby_cameras[i].index    = read_bit8(src);

        this->flyby_cameras[i].fov   = read_bitu16(src);
        this->flyby_cameras[i].roll  = read_bitu16(src);
        this->flyby_cameras[i].timer = read_bitu16(src);
        this->flyby_cameras[i].speed = read_bitu16(src);
        this->flyby_cameras[i].flags = read_bitu16(src);

        this->flyby_cameras[i].room_id = read_bitu32(src);
    }

    this->sound_sources_count = read_bitu32(src);
    this->sound_sources = (tr_sound_source_t*)malloc(this->sound_sources_count * sizeof(tr_sound_source_t));
    for(i = 0; i < this->sound_sources_count; i++)
    {
        this->sound_sources[i].x = read_bit32(src);
        this->sound_sources[i].y = read_bit32(src);
        this->sound_sources[i].z = read_bit32(src);

        this->sound_sources[i].sound_id = read_bitu16(src);
        this->sound_sources[i].flags = read_bitu16(src);
    }

    this->boxes_count = read_bitu32(src);
    this->boxes = (tr_box_t*)malloc(this->boxes_count * sizeof(tr_box_t));
    for (i = 0; i < this->boxes_count; i++)
        read_tr2_box(src, this->boxes[i]);

    this->overlaps_count = read_bitu32(src);
    this->overlaps = (uint16_t*)malloc(this->overlaps_count * sizeof(uint16_t));
    for (i = 0; i < this->overlaps_count; i++)
        this->overlaps[i] = read_bitu16(src);

    // Zones
    SDL_RWseek(src, this->boxes_count * 20, SEEK_CUR);

    this->animated_textures_count = read_bitu32(src);
    this->animated_textures = (uint16_t*)malloc(this->animated_textures_count * sizeof(uint16_t));
    for (i = 0; i < this->animated_textures_count; i++)
    {
        this->animated_textures[i] = read_bitu16(src);
    }

    this->animated_textures_uv_count = read_bitu8(src);

    if (read_bit8(src) != 'T')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if (read_bit8(src) != 'E')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if (read_bit8(src) != 'X')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if (read_bit8(src) != 0)
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    this->object_textures.resize( read_bitu32(src) );
    for (i = 0; i < this->object_textures.size(); i++)
    {
        read_tr4_object_texture(src, this->object_textures[i]);
        if (read_bitu16(src) != 0)
            Sys_extWarn("read_tr5_level: obj_tex trailing bitu16 != 0");
    }

    this->items_count = read_bitu32(src);
    this->items = (tr2_item_t*)malloc(this->items_count * sizeof(tr2_item_t));
    for (i = 0; i < this->items_count; i++)
        read_tr4_item(src, this->items[i]);

    this->ai_objects_count = read_bitu32(src);
    this->ai_objects = (tr4_ai_object_t*)malloc(this->ai_objects_count * sizeof(tr4_ai_object_t));
    for(i=0; i < this->ai_objects_count; i++)
    {
        this->ai_objects[i].object_id = read_bitu16(src);
        this->ai_objects[i].room = read_bitu16(src);

        this->ai_objects[i].x = read_bit32(src);
        this->ai_objects[i].y = read_bit32(src);
        this->ai_objects[i].z = read_bit32(src);                            // 16

        this->ai_objects[i].ocb = read_bitu16(src);
        this->ai_objects[i].flags = read_bitu16(src);                       // 20
        this->ai_objects[i].angle = read_bit32(src);                        // 24
    }

    this->demo_data_count = read_bitu16(src);
    this->demo_data = (uint8_t*)malloc(this->demo_data_count * sizeof(uint8_t));
    for(i=0; i < this->demo_data_count; i++)
        this->demo_data[i] = read_bitu8(src);

    // Soundmap
    this->soundmap = (int16_t*)malloc(TR_AUDIO_MAP_SIZE_TR5 * sizeof(int16_t));
    for(i=0; i < TR_AUDIO_MAP_SIZE_TR5; i++)
        this->soundmap[i] = read_bit16(src);

    this->sound_details_count = read_bitu32(src);
    this->sound_details = (tr_sound_details_t*)malloc(this->sound_details_count * sizeof(tr_sound_details_t));

    for(i=0; i < this->sound_details_count; i++)
    {
        this->sound_details[i].sample = read_bitu16(src);
        this->sound_details[i].volume = (uint16_t)read_bitu8(src);        // n x 2.6
        this->sound_details[i].sound_range = (uint16_t)read_bitu8(src);   // n as is
        this->sound_details[i].chance = (uint16_t)read_bitu8(src);        // If n = 99, n = 0 (max. chance)
        this->sound_details[i].pitch = (int16_t)read_bit8(src);           // n as is
        this->sound_details[i].num_samples_and_flags_1 = read_bitu8(src);
        this->sound_details[i].flags_2 = read_bitu8(src);
    }

    this->sample_indices_count = read_bitu32(src);
    this->sample_indices = (uint32_t*)malloc(this->sample_indices_count * sizeof(uint32_t));
    for(i=0; i < this->sample_indices_count; i++)
        this->sample_indices[i] = read_bitu32(src);

    SDL_RWseek(src, 6, SEEK_CUR);   // In TR5, sample indices are followed by 6 0xCD bytes. - correct - really 0xCDCDCDCDCDCD

    // LOAD SAMPLES
    i = read_bitu32(src);                                                       // Read num samples
    if(i)
    {
        this->samples_count = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        this->samples_data.resize( SDL_RWsize(src) - SDL_RWtell(src) );
        for(i = 0; i < this->samples_data.size(); i++)
            this->samples_data[i] = read_bitu8(src);
    }
}
