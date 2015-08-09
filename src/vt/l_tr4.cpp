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

#include <SDL2/SDL_endian.h>
#include <zlib.h>
#include "l_main.h"
#include "../system.h"
#include "../audio.h"

using namespace loader;

void TR_TR4Level::load()
{
    // Version
    uint32_t file_version = m_src.readU32();

    if(file_version != 0x00345254 /*&& file_version != 0x63345254*/)           // +TRLE
        Sys_extError("Wrong level version");

    m_numTextiles = 0;
    m_numRoomTextiles = 0;
    m_numObjTextiles = 0;
    m_numBumpTextiles = 0;
    m_numMiscTextiles = 0;
    m_read32BitTextiles = false;

    {
        m_numRoomTextiles = m_src.readU16();
        m_numObjTextiles = m_src.readU16();
        m_numBumpTextiles = m_src.readU16();
        m_numMiscTextiles = 2;
        m_numTextiles = m_numRoomTextiles + m_numObjTextiles + m_numBumpTextiles + m_numMiscTextiles;

        uint32_t uncomp_size = m_src.readU32();
        if(uncomp_size == 0)
            Sys_extError("read_tr4_level: textiles32 uncomp_size == 0");

        uint32_t comp_size = m_src.readU32();
        if(comp_size > 0)
        {
            std::vector<uint8_t> uncomp_buffer(uncomp_size);

            m_textile32.resize(m_numTextiles);
            std::vector<uint8_t> comp_buffer(comp_size);
            m_src.readBytes(comp_buffer.data(), comp_size);

            uLongf size = uncomp_size;
            if(uncompress(uncomp_buffer.data(), &size, comp_buffer.data(), comp_size) != Z_OK)
                Sys_extError("read_tr4_level: uncompress");

            if(size != uncomp_size)
                Sys_extError("read_tr4_level: uncompress size mismatch");
            comp_buffer.clear();

            SDL_RWops* newsrcSdl = SDL_RWFromMem(uncomp_buffer.data(), uncomp_size);
            if(newsrcSdl == nullptr)
                Sys_extError("read_tr4_level: SDL_RWFromMem");
            io::SDLReader newsrc(newsrcSdl);
            newsrc.readVector(m_textile32, m_numTextiles - m_numMiscTextiles, &DWordTexture::read);

            m_read32BitTextiles = true;
        }

        uncomp_size = m_src.readU32();
        if(uncomp_size == 0)
            Sys_extError("read_tr4_level: textiles16 uncomp_size == 0");

        comp_size = m_src.readU32();
        if(comp_size > 0)
        {
            if(m_textile32.empty())
            {
                std::vector<uint8_t> uncomp_buffer(uncomp_size);

                m_textile16.resize(m_numTextiles);
                std::vector<uint8_t> comp_buffer(comp_size);
                m_src.readBytes(comp_buffer.data(), comp_size);

                uLongf size = uncomp_size;
                if(uncompress(uncomp_buffer.data(), &size, comp_buffer.data(), comp_size) != Z_OK)
                    Sys_extError("read_tr4_level: uncompress");

                if(size != uncomp_size)
                    Sys_extError("read_tr4_level: uncompress size mismatch");
                comp_buffer.clear();

                SDL_RWops* newsrcSDL = SDL_RWFromMem(uncomp_buffer.data(), uncomp_size);
                if(newsrcSDL == nullptr)
                    Sys_extError("read_tr4_level: SDL_RWFromMem");
                io::SDLReader newsrc(newsrcSDL);
                newsrc.readVector(m_textile16, m_numTextiles - m_numMiscTextiles, &WordTexture::read);
            }
            else
            {
                m_src.skip(comp_size);
            }
        }

        uncomp_size = m_src.readU32();
        if(uncomp_size == 0)
            Sys_extError("read_tr4_level: textiles32d uncomp_size == 0");

        comp_size = m_src.readU32();
        if(comp_size > 0)
        {
            std::vector<uint8_t> uncomp_buffer(uncomp_size);

            if((uncomp_size / (256 * 256 * 4)) > 2)
                Sys_extWarn("read_tr4_level: num_misc_textiles > 2");

            if(m_textile32.empty())
            {
                m_textile32.resize(m_numTextiles);
            }
            std::vector<uint8_t> comp_buffer(comp_size);

            m_src.readBytes(comp_buffer.data(), comp_size);

            uLongf size = uncomp_size;
            if(uncompress(uncomp_buffer.data(), &size, comp_buffer.data(), comp_size) != Z_OK)
                Sys_extError("read_tr4_level: uncompress");

            if(size != uncomp_size)
                Sys_extError("read_tr4_level: uncompress size mismatch");
            comp_buffer.clear();

            SDL_RWops* newsrcSDL = SDL_RWFromMem(uncomp_buffer.data(), uncomp_size);
            if(newsrcSDL == nullptr)
                Sys_extError("read_tr4_level: SDL_RWFromMem");
            io::SDLReader newsrc(newsrcSDL);
            newsrc.readVector(m_textile32, m_numTextiles - m_numMiscTextiles, &DWordTexture::read);
        }
    }

    auto uncomp_size = m_src.readU32();
    if(uncomp_size == 0)
        Sys_extError("read_tr4_level: packed geometry uncomp_size == 0");

    auto comp_size = m_src.readU32();

    if(!comp_size)
        Sys_extError("read_tr4_level: packed geometry");

    std::vector<uint8_t> uncomp_buffer(uncomp_size);
    std::vector<uint8_t> comp_buffer(comp_size);
    m_src.readBytes(comp_buffer.data(), comp_size);

    uLongf size = uncomp_size;
    if(uncompress(uncomp_buffer.data(), &size, comp_buffer.data(), comp_size) != Z_OK)
        Sys_extError("read_tr4_level: uncompress");

    if(size != uncomp_size)
        Sys_extError("read_tr4_level: uncompress size mismatch");
    comp_buffer.clear();

    SDL_RWops* newsrcSDL = SDL_RWFromMem(uncomp_buffer.data(), uncomp_size);

    if(newsrcSDL == nullptr)
        Sys_extError("read_tr4_level: SDL_RWFromMem");

    io::SDLReader newsrc(newsrcSDL);

    // Unused
    if(newsrc.readU32() != 0)
        Sys_extWarn("Bad value for 'unused'");

    newsrc.readVector(m_rooms, newsrc.readU16(), &Room::readTr4);

    newsrc.readVector(m_floorData, newsrc.readU32());

    read_mesh_data(newsrc);

    newsrc.readVector(m_animations, newsrc.readU32(), &Animation::readTr4);

    newsrc.readVector(m_stateChanges, newsrc.readU32(), &StateChange::read);

    newsrc.readVector(m_animDispatches, newsrc.readU32(), AnimDispatch::read);

    newsrc.readVector(m_animCommands, newsrc.readU32());

    newsrc.readVector(m_meshTreeData, newsrc.readU32());

    read_frame_moveable_data(newsrc);

    newsrc.readVector(m_staticMeshes, newsrc.readU32(), &StaticMesh::read);

    if(newsrc.readI8() != 'S')
        Sys_extError("read_tr4_level: 'SPR' not found");

    if(newsrc.readI8() != 'P')
        Sys_extError("read_tr4_level: 'SPR' not found");

    if(newsrc.readI8() != 'R')
        Sys_extError("read_tr4_level: 'SPR' not found");

    newsrc.readVector(m_spriteTextures, newsrc.readU32(), &SpriteTexture::readTr4);

    newsrc.readVector(m_spriteSequences, newsrc.readU32(), &SpriteSequence::read);

    newsrc.readVector(m_cameras, newsrc.readU32(), &Camera::read);
    //SDL_RWseek(newsrc, this->cameras.size() * 16, SEEK_CUR);

    newsrc.readVector(m_flybyCameras, newsrc.readU32(), &FlybyCamera::read);
    //SDL_RWseek(newsrc, this->flyby_cameras.size() * 40, SEEK_CUR);

    newsrc.readVector(m_soundSources, newsrc.readU32(), &SoundSource::read);

    newsrc.readVector(m_boxes, newsrc.readU32(), &Box::readTr2);

    newsrc.readVector(m_overlaps, newsrc.readU32());

    // Zones
    newsrc.skip(m_boxes.size() * 20);

    newsrc.readVector(m_animatedTextures, newsrc.readU32());

    m_animatedTexturesUvCount = newsrc.readU8();

    if(newsrc.readI8() != 'T')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

    if(newsrc.readI8() != 'E')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

    if(newsrc.readI8() != 'X')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

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

    if(auto i = m_src.readU32())
    {
        m_samplesCount = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        m_src.readVector(m_samplesData, m_src.tell() - m_src.size());
    }
}
