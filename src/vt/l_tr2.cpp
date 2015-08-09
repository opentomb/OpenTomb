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

using namespace loader;

void TR_TR2Level::load()
{
    // Version
    uint32_t file_version = m_src.readU32();

    if (file_version != 0x0000002d)
        Sys_extError("Wrong level version");

    m_palette = Palette::readTr1(m_src);
    m_palette16 = Palette::readTr2(m_src);

    m_numTextiles = 0;
    m_numRoomTextiles = 0;
    m_numObjTextiles = 0;
    m_numBumpTextiles = 0;
    m_numMiscTextiles = 0;
    m_read32BitTextiles = false;
    
    m_numTextiles = m_src.readU32();
    m_src.readVector(m_textile8, m_numTextiles, &ByteTexture::read);
    m_src.readVector(m_textile16, m_numTextiles, &WordTexture::read);

    // Unused
    if (m_src.readU32() != 0)
        Sys_extWarn("Bad value for 'unused'");

    m_src.readVector(m_rooms, m_src.readU16(), Room::readTr2);
    m_src.readVector(m_floorData, m_src.readU32());

    read_mesh_data(m_src);

    m_src.readVector(m_animations, m_src.readU32(), &Animation::readTr1);

    m_src.readVector(m_stateChanges, m_src.readU32(), &StateChange::read);

    m_src.readVector(m_animDispatches, m_src.readU32(), &AnimDispatch::read);

    m_src.readVector(m_animCommands, m_src.readU32());

    m_src.readVector(m_meshTreeData, m_src.readU32());

    read_frame_moveable_data(m_src);

    m_src.readVector(m_staticMeshes, m_src.readU32(), &StaticMesh::read);

    m_src.readVector(m_objectTextures, m_src.readU32(), &ObjectTexture::readTr1);

    m_src.readVector(m_spriteTextures, m_src.readU32(), &SpriteTexture::readTr1);

    m_src.readVector(m_spriteSequences, m_src.readU32(), &SpriteSequence::read);

    if (m_demoOrUb)
        m_lightmap = LightMap::read(m_src);

    m_src.readVector(m_cameras, m_src.readU32(), &Camera::read);

    m_src.readVector(m_soundSources, m_src.readU32(), &SoundSource::read);

    m_src.readVector(m_boxes, m_src.readU32(), &Box::readTr2);

    m_src.readVector(m_overlaps, m_src.readU32());

    // Zones
    m_src.skip(m_boxes.size() * 20);

    m_animatedTexturesUvCount = 0; // No UVRotate in TR2
    m_src.readVector(m_animatedTextures, m_src.readU32());

    m_src.readVector(m_items, m_src.readU32(), &Item::readTr2);

    if (!m_demoOrUb)
        m_lightmap = LightMap::read(m_src);

    m_src.readVector(m_cinematicFrames, m_src.readU16(), &CinematicFrame::read);

    m_src.readVector(m_demoData, m_src.readU16());

    // Soundmap
    m_src.readVector(m_soundmap, TR_AUDIO_MAP_SIZE_TR2);

    m_src.readVector(m_soundDetails, m_src.readU32(), &SoundDetails::readTr1);

    m_src.readVector(m_sampleIndices, m_src.readU32());

    // remap all sample indices here
    for(size_t i = 0; i < m_soundDetails.size(); i++)
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
        m_samplesCount = 0;
        m_samplesData.resize( newsrc.size() );
        for(size_t i = 0; i < m_samplesData.size(); i++)
        {
            m_samplesData[i] = newsrc.readU8();
            if((i >= 4) && (*reinterpret_cast<uint32_t*>(m_samplesData.data()+i-4) == 0x46464952))   /// RIFF
            {
                m_samplesCount++;
            }
        }
    }
}
