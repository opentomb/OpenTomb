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

            for(size_t i = 0; i < (m_numTextiles - m_numMiscTextiles); i++)
                m_textile32[i] = tr4_textile32_t::read(newsrc);

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

                for(size_t i = 0; i < (m_numTextiles - m_numMiscTextiles); i++)
                    m_textile16[i] = tr2_textile16_t::read(newsrc);
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

            for(size_t i = (m_numTextiles - m_numMiscTextiles); i < m_numTextiles; i++)
                m_textile32[i] = tr4_textile32_t::read(newsrc);
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

    m_rooms.resize(newsrc.readU16());
    for(size_t i = 0; i < m_rooms.size(); i++)
        m_rooms[i] = tr5_room_t::readTr4(newsrc);

    m_floorData.resize(newsrc.readU32());
    for(size_t i = 0; i < m_floorData.size(); i++)
        m_floorData[i] = newsrc.readU16();

    read_mesh_data(newsrc);

    m_animations.resize(newsrc.readU32());
    for(size_t i = 0; i < m_animations.size(); i++)
        m_animations[i] = tr_animation_t::readTr4(newsrc);

    m_stateChanges.resize(newsrc.readU32());
    for(size_t i = 0; i < m_stateChanges.size(); i++)
        m_stateChanges[i] = tr_state_change_t::read(newsrc);

    m_animDispatches.resize(newsrc.readU32());
    for(size_t i = 0; i < m_animDispatches.size(); i++)
        m_animDispatches[i] = tr_anim_dispatch_t::read(newsrc);

    m_animCommands.resize(newsrc.readU32());
    for(size_t i = 0; i < m_animCommands.size(); i++)
        m_animCommands[i] = newsrc.readI16();

    m_meshTreeData.resize(newsrc.readU32());
    for(size_t i = 0; i < m_meshTreeData.size(); i++)
        m_meshTreeData[i] = newsrc.readU32();                     // 4 bytes

    read_frame_moveable_data(newsrc);

    m_staticMeshes.resize(newsrc.readU32());
    for(size_t i = 0; i < m_staticMeshes.size(); i++)
        m_staticMeshes[i] = tr_staticmesh_t::read(newsrc);

    if(newsrc.readI8() != 'S')
        Sys_extError("read_tr4_level: 'SPR' not found");

    if(newsrc.readI8() != 'P')
        Sys_extError("read_tr4_level: 'SPR' not found");

    if(newsrc.readI8() != 'R')
        Sys_extError("read_tr4_level: 'SPR' not found");

    m_spriteTextures.resize(newsrc.readU32());
    for(size_t i = 0; i < m_spriteTextures.size(); i++)
        m_spriteTextures[i] = tr_sprite_texture_t::readTr4(newsrc);

    m_spriteSequences.resize(newsrc.readU32());
    for(size_t i = 0; i < m_spriteSequences.size(); i++)
        m_spriteSequences[i] = tr_sprite_sequence_t::read(newsrc);

    m_cameras.resize(newsrc.readU32());
    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        m_cameras[i].x = newsrc.readI32();
        m_cameras[i].y = newsrc.readI32();
        m_cameras[i].z = newsrc.readI32();

        m_cameras[i].room = newsrc.readI16();
        m_cameras[i].unknown1 = newsrc.readU16();
    }
    //SDL_RWseek(newsrc, this->cameras.size() * 16, SEEK_CUR);

    m_flybyCameras.resize(newsrc.readU32());
    for(size_t i = 0; i < m_flybyCameras.size(); i++)
    {
        m_flybyCameras[i].cam_x = newsrc.readI32();
        m_flybyCameras[i].cam_y = newsrc.readI32();
        m_flybyCameras[i].cam_z = newsrc.readI32();
        m_flybyCameras[i].target_x = newsrc.readI32();
        m_flybyCameras[i].target_y = newsrc.readI32();
        m_flybyCameras[i].target_z = newsrc.readI32();

        m_flybyCameras[i].sequence = newsrc.readI8();
        m_flybyCameras[i].index = newsrc.readI8();

        m_flybyCameras[i].fov = newsrc.readU16();
        m_flybyCameras[i].roll = newsrc.readU16();
        m_flybyCameras[i].timer = newsrc.readU16();
        m_flybyCameras[i].speed = newsrc.readU16();
        m_flybyCameras[i].flags = newsrc.readU16();

        m_flybyCameras[i].room_id = newsrc.readU32();
    }
    //SDL_RWseek(newsrc, this->flyby_cameras.size() * 40, SEEK_CUR);

    m_soundSources.resize(newsrc.readU32());
    for(size_t i = 0; i < m_soundSources.size(); i++)
    {
        m_soundSources[i].x = newsrc.readI32();
        m_soundSources[i].y = newsrc.readI32();
        m_soundSources[i].z = newsrc.readI32();

        m_soundSources[i].sound_id = newsrc.readU16();
        m_soundSources[i].flags = newsrc.readU16();
    }

    m_boxes.resize(newsrc.readU32());
    for(size_t i = 0; i < m_boxes.size(); i++)
        m_boxes[i] = tr_box_t::readTr2(newsrc);

    m_overlaps.resize(newsrc.readU32());
    for(size_t i = 0; i < m_overlaps.size(); i++)
        m_overlaps[i] = newsrc.readU16();

    // Zones
    newsrc.skip(m_boxes.size() * 20);

    m_animatedTextures.resize(newsrc.readU32());
    for(size_t i = 0; i < m_animatedTextures.size(); i++)
    {
        m_animatedTextures[i] = newsrc.readU16();
    }

    m_animatedTexturesUvCount = newsrc.readU8();

    if(newsrc.readI8() != 'T')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

    if(newsrc.readI8() != 'E')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

    if(newsrc.readI8() != 'X')
        Sys_extError("read_tr4_level: '\\0TEX' not found");

    m_objectTextures.resize(newsrc.readU32());
    for(size_t i = 0; i < m_objectTextures.size(); i++)
        m_objectTextures[i] = tr4_object_texture_t::readTr4(newsrc);

    m_items.resize(newsrc.readU32());
    for(size_t i = 0; i < m_items.size(); i++)
        m_items[i] = tr2_item_t::readTr4(newsrc);

    m_aiObjects.resize(newsrc.readU32());
    for(size_t i = 0; i < m_aiObjects.size(); i++)
    {
        m_aiObjects[i].object_id = newsrc.readU16();
        m_aiObjects[i].room = newsrc.readU16();                        // 4

        m_aiObjects[i].x = newsrc.readI32();
        m_aiObjects[i].y = newsrc.readI32();
        m_aiObjects[i].z = newsrc.readI32();                            // 16

        m_aiObjects[i].ocb = newsrc.readU16();
        m_aiObjects[i].flags = newsrc.readU16();                       // 20
        m_aiObjects[i].angle = newsrc.readI32();                        // 24
    }

    m_demoData.resize(newsrc.readU16());
    for(size_t i = 0; i < m_demoData.size(); i++)
        m_demoData[i] = newsrc.readU8();

    // Soundmap
    m_soundmap.resize(TR_AUDIO_MAP_SIZE_TR4);
    for(size_t i = 0; i < m_soundmap.size(); i++)
        m_soundmap[i] = newsrc.readI16();

    m_soundDetails.resize(newsrc.readU32());
    for(size_t i = 0; i < m_soundDetails.size(); i++)
    {
        m_soundDetails[i].sample = newsrc.readU16();
        m_soundDetails[i].volume = (uint16_t)newsrc.readU8();        // n x 2.6
        m_soundDetails[i].sound_range = (uint16_t)newsrc.readU8();   // n as is
        m_soundDetails[i].chance = (uint16_t)newsrc.readU8();        // If n = 99, n = 0 (max. chance)
        m_soundDetails[i].pitch = (int16_t)newsrc.readI8();         // n as is
        m_soundDetails[i].num_samples_and_flags_1 = newsrc.readU8();
        m_soundDetails[i].flags_2 = newsrc.readU8();
    }

    // IMPORTANT NOTE: Sample indices ARE NOT USED in TR4 engine, but are parsed anyway.
    m_sampleIndices.resize(newsrc.readU32());
    for(size_t i = 0; i < m_sampleIndices.size(); i++)
        m_sampleIndices[i] = newsrc.readU32();

    // LOAD SAMPLES

    if(auto i = m_src.readU32())
    {
        m_samplesCount = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        m_samplesData.resize(m_src.tell() - m_src.size());
        m_src.readBytes(m_samplesData.data(), m_samplesData.size());
    }
}
