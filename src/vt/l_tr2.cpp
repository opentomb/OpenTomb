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
#include <GL/glew.h>
#include "l_main.h"
#include "../system.h"
#include "../audio.h"

void TR_TR2Level::load()
{
    uint32_t i;

    // Version
    uint32_t file_version = m_src.readU32();

    if (file_version != 0x0000002d)
        Sys_extError("Wrong level version");

    m_palette = tr2_palette_t::readTr1(m_src);
    m_palette16 = tr2_palette_t::readTr2(m_src);

    m_numTextiles = 0;
    m_numRoomTextiles = 0;
    m_numObjTextiles = 0;
    m_numBumpTextiles = 0;
    m_numMiscTextiles = 0;
    m_read32BitTextiles = false;
    
    m_numTextiles = m_src.readU32();
    m_textile8.resize(m_numTextiles);
    for (i = 0; i < m_textile8.size(); i++)
        read_tr_textile8(m_textile8[i]);
    m_textile16.resize(m_textile8.size());
    for (i = 0; i < m_textile16.size(); i++)
        m_textile16[i] = tr2_textile16_t::read(m_src);

    // Unused
    if (m_src.readU32() != 0)
        Sys_extWarn("Bad value for 'unused'");

    m_rooms.resize( m_src.readU16() );
    for (i = 0; i < m_rooms.size(); i++)
        m_rooms[i] = tr5_room_t::readTr2(m_src);

    m_floorData.resize( m_src.readU32() );
    for(i = 0; i < m_floorData.size(); i++)
        m_floorData[i] = m_src.readU16();

    read_mesh_data(m_src);

    m_animations.resize( m_src.readU32() );
    for (i = 0; i < m_animations.size(); i++)
        m_animations[i] = tr_animation_t::readTr1(m_src);

    m_stateChanges.resize( m_src.readU32() );
    for (i = 0; i < m_stateChanges.size(); i++)
        m_stateChanges[i] = tr_state_change_t::read(m_src);

    m_animDispatches.resize( m_src.readU32() );
    for (i = 0; i < m_animDispatches.size(); i++)
        m_animDispatches[i] = tr_anim_dispatch_t::read(m_src);

    m_animCommands.resize( m_src.readU32() );
    for (i = 0; i < m_animCommands.size(); i++)
        m_animCommands[i] = m_src.readI16();

    m_meshTreeData.resize( m_src.readU32() );
    for (i = 0; i < m_meshTreeData.size(); i++)
        m_meshTreeData[i] = m_src.readU32();                     // 4 bytes

    read_frame_moveable_data(m_src);

    m_staticMeshes.resize( m_src.readU32() );
    for (i = 0; i < m_staticMeshes.size(); i++)
        m_staticMeshes[i] = tr_staticmesh_t::read(m_src);

    m_objectTextures.resize( m_src.readU32() );
    for (i = 0; i < m_objectTextures.size(); i++)
        m_objectTextures[i] = tr4_object_texture_t::readTr1(m_src);

    m_spriteTextures.resize( m_src.readU32() );
    for (i = 0; i < m_spriteTextures.size(); i++)
        m_spriteTextures[i] = tr_sprite_texture_t::readTr1(m_src);

    m_spriteSequences.resize( m_src.readU32() );
    for (i = 0; i < m_spriteSequences.size(); i++)
        m_spriteSequences[i] = tr_sprite_sequence_t::read(m_src);

    if (m_demoOrUb)
        m_lightmap = tr_lightmap_t::read(m_src);

    m_cameras.resize( m_src.readU32() );
    for (i = 0; i < m_cameras.size(); i++)
    {
        m_cameras[i].x = m_src.readI32();
        m_cameras[i].y = m_src.readI32();
        m_cameras[i].z = m_src.readI32();

        m_cameras[i].room = m_src.readI16();
        m_cameras[i].unknown1 = m_src.readU16();
    }

    m_soundSources.resize( m_src.readU32() );
    for(i = 0; i < m_soundSources.size(); i++)
    {
        m_soundSources[i].x = m_src.readI32();
        m_soundSources[i].y = m_src.readI32();
        m_soundSources[i].z = m_src.readI32();

        m_soundSources[i].sound_id = m_src.readU16();
        m_soundSources[i].flags = m_src.readU16();
    }

    m_boxes.resize( m_src.readU32() );
    for (i = 0; i < m_boxes.size(); i++)
        m_boxes[i] = tr_box_t::readTr2(m_src);

    m_overlaps.resize(m_src.readU32());
    for (i = 0; i < m_overlaps.size(); i++)
        m_overlaps[i] = m_src.readU16();

    // Zones
    m_src.skip(m_boxes.size() * 20);

    m_animatedTextures.resize( m_src.readU32() );
    m_animatedTexturesUvCount = 0; // No UVRotate in TR2
    for (i = 0; i < m_animatedTextures.size(); i++)
    {
        m_animatedTextures[i] = m_src.readU16();
    }

    m_items.resize(m_src.readU32());
    for (i = 0; i < m_items.size(); i++)
        m_items[i] = tr2_item_t::readTr2(m_src);

    if (!m_demoOrUb)
        m_lightmap = tr_lightmap_t::read(m_src);

    m_cinematicFrames.resize( m_src.readU16() );
    for (i = 0; i < m_cinematicFrames.size(); i++)
    {
        m_cinematicFrames[i] = tr_cinematic_frame_t::read(m_src);
    }

    m_demoData.resize( m_src.readU16() );
    for(i=0; i < m_demoData.size(); i++)
        m_demoData[i] = m_src.readU8();

    // Soundmap
    m_soundmap.resize(TR_AUDIO_MAP_SIZE_TR2);
    for(i=0; i < m_soundmap.size(); i++)
        m_soundmap[i] = m_src.readI16();

    m_soundDetails.resize( m_src.readU32() );
    for(i = 0; i < m_soundDetails.size(); i++)
    {
        m_soundDetails[i].sample = m_src.readU16();
        m_soundDetails[i].volume = m_src.readU16();
        m_soundDetails[i].chance = m_src.readU16();
        m_soundDetails[i].num_samples_and_flags_1 = m_src.readU8();
        m_soundDetails[i].flags_2 = m_src.readU8();
        m_soundDetails[i].sound_range = TR_AUDIO_DEFAULT_RANGE;
        m_soundDetails[i].pitch = static_cast<int16_t>(TR_AUDIO_DEFAULT_PITCH);
    }

    m_sampleIndices.resize( m_src.readU32() );
    for(i=0; i < m_sampleIndices.size(); i++)
        m_sampleIndices[i] = m_src.readU32();

    // remap all sample indices here
    for(i = 0; i < m_soundDetails.size(); i++)
    {
        if(m_soundDetails[i].sample < m_sampleIndices.size())
        {
            m_soundDetails[i].sample = m_sampleIndices[m_soundDetails[i].sample];
        }
    }

    // LOAD SAMPLES

    // In TR2, samples are stored in separate file called MAIN.SFX.
    // If there is no such files, no samples are loaded.

    SDL_RWops* newsrcSdl = SDL_RWFromFile(m_sfxPath.c_str(), "rb");
    if (newsrcSdl == nullptr)
    {
        Sys_extWarn("read_tr2_level: failed to open \"%s\"! No samples loaded.", m_sfxPath.c_str());
    }
    else
    {
        io::SDLReader newsrc(newsrcSdl);
        m_samplesData.resize( newsrc.size() );
        m_samplesCount = 0;
        for(i = 0; i < m_samplesData.size(); i++)
        {
            m_samplesData[i] = newsrc.readU8();
            if((i >= 4) && (*reinterpret_cast<uint32_t*>(m_samplesData.data()+i-4) == 0x46464952))   /// RIFF
            {
                m_samplesCount++;
            }
        }
    }
}
