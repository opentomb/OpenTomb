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
#include "l_main.h"
#include "../system.h"
#include "../audio.h"

#define RCSID "$Id: l_tr3.cpp,v 1.15 2002/09/20 15:59:02 crow Exp $"

void TR_Level::read_tr3_room_light(SDL_RWops * const src, tr5_room_light_t & light)
{
    read_tr_vertex32(src, light.pos);
    light.color.r = read_bitu8(src);
    light.color.g = read_bitu8(src);
    light.color.b = read_bitu8(src);
    light.color.a = read_bitu8(src);
    light.fade1 = read_bitu32(src);
    light.fade2 = read_bitu32(src);

    light.intensity = 1.0f;

    light.r_outer = (float)light.fade1;
    light.r_inner = (float)light.fade1 / 2.0;

    light.light_type = 0x01; // Point light
}

void TR_Level::read_tr3_room_vertex(SDL_RWops *const src, tr5_room_vertex_t & room_vertex)
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

    room_vertex.colour.r = ((room_vertex.lighting2 & 0x7C00) >> 10) / 62.0f;
    room_vertex.colour.g = ((room_vertex.lighting2 & 0x03E0) >> 5) / 62.0f;
    room_vertex.colour.b = ((room_vertex.lighting2 & 0x001F)) / 62.0f;
    room_vertex.colour.a = 1.0f;
}

void TR_Level::read_tr3_room_staticmesh(SDL_RWops *const src, tr2_room_staticmesh_t & room_static_mesh)
{
    read_tr_vertex32(src, room_static_mesh.pos);
    room_static_mesh.rotation = (float)read_bitu16(src) / 16384.0f * -90;
    room_static_mesh.intensity1 = read_bit16(src);
    room_static_mesh.intensity2 = read_bit16(src);
    room_static_mesh.object_id = read_bitu16(src);

    room_static_mesh.tint.r = ((room_static_mesh.intensity1 & 0x001F)) / 62.0f;

    room_static_mesh.tint.g = ((room_static_mesh.intensity1 & 0x03E0) >> 5) / 62.0f;

    room_static_mesh.tint.b = ((room_static_mesh.intensity1 & 0x7C00) >> 10) / 62.0f;
    room_static_mesh.tint.a = 1.0f;
}

void TR_Level::read_tr3_room(SDL_RWops * const src, tr5_room_t & room)
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

    pos = SDL_RWseek(src, 0, RW_SEEK_CUR);

    room.num_layers = 0;

    room.num_vertices = read_bitu16(src);
    room.vertices = (tr5_room_vertex_t*)calloc(room.num_vertices, sizeof(tr5_room_vertex_t));
    for(i = 0; i < room.num_vertices; i++)
        read_tr3_room_vertex(src, room.vertices[i]);

    room.num_rectangles = read_bitu16(src);
    room.rectangles = (tr4_face4_t*)malloc(room.num_rectangles * sizeof(tr4_face4_t));
    for(i = 0; i < room.num_rectangles; i++)
        read_tr_face4(src, room.rectangles[i]);

    room.num_triangles = read_bitu16(src);
    room.triangles = (tr4_face3_t*)malloc(room.num_triangles * sizeof(tr4_face3_t));
    for(i = 0; i < room.num_triangles; i++)
        read_tr_face3(src, room.triangles[i]);

    room.num_sprites = read_bitu16(src);
    room.sprites = (tr_room_Sprite*)malloc(room.num_sprites * sizeof(tr_room_Sprite));
    for(i = 0; i < room.num_sprites; i++)
        read_tr_room_sprite(src, room.sprites[i]);

    // set to the right position in case that there is some unused data
    SDL_RWseek(src, pos + (num_data_words * 2), RW_SEEK_SET);

    room.num_portals = read_bitu16(src);
    room.portals = (tr_room_portal_t*)malloc(room.num_portals * sizeof(tr_room_portal_t));
    for(i = 0; i < room.num_portals; i++)
        read_tr_room_portal(src, room.portals[i]);

    room.num_zsectors = read_bitu16(src);
    room.num_xsectors = read_bitu16(src);
    room.sector_list = (tr_room_sector_t*)malloc(room.num_zsectors * room.num_xsectors * sizeof(tr_room_sector_t));
    for(i = 0; i < (uint32_t)(room.num_zsectors * room.num_xsectors); i++)
        read_tr_room_sector(src, room.sector_list[i]);

    room.intensity1 = read_bit16(src);
    room.intensity2 = read_bit16(src);

    // only in TR2
    room.light_mode = 0;

    room.num_lights = read_bitu16(src);
    room.lights = (tr5_room_light_t*)malloc(room.num_lights * sizeof(tr5_room_light_t));
    for(i = 0; i < room.num_lights; i++)
        read_tr3_room_light(src, room.lights[i]);

    room.num_static_meshes = read_bitu16(src);
    room.static_meshes = (tr2_room_staticmesh_t*)malloc(room.num_static_meshes * sizeof(tr2_room_staticmesh_t));
    for(i = 0; i < room.num_static_meshes; i++)
        read_tr3_room_staticmesh(src, room.static_meshes[i]);

    room.alternate_room = read_bit16(src);
    room.alternate_group = 0;   // Doesn't exist in TR1-3

    room.flags = read_bitu16(src);

    if(room.flags & 0x0080)
    {
        room.flags |= 0x0002;   // Move quicksand flag to another bit to avoid confusion with NL flag.
        room.flags ^= 0x0080;
    }

    // Only in TR3-5

    room.water_scheme = read_bitu8(src);
    room.reverb_info = read_bitu8(src);

    SDL_RWseek(src, 1, SEEK_CUR);   // Alternate_group override?

    room.light_colour.r = room.intensity1 / 65534.0f;
    room.light_colour.g = room.intensity1 / 65534.0f;
    room.light_colour.b = room.intensity1 / 65534.0f;
    room.light_colour.a = 1.0f;
}

void TR_Level::read_tr3_item(SDL_RWops *const src, tr2_item_t & item)
{
    item.object_id = read_bit16(src);
    item.room = read_bit16(src);
    read_tr_vertex32(src, item.pos);
    item.rotation = (float)read_bitu16(src) / 16384.0f * -90;
    item.intensity1 = read_bitu16(src);
    item.intensity2 = read_bitu16(src);
    item.ocb = 0;   // Not present in TR3!
    item.flags = read_bitu16(src);
}

void TR_Level::read_tr3_level(SDL_RWops *const src)
{
    uint32_t i;

    // Version
    uint32_t file_version = read_bitu32(src);

    if((file_version != 0xFF080038) && (file_version != 0xFF180038) && (file_version != 0xFF180034))
        Sys_extError("Wrong level version");

    read_tr_palette(src, this->palette);
    read_tr2_palette16(src, this->palette16);

    this->num_textiles = 0;
    this->num_room_textiles = 0;
    this->num_obj_textiles = 0;
    this->num_bump_textiles = 0;
    this->num_misc_textiles = 0;
    this->read_32bit_textiles = false;

    this->textile8_count = this->num_textiles = read_bitu32(src);
    this->textile8 = (tr_textile8_t*)malloc(this->textile8_count * sizeof(tr_textile8_t));
    for(i = 0; i < this->textile8_count; i++)
        read_tr_textile8(src, this->textile8[i]);
    this->textile16_count = this->textile8_count;
    this->textile16 = (tr2_textile16_t*)malloc(this->textile16_count * sizeof(tr2_textile16_t));
    for(i = 0; i < this->textile16_count; i++)
        read_tr2_textile16(src, this->textile16[i]);

    if(file_version == 0xFF180034)                                          // VICT.TR2
    {
        return;                                                             // Here only palette and textiles
    }

    // Unused
    if(read_bitu32(src) != 0)
        Sys_extWarn("Bad value for 'unused'");

    this->rooms.resize(read_bitu16(src));
    for(i = 0; i < this->rooms.size(); i++)
        read_tr3_room(src, this->rooms[i]);

    this->floor_data.resize( read_bitu32(src) );
    for(i = 0; i < this->floor_data.size(); i++)
        this->floor_data[i] = read_bitu16(src);

    read_mesh_data(src);

    this->animations.resize( read_bitu32(src) );
    for(i = 0; i < this->animations.size(); i++)
        read_tr_animation(src, this->animations[i]);

    this->state_changes.resize( read_bitu32(src) );
    for(i = 0; i < this->state_changes.size(); i++)
        read_tr_state_changes(src, this->state_changes[i]);

    this->anim_dispatches.resize( read_bitu32(src) );
    for(i = 0; i < this->anim_dispatches.size(); i++)
        read_tr_anim_dispatches(src, this->anim_dispatches[i]);

    this->anim_commands.resize( read_bitu32(src) );
    for(i = 0; i < this->anim_commands.size(); i++)
        this->anim_commands[i] = read_bit16(src);

    this->mesh_tree_data_size = read_bitu32(src);
    this->mesh_tree_data = (uint32_t*)malloc(this->mesh_tree_data_size * sizeof(uint32_t));
    for(i = 0; i < this->mesh_tree_data_size; i++)
        this->mesh_tree_data[i] = read_bitu32(src);                     // 4 bytes

    read_frame_moveable_data(src);

    this->static_meshes.resize( read_bitu32(src) );
    for(i = 0; i < this->static_meshes.size(); i++)
        read_tr_staticmesh(src, this->static_meshes[i]);

    this->sprite_textures.resize(read_bitu32(src));
    for(i = 0; i < this->sprite_textures.size(); i++)
        read_tr_sprite_texture(src, this->sprite_textures[i]);

    this->sprite_sequences.resize( read_bitu32(src) );
    for(i = 0; i < this->sprite_sequences.size(); i++)
        read_tr_sprite_sequence(src, this->sprite_sequences[i]);

    this->cameras.resize( read_bitu32(src) );
    for(i = 0; i < this->cameras.size(); i++)
    {
        this->cameras[i].x = read_bit32(src);
        this->cameras[i].y = read_bit32(src);
        this->cameras[i].z = read_bit32(src);

        this->cameras[i].room = read_bit16(src);
        this->cameras[i].unknown1 = read_bitu16(src);
    }

    this->sound_sources.resize( read_bitu32(src) );
    for(i = 0; i < this->sound_sources.size(); i++)
    {
        this->sound_sources[i].x = read_bit32(src);
        this->sound_sources[i].y = read_bit32(src);
        this->sound_sources[i].z = read_bit32(src);

        this->sound_sources[i].sound_id = read_bitu16(src);
        this->sound_sources[i].flags = read_bitu16(src);
    }

    this->boxes.resize( read_bitu32(src) );
    for(i = 0; i < this->boxes.size(); i++)
        read_tr2_box(src, this->boxes[i]);

    this->overlaps.resize(read_bitu32(src));
    for(i = 0; i < this->overlaps.size(); i++)
        this->overlaps[i] = read_bitu16(src);

    // Zones
    SDL_RWseek(src, this->boxes.size() * 20, RW_SEEK_CUR);

    this->animated_textures.resize( read_bitu32(src) );
    this->animated_textures_uv_count = 0; // No UVRotate in TR3
    for(i = 0; i < this->animated_textures.size(); i++)
    {
        this->animated_textures[i] = read_bitu16(src);
    }

    this->object_textures.resize(read_bitu32(src));
    for(i = 0; i < this->object_textures.size(); i++)
        read_tr_object_texture(src, this->object_textures[i]);

    this->items.resize(read_bitu32(src));
    for(i = 0; i < this->items.size(); i++)
        read_tr3_item(src, this->items[i]);

    read_tr_lightmap(src, this->lightmap);

    this->cinematic_frames.resize( read_bitu16(src) );
    for(i = 0; i < this->cinematic_frames.size(); i++)
    {
        read_tr_cinematic_frame(src, this->cinematic_frames[i]);
    }

    this->demo_data.resize( read_bitu16(src) );
    for(i = 0; i < this->demo_data.size(); i++)
        this->demo_data[i] = read_bitu8(src);

    // Soundmap
    this->soundmap.resize(TR_AUDIO_MAP_SIZE_TR3);
    for(i = 0; i < this->soundmap.size(); i++)
        this->soundmap[i] = read_bit16(src);

    this->sound_details.resize( read_bitu32(src) );
    for(i = 0; i < this->sound_details.size(); i++)
    {
        this->sound_details[i].sample = read_bitu16(src);
        this->sound_details[i].volume = (uint16_t)read_bitu8(src);
        this->sound_details[i].sound_range = (uint16_t)read_bitu8(src);
        this->sound_details[i].chance = (uint16_t)read_bit8(src);
        this->sound_details[i].pitch = (int16_t)read_bit8(src);
        this->sound_details[i].num_samples_and_flags_1 = read_bitu8(src);
        this->sound_details[i].flags_2 = read_bitu8(src);
    }

    this->sample_indices.resize( read_bitu32(src) );
    for(i = 0; i < this->sample_indices.size(); i++)
        this->sample_indices[i] = read_bitu32(src);

    // remap all sample indices here
    for(i = 0; i < this->sound_details.size(); i++)
    {
        if(this->sound_details[i].sample < this->sample_indices.size())
        {
            this->sound_details[i].sample = this->sample_indices[this->sound_details[i].sample];
        }
    }

    // LOAD SAMPLES

    // In TR3, samples are stored in separate file called MAIN.SFX.
    // If there is no such files, no samples are loaded.

    SDL_RWops *newsrc = SDL_RWFromFile(this->sfx_path, "rb");
    if(newsrc == NULL)
    {
        Sys_extWarn("read_tr2_level: failed to open \"%s\"! No samples loaded.", this->sfx_path);
    }
    else
    {
        this->samples_data.resize(SDL_RWsize(newsrc));
        this->samples_count = 0;
        for(i = 0; i < this->samples_data.size(); i++)
        {
            this->samples_data[i] = read_bitu8(newsrc);
            if((i >= 4) && (*((uint32_t*)(this->samples_data.data() + i - 4)) == 0x46464952))   /// RIFF
            {
                this->samples_count++;
            }
        }

        SDL_RWclose(newsrc);
        newsrc = NULL;
    }
}