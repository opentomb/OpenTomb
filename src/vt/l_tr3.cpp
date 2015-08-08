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

    read_tr_palette(src, m_palette);
    read_tr2_palette16(src, m_palette16);

    m_numTextiles = 0;
    m_numRoomTextiles = 0;
    m_numObjTextiles = 0;
    m_numBumpTextiles = 0;
    m_numMiscTextiles = 0;
    m_read32BitTextiles = false;

    m_numTextiles = read_bitu32(src);
    m_textile8.resize(m_numTextiles);
    for(i = 0; i < m_textile8.size(); i++)
        read_tr_textile8(src, m_textile8[i]);
    m_textile16.resize( m_textile8.size() );
    for(i = 0; i < m_textile16.size(); i++)
        read_tr2_textile16(src, m_textile16[i]);

    if(file_version == 0xFF180034)                                          // VICT.TR2
    {
        return;                                                             // Here only palette and textiles
    }

    // Unused
    if(read_bitu32(src) != 0)
        Sys_extWarn("Bad value for 'unused'");

    m_rooms.resize(read_bitu16(src));
    for(i = 0; i < m_rooms.size(); i++)
        read_tr3_room(src, m_rooms[i]);

    m_floorData.resize( read_bitu32(src) );
    for(i = 0; i < m_floorData.size(); i++)
        m_floorData[i] = read_bitu16(src);

    read_mesh_data(src);

    m_animations.resize( read_bitu32(src) );
    for(i = 0; i < m_animations.size(); i++)
        read_tr_animation(src, m_animations[i]);

    m_stateChanges.resize( read_bitu32(src) );
    for(i = 0; i < m_stateChanges.size(); i++)
        read_tr_state_changes(src, m_stateChanges[i]);

    m_animDispatches.resize( read_bitu32(src) );
    for(i = 0; i < m_animDispatches.size(); i++)
        read_tr_anim_dispatches(src, m_animDispatches[i]);

    m_animCommands.resize( read_bitu32(src) );
    for(i = 0; i < m_animCommands.size(); i++)
        m_animCommands[i] = read_bit16(src);

    m_meshTreeData.resize( read_bitu32(src) );
    for(i = 0; i < m_meshTreeData.size(); i++)
        m_meshTreeData[i] = read_bitu32(src);                     // 4 bytes

    read_frame_moveable_data(src);

    m_staticMeshes.resize( read_bitu32(src) );
    for(i = 0; i < m_staticMeshes.size(); i++)
        read_tr_staticmesh(src, m_staticMeshes[i]);

    m_spriteTextures.resize(read_bitu32(src));
    for(i = 0; i < m_spriteTextures.size(); i++)
        read_tr_sprite_texture(src, m_spriteTextures[i]);

    m_spriteSequences.resize( read_bitu32(src) );
    for(i = 0; i < m_spriteSequences.size(); i++)
        read_tr_sprite_sequence(src, m_spriteSequences[i]);

    m_cameras.resize( read_bitu32(src) );
    for(i = 0; i < m_cameras.size(); i++)
    {
        m_cameras[i].x = read_bit32(src);
        m_cameras[i].y = read_bit32(src);
        m_cameras[i].z = read_bit32(src);

        m_cameras[i].room = read_bit16(src);
        m_cameras[i].unknown1 = read_bitu16(src);
    }

    m_soundSources.resize( read_bitu32(src) );
    for(i = 0; i < m_soundSources.size(); i++)
    {
        m_soundSources[i].x = read_bit32(src);
        m_soundSources[i].y = read_bit32(src);
        m_soundSources[i].z = read_bit32(src);

        m_soundSources[i].sound_id = read_bitu16(src);
        m_soundSources[i].flags = read_bitu16(src);
    }

    m_boxes.resize( read_bitu32(src) );
    for(i = 0; i < m_boxes.size(); i++)
        read_tr2_box(src, m_boxes[i]);

    m_overlaps.resize(read_bitu32(src));
    for(i = 0; i < m_overlaps.size(); i++)
        m_overlaps[i] = read_bitu16(src);

    // Zones
    SDL_RWseek(src, m_boxes.size() * 20, RW_SEEK_CUR);

    m_animatedTextures.resize( read_bitu32(src) );
    m_animatedTexturesUvCount = 0; // No UVRotate in TR3
    for(i = 0; i < m_animatedTextures.size(); i++)
    {
        m_animatedTextures[i] = read_bitu16(src);
    }

    m_objectTextures.resize(read_bitu32(src));
    for(i = 0; i < m_objectTextures.size(); i++)
        read_tr_object_texture(src, m_objectTextures[i]);

    m_items.resize(read_bitu32(src));
    for(i = 0; i < m_items.size(); i++)
        read_tr3_item(src, m_items[i]);

    read_tr_lightmap(src, m_lightmap);

    m_cinematicFrames.resize( read_bitu16(src) );
    for(i = 0; i < m_cinematicFrames.size(); i++)
    {
        read_tr_cinematic_frame(src, m_cinematicFrames[i]);
    }

    m_demoData.resize( read_bitu16(src) );
    for(i = 0; i < m_demoData.size(); i++)
        m_demoData[i] = read_bitu8(src);

    // Soundmap
    m_soundmap.resize(TR_AUDIO_MAP_SIZE_TR3);
    for(i = 0; i < m_soundmap.size(); i++)
        m_soundmap[i] = read_bit16(src);

    m_soundDetails.resize( read_bitu32(src) );
    for(i = 0; i < m_soundDetails.size(); i++)
    {
        m_soundDetails[i].sample = read_bitu16(src);
        m_soundDetails[i].volume = (uint16_t)read_bitu8(src);
        m_soundDetails[i].sound_range = (uint16_t)read_bitu8(src);
        m_soundDetails[i].chance = (uint16_t)read_bit8(src);
        m_soundDetails[i].pitch = (int16_t)read_bit8(src);
        m_soundDetails[i].num_samples_and_flags_1 = read_bitu8(src);
        m_soundDetails[i].flags_2 = read_bitu8(src);
    }

    m_sampleIndices.resize( read_bitu32(src) );
    for(i = 0; i < m_sampleIndices.size(); i++)
        m_sampleIndices[i] = read_bitu32(src);

    // remap all sample indices here
    for(i = 0; i < m_soundDetails.size(); i++)
    {
        if(m_soundDetails[i].sample < m_sampleIndices.size())
        {
            m_soundDetails[i].sample = m_sampleIndices[m_soundDetails[i].sample];
        }
    }

    // LOAD SAMPLES

    // In TR3, samples are stored in separate file called MAIN.SFX.
    // If there is no such files, no samples are loaded.

    SDL_RWops *newsrc = SDL_RWFromFile(m_sfxPath.c_str(), "rb");
    if(newsrc == NULL)
    {
        Sys_extWarn("read_tr2_level: failed to open \"%s\"! No samples loaded.", m_sfxPath.c_str());
    }
    else
    {
        m_samplesData.resize(SDL_RWsize(newsrc));
        m_samplesCount = 0;
        for(i = 0; i < m_samplesData.size(); i++)
        {
            m_samplesData[i] = read_bitu8(newsrc);
            if((i >= 4) && (*((uint32_t*)(m_samplesData.data() + i - 4)) == 0x46464952))   /// RIFF
            {
                m_samplesCount++;
            }
        }

        SDL_RWclose(newsrc);
        newsrc = NULL;
    }
}