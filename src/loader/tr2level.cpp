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

#include "tr2level.h"

using namespace loader;

#define TR_AUDIO_MAP_SIZE_TR2  370

void TR2Level::load()
{
    // Version
    uint32_t file_version = m_reader.readU32();

    if(file_version != 0x0000002d)
        BOOST_THROW_EXCEPTION(std::runtime_error("TR2 Level: Wrong level version"));

    m_palette = Palette::readTr1(m_reader);
    /*Palette palette16 =*/ Palette::readTr2(m_reader);

    auto numTextiles = m_reader.readU32();
    std::vector<ByteTexture> texture8;
    m_reader.readVector(texture8, numTextiles, &ByteTexture::read);
    std::vector<WordTexture> texture16;
    m_reader.readVector(texture16, numTextiles, &WordTexture::read);

    // Unused
    if(m_reader.readU32() != 0)
        BOOST_LOG_TRIVIAL(warning) << "TR2 Level: Bad value for 'unused'";

    m_reader.readVector(m_rooms, m_reader.readU16(), Room::readTr2);
    m_reader.readVector(m_floorData, m_reader.readU32());

    readMeshData(m_reader);

    m_reader.readVector(m_animations, m_reader.readU32(), &Animation::readTr1);

    m_reader.readVector(m_transitions, m_reader.readU32(), &Transitions::read);

    m_reader.readVector(m_transitionCases, m_reader.readU32(), &TransitionCase::read);

    m_reader.readVector(m_animCommands, m_reader.readU32());

    m_reader.readVector(m_boneTrees, m_reader.readU32());

    readPoseDataAndModels(m_reader);

    m_reader.readVector(m_staticMeshes, m_reader.readU32(), &StaticMesh::read);

    m_reader.readVector(m_objectTextures, m_reader.readU32(), &ObjectTexture::readTr1);

    m_reader.readVector(m_spriteTextures, m_reader.readU32(), &SpriteTexture::readTr1);

    m_reader.readVector(m_spriteSequences, m_reader.readU32(), &SpriteSequence::read);

    if(m_demoOrUb)
        m_lightmap = LightMap::read(m_reader);

    m_reader.readVector(m_cameras, m_reader.readU32(), &Camera::read);

    m_reader.readVector(m_soundSources, m_reader.readU32(), &SoundSource::read);

    m_reader.readVector(m_boxes, m_reader.readU32(), &Box::readTr2);

    m_reader.readVector(m_overlaps, m_reader.readU32());

    m_reader.readVector(m_zones, m_boxes.size(), &Zone::readTr2);

    m_animatedTexturesUvCount = 0; // No UVRotate in TR2
    m_reader.readVector(m_animatedTextures, m_reader.readU32());

    m_reader.readVector(m_items, m_reader.readU32(), &Item::readTr2);

    if(!m_demoOrUb)
        m_lightmap = LightMap::read(m_reader);

    m_reader.readVector(m_cinematicFrames, m_reader.readU16(), &CinematicFrame::read);

    m_reader.readVector(m_demoData, m_reader.readU16());

    // Soundmap
    m_reader.readVector(m_soundmap, TR_AUDIO_MAP_SIZE_TR2);

    m_reader.readVector(m_soundDetails, m_reader.readU32(), &SoundDetails::readTr1);

    m_reader.readVector(m_sampleIndices, m_reader.readU32());

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

    io::SDLReader newsrc(m_sfxPath);
    if(!newsrc.isOpen())
    {
        BOOST_LOG_TRIVIAL(warning) << "TR2 Level: failed to open '" << m_sfxPath << "', no samples loaded.";
    }
    else
    {
        m_samplesCount = 0;
        m_samplesData.resize(static_cast<size_t>(newsrc.size()));
        for(size_t i = 0; i < m_samplesData.size(); i++)
        {
            m_samplesData[i] = newsrc.readU8();
            if(i >= 4 && *reinterpret_cast<uint32_t*>(m_samplesData.data() + i - 4) == 0x46464952)   /// RIFF
            {
                m_samplesCount++;
            }
        }
    }

    m_textures.resize(texture16.size());
    for(size_t i = 0; i < texture16.size(); i++)
        convertTexture(texture16[i], m_textures[i]);
}
