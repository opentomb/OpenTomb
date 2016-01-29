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

#include "tr1level.h"

using namespace loader;

#define TR_AUDIO_MAP_SIZE_TR1  256

void TR1Level::load()
{
    BOOST_LOG_TRIVIAL(debug) << "Start. File size = " << m_reader.size();

    m_reader.seek(0);

    // Version
    uint32_t file_version = m_reader.readU32();

    if(file_version != 0x00000020)
        BOOST_THROW_EXCEPTION(std::runtime_error("TR1 Level: Wrong level version"));

    BOOST_LOG_TRIVIAL(debug) << "Reading textures";
    std::vector<ByteTexture> texture8;
    m_reader.readVector(texture8, m_reader.readU32(), &ByteTexture::read);

    // Unused
    if(m_reader.readU32() != 0)
        BOOST_LOG_TRIVIAL(warning) << "TR1 Level: Bad value for 'unused'";

    BOOST_LOG_TRIVIAL(debug) << "Reading rooms";
    m_reader.readVector(m_rooms, m_reader.readU16(), &Room::readTr1);

    BOOST_LOG_TRIVIAL(debug) << "Reading floor data";
    m_reader.readVector(m_floorData, m_reader.readU32());

    BOOST_LOG_TRIVIAL(debug) << "Reading mesh data";
    readMeshData(m_reader);

    BOOST_LOG_TRIVIAL(debug) << "Reading animations";
    m_reader.readVector(m_animations, m_reader.readU32(), &Animation::readTr1);

    BOOST_LOG_TRIVIAL(debug) << "Reading state changes";
    m_reader.readVector(m_stateChanges, m_reader.readU32(), &StateChange::read);

    BOOST_LOG_TRIVIAL(debug) << "Reading animation dispatches";
    m_reader.readVector(m_animDispatches, m_reader.readU32(), &AnimDispatch::read);

    BOOST_LOG_TRIVIAL(debug) << "Reading animation commands";
    m_reader.readVector(m_animCommands, m_reader.readU32());

    BOOST_LOG_TRIVIAL(debug) << "Reading mesh tree";
    m_reader.readVector(m_meshTreeData, m_reader.readU32());

    BOOST_LOG_TRIVIAL(debug) << "Reading moveables";
    readFrameMoveableData(m_reader);

    // try to fix ugly stick
    for(size_t i = 0; i < m_animations.size(); i++)
    {
        uint32_t frame_offset = m_animations[i].frame_offset / 2;
        m_animations[i].poseDataSize = m_frameData[frame_offset + 9] * 2 + 10;
    }

    BOOST_LOG_TRIVIAL(debug) << "Reading static meshes";
    m_reader.readVector(m_staticMeshes, m_reader.readU32(), &StaticMesh::read);

    BOOST_LOG_TRIVIAL(debug) << "Reading object textures";
    m_reader.readVector(m_objectTextures, m_reader.readU32(), ObjectTexture::readTr1);

    BOOST_LOG_TRIVIAL(debug) << "Reading sprite textures";
    m_reader.readVector(m_spriteTextures, m_reader.readU32(), &SpriteTexture::readTr1);

    BOOST_LOG_TRIVIAL(debug) << "Reading sprite sequences";
    m_reader.readVector(m_spriteSequences, m_reader.readU32(), &SpriteSequence::read);

    if(m_demoOrUb)
    {
        BOOST_LOG_TRIVIAL(debug) << "Reading palette";
        m_palette = Palette::readTr1(m_reader);
    }

    BOOST_LOG_TRIVIAL(debug) << "Reading cameras";
    m_reader.readVector(m_cameras, m_reader.readU32(), &Camera::read);

    BOOST_LOG_TRIVIAL(debug) << "Reading sound sources";
    m_reader.readVector(m_soundSources, m_reader.readU32(), &SoundSource::read);

    BOOST_LOG_TRIVIAL(debug) << "Reading boxes";
    m_reader.readVector(m_boxes, m_reader.readU32(), &Box::readTr1);

    BOOST_LOG_TRIVIAL(debug) << "Reading overlaps";
    m_reader.readVector(m_overlaps, m_reader.readU32());

    BOOST_LOG_TRIVIAL(debug) << "Reading zones";
    m_reader.readVector(m_zones, m_boxes.size(), &Zone::readTr1);

    BOOST_LOG_TRIVIAL(debug) << "Reading animated textures";
    m_animatedTexturesUvCount = 0; // No UVRotate in TR1
    m_reader.readVector(m_animatedTextures, m_reader.readU32());

    BOOST_LOG_TRIVIAL(debug) << "Reading items";
    m_reader.readVector(m_items, m_reader.readU32(), &Item::readTr1);

    BOOST_LOG_TRIVIAL(debug) << "Reading lightmap";
    m_lightmap = LightMap::read(m_reader);

    if(!m_demoOrUb)
    {
        BOOST_LOG_TRIVIAL(debug) << "Reading palette";
        m_palette = Palette::readTr1(m_reader);
    }

    BOOST_LOG_TRIVIAL(debug) << "Reading cinematic frames";
    m_reader.readVector(m_cinematicFrames, m_reader.readU16(), &CinematicFrame::read);

    BOOST_LOG_TRIVIAL(debug) << "Reading demo data";
    m_reader.readVector(m_demoData, m_reader.readU16());

    // Soundmap
    BOOST_LOG_TRIVIAL(debug) << "Reading soundmap";
    m_reader.readVector(m_soundmap, TR_AUDIO_MAP_SIZE_TR1);

    BOOST_LOG_TRIVIAL(debug) << "Reading sound details";
    m_reader.readVector(m_soundDetails, m_reader.readU32(), &SoundDetails::readTr1);

    // LOAD SAMPLES

    // In TR1, samples are embedded into level file as solid block, preceded by
    // block size in bytes. Sample block is followed by sample indices array.

#if 0
    m_samplesCount = 0;
    m_samplesData.resize(m_src.readU32());
    for(size_t i = 0; i < m_samplesData.size(); i++)
    {
        m_samplesData[i] = m_src.readU8();
        if((i >= 4) && (*reinterpret_cast<uint32_t*>(m_samplesData.data() + i - 4) == 0x46464952))   /// RIFF
        {
            m_samplesCount++;
        }
    }

    m_sampleIndices.resize(m_src.readU32());
    for(size_t i = 0; i < m_sampleIndices.size(); i++)
        m_sampleIndices[i] = m_src.readU32();
#else
    BOOST_LOG_TRIVIAL(debug) << "Reading sample data";
    m_reader.readVector(m_samplesData, m_reader.readU32());
    BOOST_LOG_TRIVIAL(debug) << "Reading sample indices";
    m_reader.readVector(m_sampleIndices, m_reader.readU32());
    m_samplesCount = m_sampleIndices.size();
#endif

    BOOST_LOG_TRIVIAL(debug) << "Converting textures";
    m_textures.resize(texture8.size());
    for(size_t i = 0; i < texture8.size(); i++)
        convertTexture(texture8[i], *m_palette, m_textures[i]);

    BOOST_LOG_TRIVIAL(debug) << "Done. File position = " << m_reader.tell();
}
