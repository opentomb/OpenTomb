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

#include "tr5level.h"

#include <iostream>

using namespace loader;

#define TR_AUDIO_MAP_SIZE_TR5  450

void TR5Level::load()
{
    // Version
    uint32_t file_version = m_reader.readU32();

    if(file_version != 0x00345254)
        throw std::runtime_error("Wrong level version");

    auto numRoomTextiles = m_reader.readU16();
    auto numObjTextiles = m_reader.readU16();
    auto numBumpTextiles = m_reader.readU16();
    auto numMiscTextiles = 3;
    auto numTextiles = numRoomTextiles + numObjTextiles + numBumpTextiles + numMiscTextiles;

    auto uncomp_size = m_reader.readU32();
    if(uncomp_size == 0)
        throw std::runtime_error("read_tr5_level: textiles32 uncomp_size == 0");

    auto comp_size = m_reader.readU32();
    if(comp_size > 0)
    {
        std::vector<uint8_t> comp_buffer(comp_size);
        m_reader.readBytes(comp_buffer.data(), comp_size);

        io::SDLReader newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
        newsrc.readVector(m_textile32, numTextiles - numMiscTextiles, &DWordTexture::read);
    }

    uncomp_size = m_reader.readU32();
    if(uncomp_size == 0)
        throw std::runtime_error("read_tr5_level: textiles16 uncomp_size == 0");

    comp_size = m_reader.readU32();
    if(comp_size > 0)
    {
        if(m_textile32.empty())
        {
            std::vector<uint8_t> comp_buffer(comp_size);
            m_reader.readBytes(comp_buffer.data(), comp_size);

            io::SDLReader newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
            newsrc.readVector(m_textile16, numTextiles - numMiscTextiles, &WordTexture::read);
        }
        else
        {
            m_reader.skip(comp_size);
        }
    }

    uncomp_size = m_reader.readU32();
    if(uncomp_size == 0)
        throw std::runtime_error("read_tr5_level: textiles32d uncomp_size == 0");

    comp_size = m_reader.readU32();
    if(comp_size > 0)
    {
        if(m_textile32.empty())
        {
            if((uncomp_size / (256 * 256 * 4)) > 3)
                std::cerr << "read_tr5_level: num_misc_textiles > 3\n";

            std::vector<uint8_t> comp_buffer(comp_size);
            m_reader.readBytes(comp_buffer.data(), comp_size);

            io::SDLReader newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
            newsrc.appendVector(m_textile32, numMiscTextiles, &DWordTexture::read);
        }
        else
        {
            m_reader.skip(uncomp_size);
        }
    }

    m_laraType = m_reader.readU16();
    m_weatherType = m_reader.readU16();

    if(m_reader.readU32() != 0)
        std::cerr << "Bad value for flags[1]\n";

    if(m_reader.readU32() != 0)
        std::cerr << "Bad value for flags[2]\n";

    if(m_reader.readU32() != 0)
        std::cerr << "Bad value for flags[3]\n";

    if(m_reader.readU32() != 0)
        std::cerr << "Bad value for flags[4]\n";

    if(m_reader.readU32() != 0)
        std::cerr << "Bad value for flags[5]\n";

    if(m_reader.readU32() != 0)
        std::cerr << "Bad value for flags[6]\n";

    if(m_reader.readU32() != 0)
        std::cerr << "Bad value for flags[7]\n";

    // LevelDataSize1
    m_reader.readU32();
    // LevelDataSize2
    m_reader.readU32();

    // Unused
    if(m_reader.readU32() != 0)
        std::cerr << "Bad value for 'unused'\n";

    m_reader.readVector(m_rooms, m_reader.readU32(), &Room::readTr5);

    m_reader.readVector(m_floorData, m_reader.readU32());

    readMeshData(m_reader);

    m_reader.readVector(m_animations, m_reader.readU32(), &Animation::readTr4);

    m_reader.readVector(m_stateChanges, m_reader.readU32(), &StateChange::read);

    m_reader.readVector(m_animDispatches, m_reader.readU32(), &AnimDispatch::read);

    m_reader.readVector(m_animCommands, m_reader.readU32());

    m_reader.readVector(m_meshTreeData, m_reader.readU32());

    readFrameMoveableData(m_reader);

    m_reader.readVector(m_staticMeshes, m_reader.readU32(), &StaticMesh::read);

    if(m_reader.readI8() != 'S')
        throw std::runtime_error("read_tr5_level: 'SPR' not found");

    if(m_reader.readI8() != 'P')
        throw std::runtime_error("read_tr5_level: 'SPR' not found");

    if(m_reader.readI8() != 'R')
        throw std::runtime_error("read_tr5_level: 'SPR' not found");

    if(m_reader.readI8() != 0)
        throw std::runtime_error("read_tr5_level: 'SPR' not found");

    m_reader.readVector(m_spriteTextures, m_reader.readU32(), &SpriteTexture::readTr4);

    m_reader.readVector(m_spriteSequences, m_reader.readU32(), &SpriteSequence::read);

    m_reader.readVector(m_cameras, m_reader.readU32(), &Camera::read);

    m_reader.readVector(m_flybyCameras, m_reader.readU32(), &FlybyCamera::read);

    m_reader.readVector(m_soundSources, m_reader.readU32(), &SoundSource::read);

    m_reader.readVector(m_boxes, m_reader.readU32(), &Box::readTr2);

    m_reader.readVector(m_overlaps, m_reader.readU32());

    // Zones
    m_reader.skip(m_boxes.size() * 20);

    m_reader.readVector(m_animatedTextures, m_reader.readU32());

    m_animatedTexturesUvCount = m_reader.readU8();

    if(m_reader.readI8() != 'T')
        throw std::runtime_error("read_tr5_level: '\\0TEX' not found");

    if(m_reader.readI8() != 'E')
        throw std::runtime_error("read_tr5_level: '\\0TEX' not found");

    if(m_reader.readI8() != 'X')
        throw std::runtime_error("read_tr5_level: '\\0TEX' not found");

    if(m_reader.readI8() != 0)
        throw std::runtime_error("read_tr5_level: '\\0TEX' not found");

    m_reader.readVector(m_objectTextures, m_reader.readU32(), &ObjectTexture::readTr5);

    m_reader.readVector(m_items, m_reader.readU32(), &Item::readTr4);

    m_reader.readVector(m_aiObjects, m_reader.readU32(), &AIObject::read);

    m_reader.readVector(m_demoData, m_reader.readU16());

    // Soundmap
    m_reader.readVector(m_soundmap, TR_AUDIO_MAP_SIZE_TR5);

    m_reader.readVector(m_soundDetails, m_reader.readU32(), &SoundDetails::readTr3);

    m_reader.readVector(m_sampleIndices, m_reader.readU32());

    m_reader.skip(6);   // In TR5, sample indices are followed by 6 0xCD bytes. - correct - really 0xCDCDCDCDCDCD

    // LOAD SAMPLES
    if(auto i = m_reader.readU32())
    {
        m_samplesCount = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        m_reader.readVector(m_samplesData, static_cast<size_t>(m_reader.size() - m_reader.tell()));
    }

    if(!m_textile32.empty())
        return;

    m_textile32.resize(m_textile16.size());
    for(size_t i = 0; i < m_textile16.size(); i++)
        convertTexture(m_textile16[i], m_textile32[i]);
}
