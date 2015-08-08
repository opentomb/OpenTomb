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

    room.vertices.resize( read_bitu16(src) );
    for(i = 0; i < room.vertices.size(); i++)
        read_tr3_room_vertex(src, room.vertices[i]);

    room.rectangles.resize( read_bitu16(src) );
    for(i = 0; i < room.rectangles.size(); i++)
        read_tr_face4(src, room.rectangles[i]);

    room.triangles.resize( read_bitu16(src) );
    for(i = 0; i < room.triangles.size(); i++)
        read_tr_face3(src, room.triangles[i]);

    room.sprites.resize( read_bitu16(src) );
    for(i = 0; i < room.sprites.size(); i++)
        read_tr_room_sprite(src, room.sprites[i]);

    // set to the right position in case that there is some unused data
    SDL_RWseek(src, pos + (num_data_words * 2), RW_SEEK_SET);

    room.portals.resize( read_bitu16(src) );
    for(i = 0; i < room.portals.size(); i++)
        read_tr_room_portal(src, room.portals[i]);

    room.num_zsectors = read_bitu16(src);
    room.num_xsectors = read_bitu16(src);
    room.sector_list.resize(room.num_zsectors * room.num_xsectors);
    for(i = 0; i < (uint32_t)(room.num_zsectors * room.num_xsectors); i++)
        read_tr_room_sector(src, room.sector_list[i]);

    room.intensity1 = read_bit16(src);
    room.intensity2 = read_bit16(src);

    // only in TR2
    room.light_mode = 0;

    room.lights.resize( read_bitu16(src) );
    for(i = 0; i < room.lights.size(); i++)
        read_tr3_room_light(src, room.lights[i]);

    room.static_meshes.resize( read_bitu16(src) );
    for(i = 0; i < room.static_meshes.size(); i++)
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

    read_tr_palette(src, this->m_palette);
    read_tr2_palette16(src, this->m_palette16);

    this->m_numTextiles = 0;
    this->m_numRoomTextiles = 0;
    this->m_numObjTextiles = 0;
    this->m_numBumpTextiles = 0;
    this->m_numMiscTextiles = 0;
    this->m_read32BitTextiles = false;

    this->m_numTextiles = read_bitu32(src);
    this->m_textile8.resize(this->m_numTextiles);
    for(i = 0; i < this->m_textile8.size(); i++)
        read_tr_textile8(src, this->m_textile8[i]);
    this->m_textile16.resize( this->m_textile8.size() );
    for(i = 0; i < this->m_textile16.size(); i++)
        read_tr2_textile16(src, this->m_textile16[i]);

    if(file_version == 0xFF180034)                                          // VICT.TR2
    {
        return;                                                             // Here only palette and textiles
    }

    // Unused
    if(read_bitu32(src) != 0)
        Sys_extWarn("Bad value for 'unused'");

    this->m_rooms.resize(read_bitu16(src));
    for(i = 0; i < this->m_rooms.size(); i++)
        read_tr3_room(src, this->m_rooms[i]);

    this->m_floorData.resize( read_bitu32(src) );
    for(i = 0; i < this->m_floorData.size(); i++)
        this->m_floorData[i] = read_bitu16(src);

    read_mesh_data(src);

    this->m_animations.resize( read_bitu32(src) );
    for(i = 0; i < this->m_animations.size(); i++)
        read_tr_animation(src, this->m_animations[i]);

    this->m_stateChanges.resize( read_bitu32(src) );
    for(i = 0; i < this->m_stateChanges.size(); i++)
        read_tr_state_changes(src, this->m_stateChanges[i]);

    this->m_animDispatches.resize( read_bitu32(src) );
    for(i = 0; i < this->m_animDispatches.size(); i++)
        read_tr_anim_dispatches(src, this->m_animDispatches[i]);

    this->m_animCommands.resize( read_bitu32(src) );
    for(i = 0; i < this->m_animCommands.size(); i++)
        this->m_animCommands[i] = read_bit16(src);

    this->m_meshTreeData.resize( read_bitu32(src) );
    for(i = 0; i < this->m_meshTreeData.size(); i++)
        this->m_meshTreeData[i] = read_bitu32(src);                     // 4 bytes

    read_frame_moveable_data(src);

    this->m_staticMeshes.resize( read_bitu32(src) );
    for(i = 0; i < this->m_staticMeshes.size(); i++)
        read_tr_staticmesh(src, this->m_staticMeshes[i]);

    this->m_spriteTextures.resize(read_bitu32(src));
    for(i = 0; i < this->m_spriteTextures.size(); i++)
        read_tr_sprite_texture(src, this->m_spriteTextures[i]);

    this->m_spriteSequences.resize( read_bitu32(src) );
    for(i = 0; i < this->m_spriteSequences.size(); i++)
        read_tr_sprite_sequence(src, this->m_spriteSequences[i]);

    this->m_cameras.resize( read_bitu32(src) );
    for(i = 0; i < this->m_cameras.size(); i++)
    {
        this->m_cameras[i].x = read_bit32(src);
        this->m_cameras[i].y = read_bit32(src);
        this->m_cameras[i].z = read_bit32(src);

        this->m_cameras[i].room = read_bit16(src);
        this->m_cameras[i].unknown1 = read_bitu16(src);
    }

    this->m_soundSources.resize( read_bitu32(src) );
    for(i = 0; i < this->m_soundSources.size(); i++)
    {
        this->m_soundSources[i].x = read_bit32(src);
        this->m_soundSources[i].y = read_bit32(src);
        this->m_soundSources[i].z = read_bit32(src);

        this->m_soundSources[i].sound_id = read_bitu16(src);
        this->m_soundSources[i].flags = read_bitu16(src);
    }

    this->m_boxes.resize( read_bitu32(src) );
    for(i = 0; i < this->m_boxes.size(); i++)
        read_tr2_box(src, this->m_boxes[i]);

    this->m_overlaps.resize(read_bitu32(src));
    for(i = 0; i < this->m_overlaps.size(); i++)
        this->m_overlaps[i] = read_bitu16(src);

    // Zones
    SDL_RWseek(src, this->m_boxes.size() * 20, RW_SEEK_CUR);

    this->m_animatedTextures.resize( read_bitu32(src) );
    this->m_animatedTexturesUvCount = 0; // No UVRotate in TR3
    for(i = 0; i < this->m_animatedTextures.size(); i++)
    {
        this->m_animatedTextures[i] = read_bitu16(src);
    }

    this->m_objectTextures.resize(read_bitu32(src));
    for(i = 0; i < this->m_objectTextures.size(); i++)
        read_tr_object_texture(src, this->m_objectTextures[i]);

    this->m_items.resize(read_bitu32(src));
    for(i = 0; i < this->m_items.size(); i++)
        read_tr3_item(src, this->m_items[i]);

    read_tr_lightmap(src, this->m_lightmap);

    this->m_cinematicFrames.resize( read_bitu16(src) );
    for(i = 0; i < this->m_cinematicFrames.size(); i++)
    {
        read_tr_cinematic_frame(src, this->m_cinematicFrames[i]);
    }

    this->m_demoData.resize( read_bitu16(src) );
    for(i = 0; i < this->m_demoData.size(); i++)
        this->m_demoData[i] = read_bitu8(src);

    // Soundmap
    this->m_soundmap.resize(TR_AUDIO_MAP_SIZE_TR3);
    for(i = 0; i < this->m_soundmap.size(); i++)
        this->m_soundmap[i] = read_bit16(src);

    this->m_soundDetails.resize( read_bitu32(src) );
    for(i = 0; i < this->m_soundDetails.size(); i++)
    {
        this->m_soundDetails[i].sample = read_bitu16(src);
        this->m_soundDetails[i].volume = (uint16_t)read_bitu8(src);
        this->m_soundDetails[i].sound_range = (uint16_t)read_bitu8(src);
        this->m_soundDetails[i].chance = (uint16_t)read_bit8(src);
        this->m_soundDetails[i].pitch = (int16_t)read_bit8(src);
        this->m_soundDetails[i].num_samples_and_flags_1 = read_bitu8(src);
        this->m_soundDetails[i].flags_2 = read_bitu8(src);
    }

    this->m_sampleIndices.resize( read_bitu32(src) );
    for(i = 0; i < this->m_sampleIndices.size(); i++)
        this->m_sampleIndices[i] = read_bitu32(src);

    // remap all sample indices here
    for(i = 0; i < this->m_soundDetails.size(); i++)
    {
        if(this->m_soundDetails[i].sample < this->m_sampleIndices.size())
        {
            this->m_soundDetails[i].sample = this->m_sampleIndices[this->m_soundDetails[i].sample];
        }
    }

    // LOAD SAMPLES

    // In TR3, samples are stored in separate file called MAIN.SFX.
    // If there is no such files, no samples are loaded.

    SDL_RWops *newsrc = SDL_RWFromFile(this->m_sfxPath.c_str(), "rb");
    if(newsrc == NULL)
    {
        Sys_extWarn("read_tr2_level: failed to open \"%s\"! No samples loaded.", this->m_sfxPath.c_str());
    }
    else
    {
        this->m_samplesData.resize(SDL_RWsize(newsrc));
        this->m_samplesCount = 0;
        for(i = 0; i < this->m_samplesData.size(); i++)
        {
            this->m_samplesData[i] = read_bitu8(newsrc);
            if((i >= 4) && (*((uint32_t*)(this->m_samplesData.data() + i - 4)) == 0x46464952))   /// RIFF
            {
                this->m_samplesCount++;
            }
        }

        SDL_RWclose(newsrc);
        newsrc = NULL;
    }
}