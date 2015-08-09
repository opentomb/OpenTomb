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

        for(size_t i = 0; i < (m_numTextiles - m_numMiscTextiles); i++)
            m_textile32[i] = tr4_textile32_t::read(newsrc);
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

        for(size_t i = (m_numTextiles - m_numMiscTextiles); i < m_numTextiles; i++)
            m_textile32[i] = tr4_textile32_t::read(newsrc);
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

    m_rooms.resize(m_src.readU32());
    for(size_t i = 0; i < m_rooms.size(); i++)
        m_rooms[i] = tr5_room_t::readTr5(m_src);

    m_floorData.resize(m_src.readU32());
    for(size_t i = 0; i < m_floorData.size(); i++)
        m_floorData[i] = m_src.readU16();

    read_mesh_data(m_src);

    m_animations.resize(m_src.readU32());
    for(size_t i = 0; i < m_animations.size(); i++)
    {
        m_animations[i] = tr_animation_t::readTr4(m_src);
    }

    m_stateChanges.resize(m_src.readU32());
    for(size_t i = 0; i < m_stateChanges.size(); i++)
        m_stateChanges[i] = tr_state_change_t::read(m_src);

    m_animDispatches.resize(m_src.readU32());
    for(size_t i = 0; i < m_animDispatches.size(); i++)
        m_animDispatches[i] = tr_anim_dispatch_t::read(m_src);

    m_animCommands.resize(m_src.readU32());
    for(size_t i = 0; i < m_animCommands.size(); i++)
        m_animCommands[i] = m_src.readI16();

    m_meshTreeData.resize(m_src.readU32());
    for(size_t i = 0; i < m_meshTreeData.size(); i++)
        m_meshTreeData[i] = m_src.readU32();                     // 4 bytes

    read_frame_moveable_data(m_src);

    m_staticMeshes.resize(m_src.readU32());
    for(size_t i = 0; i < m_staticMeshes.size(); i++)
        m_staticMeshes[i] = tr_staticmesh_t::read(m_src);

    if(m_src.readI8() != 'S')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if(m_src.readI8() != 'P')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if(m_src.readI8() != 'R')
        Sys_extError("read_tr5_level: 'SPR' not found");

    if(m_src.readI8() != 0)
        Sys_extError("read_tr5_level: 'SPR' not found");

    m_spriteTextures.resize(m_src.readU32());
    for(size_t i = 0; i < m_spriteTextures.size(); i++)
        m_spriteTextures[i] = tr_sprite_texture_t::readTr4(m_src);

    m_spriteSequences.resize(m_src.readU32());
    for(size_t i = 0; i < m_spriteSequences.size(); i++)
        m_spriteSequences[i] = tr_sprite_sequence_t::read(m_src);

    m_cameras.resize(m_src.readU32());
    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        m_cameras[i].x = m_src.readI32();
        m_cameras[i].y = m_src.readI32();
        m_cameras[i].z = m_src.readI32();

        m_cameras[i].room = m_src.readI16();
        m_cameras[i].unknown1 = m_src.readU16();
    }

    m_flybyCameras.resize(m_src.readU32());
    for(size_t i = 0; i < m_flybyCameras.size(); i++)
    {
        m_flybyCameras[i].cam_x = m_src.readI32();
        m_flybyCameras[i].cam_y = m_src.readI32();
        m_flybyCameras[i].cam_z = m_src.readI32();
        m_flybyCameras[i].target_x = m_src.readI32();
        m_flybyCameras[i].target_y = m_src.readI32();
        m_flybyCameras[i].target_z = m_src.readI32();

        m_flybyCameras[i].sequence = m_src.readI8();
        m_flybyCameras[i].index = m_src.readI8();

        m_flybyCameras[i].fov = m_src.readU16();
        m_flybyCameras[i].roll = m_src.readU16();
        m_flybyCameras[i].timer = m_src.readU16();
        m_flybyCameras[i].speed = m_src.readU16();
        m_flybyCameras[i].flags = m_src.readU16();

        m_flybyCameras[i].room_id = m_src.readU32();
    }

    m_soundSources.resize(m_src.readU32());
    for(size_t i = 0; i < m_soundSources.size(); i++)
    {
        m_soundSources[i].x = m_src.readI32();
        m_soundSources[i].y = m_src.readI32();
        m_soundSources[i].z = m_src.readI32();

        m_soundSources[i].sound_id = m_src.readU16();
        m_soundSources[i].flags = m_src.readU16();
    }

    m_boxes.resize(m_src.readU32());
    for(size_t i = 0; i < m_boxes.size(); i++)
        m_boxes[i] = tr_box_t::readTr2(m_src);

    m_overlaps.resize(m_src.readU32());
    for(size_t i = 0; i < m_overlaps.size(); i++)
        m_overlaps[i] = m_src.readU16();

    // Zones
    m_src.skip(m_boxes.size() * 20);

    m_animatedTextures.resize(m_src.readU32());
    for(size_t i = 0; i < m_animatedTextures.size(); i++)
    {
        m_animatedTextures[i] = m_src.readU16();
    }

    m_animatedTexturesUvCount = m_src.readU8();

    if(m_src.readI8() != 'T')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if(m_src.readI8() != 'E')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if(m_src.readI8() != 'X')
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    if(m_src.readI8() != 0)
        Sys_extError("read_tr5_level: '\\0TEX' not found");

    m_objectTextures.resize(m_src.readU32());
    for(size_t i = 0; i < m_objectTextures.size(); i++)
    {
        m_objectTextures[i] = tr4_object_texture_t::readTr4(m_src);
        if(m_src.readU16() != 0)
            Sys_extWarn("read_tr5_level: obj_tex trailing bitu16 != 0");
    }

    m_items.resize(m_src.readU32());
    for(size_t i = 0; i < m_items.size(); i++)
        m_items[i] = tr2_item_t::readTr4(m_src);

    m_aiObjects.resize(m_src.readU32());
    for(size_t i = 0; i < m_aiObjects.size(); i++)
    {
        m_aiObjects[i].object_id = m_src.readU16();
        m_aiObjects[i].room = m_src.readU16();

        m_aiObjects[i].x = m_src.readI32();
        m_aiObjects[i].y = m_src.readI32();
        m_aiObjects[i].z = m_src.readI32();                            // 16

        m_aiObjects[i].ocb = m_src.readU16();
        m_aiObjects[i].flags = m_src.readU16();                       // 20
        m_aiObjects[i].angle = m_src.readI32();                        // 24
    }

    m_demoData.resize(m_src.readU16());
    for(size_t i = 0; i < m_demoData.size(); i++)
        m_demoData[i] = m_src.readU8();

    // Soundmap
    m_soundmap.resize(TR_AUDIO_MAP_SIZE_TR5);
    for(size_t i = 0; i < m_soundmap.size(); i++)
        m_soundmap[i] = m_src.readI16();

    m_soundDetails.resize(m_src.readU32());
    for(size_t i = 0; i < m_soundDetails.size(); i++)
    {
        m_soundDetails[i].sample = m_src.readU16();
        m_soundDetails[i].volume = (uint16_t)m_src.readU8();        // n x 2.6
        m_soundDetails[i].sound_range = (uint16_t)m_src.readU8();   // n as is
        m_soundDetails[i].chance = (uint16_t)m_src.readU8();        // If n = 99, n = 0 (max. chance)
        m_soundDetails[i].pitch = (int16_t)m_src.readI8();           // n as is
        m_soundDetails[i].num_samples_and_flags_1 = m_src.readU8();
        m_soundDetails[i].flags_2 = m_src.readU8();
    }

    m_sampleIndices.resize(m_src.readU32());
    for(size_t i = 0; i < m_sampleIndices.size(); i++)
        m_sampleIndices[i] = m_src.readU32();

    m_src.skip(6);   // In TR5, sample indices are followed by 6 0xCD bytes. - correct - really 0xCDCDCDCDCDCD

    // LOAD SAMPLES
    if(auto i = m_src.readU32())
    {
        m_samplesCount = i;
        // Since sample data is the last part, we simply load whole last
        // block of file as single array.
        m_samplesData.resize(m_src.size() - m_src.tell());
        m_src.readBytes(m_samplesData.data(), m_samplesData.size());
    }
}