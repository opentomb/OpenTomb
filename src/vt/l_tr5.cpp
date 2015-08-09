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
#include <zlib.h>
#include "l_main.h"
#include "../system.h"
#include "../audio.h"

using namespace loader;

void TR_TR5Level::load()
{
    // Version
    uint32_t file_version = m_src.readU32();

    if(file_version != 0x00345254)
        Sys_extError("Wrong level version");

    m_numTextiles = 0;
    m_numRoomTextiles = 0;
    m_numObjTextiles = 0;
    m_numBumpTextiles = 0;
    m_numMiscTextiles = 0;
    m_read32BitTextiles = false;

    m_numRoomTextiles = m_src.readU16();
    m_numObjTextiles = m_src.readU16();
    m_numBumpTextiles = m_src.readU16();
    m_numMiscTextiles = 3;
    m_numTextiles = m_numRoomTextiles + m_numObjTextiles + m_numBumpTextiles + m_numMiscTextiles;

    auto uncomp_size = m_src.readU32();
    if(uncomp_size == 0)
        Sys_extError("read_tr5_level: textiles32 uncomp_size == 0");

    auto comp_size = m_src.readU32();
    if(comp_size > 0)
    {
        std::vector<uint8_t> uncomp_buffer(uncomp_size);

        m_textile32.resize(m_numTextiles);
        std::vector<uint8_t> comp_buffer(comp_size);

        m_src.readBytes(comp_buffer.data(), comp_size);

        uLongf size = uncomp_size;
        if(uncompress(uncomp_buffer.data(), &size, comp_buffer.data(), comp_size) != Z_OK)
            Sys_extError("read_tr5_level: uncompress");

        if(size != uncomp_size)
            Sys_extError("read_tr5_level: uncompress size mismatch");
        comp_buffer.clear();

        SDL_RWops* newsrcSDL = SDL_RWFromMem(uncomp_buffer.data(), uncomp_size);
        if(newsrcSDL == nullptr)
            Sys_extError("read_tr5_level: SDL_RWFromMem");

        io::SDLReader newsrc(newsrcSDL);
        newsrc.readVector(m_textile32, m_numTextiles - m_numMiscTextiles, &DWordTexture::read);
        m_read32BitTextiles = true;
    }

    uncomp_size = m_src.readU32();
    if(uncomp_size == 0)
        Sys_extError("read_tr5_level: textiles16 uncomp_size == 0");

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
                Sys_extError("read_tr5_level: uncompress");

            if(size != uncomp_size)
                Sys_extError("read_tr5_level: uncompress size mismatch");

            SDL_RWops* newsrcSDL = SDL_RWFromMem(uncomp_buffer.data(), uncomp_size);
            if(newsrcSDL == nullptr)
                Sys_extError("read_tr5_level: SDL_RWFromMem");

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
        Sys_extError("read_tr5_level: textiles32d uncomp_size == 0");

    comp_size = m_src.readU32();
    if(comp_size > 0)
    {
        std::vector<uint8_t> uncomp_buffer(uncomp_size);

        if((uncomp_size / (256 * 256 * 4)) > 3)
            Sys_extWarn("read_tr5_level: num_misc_textiles > 3");

        if(m_textile32.empty())
        {
            m_textile32.resize(m_numMiscTextiles);
        }

        std::vector<uint8_t> comp_buffer(comp_size);
        m_src.readBytes(comp_buffer.data(), comp_size);

        uLongf size = uncomp_size;
        if(uncompress(uncomp_buffer.data(), &size, comp_buffer.data(), comp_size) != Z_OK)
            Sys_extError("read_tr5_level: uncompress");

        if(size != uncomp_size)
            Sys_extError("read_tr5_level: uncompress size mismatch");

        SDL_RWops* newsrcSDL = SDL_RWFromMem(uncomp_buffer.data(), uncomp_size);
        if(newsrcSDL == nullptr)
            Sys_extError("read_tr5_level: SDL_RWFromMem");

        io::SDLReader newsrc(newsrcSDL);
        newsrc.readVector(m_textile32, m_numTextiles - m_numMiscTextiles, &DWordTexture::read);
    }

    // flags?
    /*
       I found 2 flags in the TR5 file format. Directly after the sprite textures are 2 ints as a flag. The first one is the lara type:
       0 Normal
       3 Catsuit
       4 Divesuit
       6 Invisible

       The second one is the weather type (effects all outside rooms):
       0 No weather
       1 Rain
       2 Snow (in title.trc these are red triangles falling from the sky).
     */
#if 1
    m_src.skip(2*sizeof(uint16_t));
#else
    i = m_src.readU16();
    i = m_src.readU16();
#endif
    if(m_src.readU32() != 0)
        Sys_extWarn("Bad value for flags[1]");

    if(m_src.readU32() != 0)
        Sys_extWarn("Bad value for flags[2]");

    if(m_src.readU32() != 0)
        Sys_extWarn("Bad value for flags[3]");

    if(m_src.readU32() != 0)
        Sys_extWarn("Bad value for flags[4]");

    if(m_src.readU32() != 0)
        Sys_extWarn("Bad value for flags[5]");

    if(m_src.readU32() != 0)
        Sys_extWarn("Bad value for flags[6]");

    if(m_src.readU32() != 0)
        Sys_extWarn("Bad value for flags[7]");

    // LevelDataSize1
    m_src.readU32();
    // LevelDataSize2
    m_src.readU32();

    // Unused
    if(m_src.readU32() != 0)
        Sys_extWarn("Bad value for 'unused'");

    m_src.readVector(m_rooms, m_src.readU32(), &Room::readTr5);

    m_src.readVector(m_floorData, m_src.readU32());

    read_mesh_data(m_src);

    m_src.readVector(m_animations, m_src.readU32(), &Animation::readTr4);

    m_src.readVector(m_stateChanges, m_src.readU32(), &StateChange::read);

    m_src.readVector(m_animDispatches, m_src.readU32(), &AnimDispatch::read);

    m_src.readVector(m_animCommands, m_src.readU32());

    m_src.readVector(m_meshTreeData, m_src.readU32());

    read_frame_moveable_data(m_src);

    m_src.readVector(m_staticMeshes, m_src.readU32(), &StaticMesh::read);

    if(m_src.readI8() != 'S')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if(m_src.readI8() != 'P')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if(m_src.readI8() != 'R')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if(m_src.readI8() != 0)
        Sys_extError("read_tr5_level: 'SPR' not found");

    m_src.readVector(m_spriteTextures, m_src.readU32(), &SpriteTexture::readTr4);

    m_src.readVector(m_spriteSequences, m_src.readU32(), &SpriteSequence::read);

    m_src.readVector(m_cameras, m_src.readU32(), &Camera::read);

    m_src.readVector(m_flybyCameras, m_src.readU32(), &FlybyCamera::read);

    m_src.readVector(m_soundSources, m_src.readU32(), &SoundSource::read);

    m_src.readVector(m_boxes, m_src.readU32(), &Box::readTr2);

    m_src.readVector(m_overlaps, m_src.readU32());

    // Zones
    m_src.skip(m_boxes.size() * 20);

    m_src.readVector(m_animatedTextures, m_src.readU32());

    m_animatedTexturesUvCount = m_src.readU8();

    if(m_src.readI8() != 'T')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if(m_src.readI8() != 'E')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if(m_src.readI8() != 'X')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if(m_src.readI8() != 0)
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    m_src.readVector(m_objectTextures, m_src.readU32(), &ObjectTexture::readTr5);

    m_src.readVector(m_items, m_src.readU32(), &Item::readTr4);

    m_src.readVector(m_aiObjects, m_src.readU32(), &AIObject::read);

    m_src.readVector(m_demoData, m_src.readU16());

    // Soundmap
    m_src.readVector(m_soundmap, TR_AUDIO_MAP_SIZE_TR5);

    m_src.readVector(m_soundDetails, m_src.readU32(), &SoundDetails::readTr3);

    m_src.readVector(m_sampleIndices, m_src.readU32());

    m_src.skip(6);   // In TR5, sample indices are followed by 6 0xCD bytes. - correct - really 0xCDCDCDCDCDCD

    // LOAD SAMPLES
    if(auto i = m_src.readU32())
    {
        m_samplesCount = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        m_src.readVector(m_samplesData, m_src.size() - m_src.tell());
    }
}
