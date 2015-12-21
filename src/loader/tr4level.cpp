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

#include "tr4level.h"

#include <iostream>

using namespace loader;

#define TR_AUDIO_MAP_SIZE_TR4  370

void TR4Level::load()
{
    // Version
    uint32_t file_version = m_reader.readU32();

    if(file_version != 0x00345254 /*&& file_version != 0x63345254*/)           // +TRLE
        BOOST_THROW_EXCEPTION( std::runtime_error("Wrong level version") );

    std::vector<WordTexture> texture16;
    {
        auto numRoomTextiles = m_reader.readU16();
        auto numObjTextiles = m_reader.readU16();
        auto numBumpTextiles = m_reader.readU16();
        auto numMiscTextiles = 2;
        auto numTextiles = numRoomTextiles + numObjTextiles + numBumpTextiles + numMiscTextiles;

        uint32_t uncomp_size = m_reader.readU32();
        if(uncomp_size == 0)
            BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: textiles32 uncomp_size == 0") );

        uint32_t comp_size = m_reader.readU32();
        if(comp_size > 0)
        {
            std::vector<uint8_t> comp_buffer(comp_size);
            m_reader.readBytes(comp_buffer.data(), comp_size);

            io::SDLReader newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
            newsrc.readVector(m_textures, numTextiles - numMiscTextiles, &DWordTexture::read);
        }

        uncomp_size = m_reader.readU32();
        if(uncomp_size == 0)
            BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: textiles16 uncomp_size == 0") );

        comp_size = m_reader.readU32();
        if(comp_size > 0)
        {
            if(m_textures.empty())
            {
                std::vector<uint8_t> comp_buffer(comp_size);
                m_reader.readBytes(comp_buffer.data(), comp_size);

                io::SDLReader newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
                newsrc.readVector(texture16, numTextiles - numMiscTextiles, &WordTexture::read);
            }
            else
            {
                m_reader.skip(comp_size);
            }
        }

        uncomp_size = m_reader.readU32();
        if(uncomp_size == 0)
            BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: textiles32d uncomp_size == 0") );

        comp_size = m_reader.readU32();
        if(comp_size > 0)
        {
            if(!m_textures.empty())
            {
                m_reader.skip(comp_size);
            }
            else
            {
                if(uncomp_size / (256 * 256 * 4) > 2)
                    std::cerr << "read_tr4_level: num_misc_textiles > 2\n";

                if(m_textures.empty())
                {
                    m_textures.resize(numTextiles);
                }
                std::vector<uint8_t> comp_buffer(comp_size);

                m_reader.readBytes(comp_buffer.data(), comp_size);

                io::SDLReader newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
                newsrc.appendVector(m_textures, numMiscTextiles, &DWordTexture::read);
            }
        }
    }

    auto uncomp_size = m_reader.readU32();
    if(uncomp_size == 0)
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: packed geometry uncomp_size == 0") );

    auto comp_size = m_reader.readU32();

    if(!comp_size)
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: packed geometry") );

    std::vector<uint8_t> comp_buffer(comp_size);
    m_reader.readBytes(comp_buffer.data(), comp_size);

    io::SDLReader newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
    if(!newsrc.isOpen())
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: SDL_RWFromMem") );

    // Unused
    if(newsrc.readU32() != 0)
        std::cerr << "Bad value for 'unused'\n";

    newsrc.readVector(m_rooms, newsrc.readU16(), &Room::readTr4);

    newsrc.readVector(m_floorData, newsrc.readU32());

    readMeshData(newsrc);

    newsrc.readVector(m_animations, newsrc.readU32(), &Animation::readTr4);

    newsrc.readVector(m_stateChanges, newsrc.readU32(), &StateChange::read);

    newsrc.readVector(m_animDispatches, newsrc.readU32(), AnimDispatch::read);

    newsrc.readVector(m_animCommands, newsrc.readU32());

    newsrc.readVector(m_meshTreeData, newsrc.readU32());

    readFrameMoveableData(newsrc);

    newsrc.readVector(m_staticMeshes, newsrc.readU32(), &StaticMesh::read);

    if(newsrc.readI8() != 'S')
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: 'SPR' not found") );

    if(newsrc.readI8() != 'P')
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: 'SPR' not found") );

    if(newsrc.readI8() != 'R')
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: 'SPR' not found") );

    newsrc.readVector(m_spriteTextures, newsrc.readU32(), &SpriteTexture::readTr4);

    newsrc.readVector(m_spriteSequences, newsrc.readU32(), &SpriteSequence::read);

    newsrc.readVector(m_cameras, newsrc.readU32(), &Camera::read);
    //SDL_RWseek(newsrc, this->cameras.size() * 16, SEEK_CUR);

    newsrc.readVector(m_flybyCameras, newsrc.readU32(), &FlybyCamera::read);
    //SDL_RWseek(newsrc, this->flyby_cameras.size() * 40, SEEK_CUR);

    newsrc.readVector(m_soundSources, newsrc.readU32(), &SoundSource::read);

    newsrc.readVector(m_boxes, newsrc.readU32(), &Box::readTr2);

    newsrc.readVector(m_overlaps, newsrc.readU32());

    newsrc.readVector(m_zones, m_boxes.size(), &Zone::readTr2);

    newsrc.readVector(m_animatedTextures, newsrc.readU32());

    m_animatedTexturesUvCount = newsrc.readU8();

    if(newsrc.readI8() != 'T')
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: '\\0TEX' not found") );

    if(newsrc.readI8() != 'E')
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: '\\0TEX' not found") );

    if(newsrc.readI8() != 'X')
        BOOST_THROW_EXCEPTION( std::runtime_error("read_tr4_level: '\\0TEX' not found") );

    newsrc.readVector(m_objectTextures, newsrc.readU32(), &ObjectTexture::readTr4);

    newsrc.readVector(m_items, newsrc.readU32(), &Item::readTr4);

    newsrc.readVector(m_aiObjects, newsrc.readU32(), &AIObject::read);

    newsrc.readVector(m_demoData, newsrc.readU16());

    // Soundmap
    newsrc.readVector(m_soundmap, TR_AUDIO_MAP_SIZE_TR4);

    newsrc.readVector(m_soundDetails, newsrc.readU32(), &SoundDetails::readTr3);

    // IMPORTANT NOTE: Sample indices ARE NOT USED in TR4 engine, but are parsed anyway.
    newsrc.readVector(m_sampleIndices, newsrc.readU32());

    // LOAD SAMPLES

    if(auto i = m_reader.readU32())
    {
        m_samplesCount = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        m_reader.readVector(m_samplesData, static_cast<size_t>(m_reader.size() - m_reader.tell()));
    }

    if(!m_textures.empty())
        return;

    m_textures.resize(texture16.size());
    for(size_t i = 0; i < texture16.size(); i++)
        convertTexture(texture16[i], m_textures[i]);
}
