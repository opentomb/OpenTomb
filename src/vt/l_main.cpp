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
#include <algorithm>

#include "l_main.h"
#include "../system.h"

using namespace loader;

/// \brief reads the mesh data.
void TR_Level::read_mesh_data(io::SDLReader& reader)
{
    uint32_t meshDataWords = reader.readU32();
    const auto basePos = reader.tell();

    const auto meshDataSize = meshDataWords * 2;
    reader.skip(meshDataSize);

    reader.readVector(m_meshIndices, reader.readU32());
    const auto endPos = reader.tell();

    m_meshes.clear();

    uint32_t meshDataPos = 0;
    for (uint32_t i = 0; i < m_meshIndices.size(); i++)
    {
        std::replace(m_meshIndices.begin(), m_meshIndices.end(), meshDataPos, i);

        reader.seek(basePos + meshDataPos);

        if (m_gameVersion >= TR_IV)
            m_meshes.emplace_back( Mesh::readTr4(reader) );
        else
            m_meshes.emplace_back( Mesh::readTr1(reader) );

        for(size_t j = 0; j < m_meshIndices.size(); j++)
        {
            if(m_meshIndices[j] > meshDataPos)
            {
                meshDataPos = m_meshIndices[j];
                break;
            }
        }
    }

    reader.seek(endPos);
}

/// \brief reads frame and moveable data.
void TR_Level::read_frame_moveable_data(io::SDLReader& reader)
{
    m_frameData.resize(reader.readU32());
    const auto frameDataPos = reader.tell();
    reader.readVector(m_frameData, m_frameData.size());

    m_moveables.resize(reader.readU32() );
    for (size_t i = 0; i < m_moveables.size(); i++)
    {
        if(m_gameVersion < TR_V)
        {
            m_moveables[i] = Moveable::readTr1(reader);
            // Disable unused skybox polygons.
            if((m_gameVersion == TR_III) && (m_moveables[i].object_id == 355))
            {
                m_meshes[m_meshIndices[m_moveables[i].starting_mesh]].coloured_triangles.resize(16);
            }
        }
        else
        {
            m_moveables[i] = Moveable::readTr5(reader);
        }
    }

    const auto endPos = reader.tell();

    uint32_t pos = 0;
    for (size_t i = 0; i < m_frameData.size(); i++)
    {
        for (size_t j = 0; j < m_moveables.size(); j++)
            if (m_moveables[j].frame_offset == pos)
            {
                m_moveables[j].frame_index = i;
                m_moveables[j].frame_offset = 0;
            }

        reader.seek(frameDataPos + pos);

        pos = 0;
        for(size_t j = 0; j < m_moveables.size(); j++)
        {
            if(m_moveables[j].frame_offset > pos)
            {
                pos = m_moveables[j].frame_offset;
                break;
            }
        }
    }
    reader.seek(endPos);
}

std::unique_ptr<TR_Level> TR_Level::createLoader(const std::string& filename, int32_t game_version)
{
    int len2 = 0;

    for(size_t i = 0; i < filename.length(); i++)
    {
        if((filename[i] == '/') || (filename[i] == '\\'))
        {
            len2 = i;
        }
    }

    std::string sfxPath;

    if(len2 > 0)
    {
        sfxPath = filename.substr(0, len2 + 1) + "MAIN.SFX";
    }
    else
    {
        sfxPath = "MAIN.SFX";
    }

    return createLoader(SDL_RWFromFile(filename.c_str(), "rb"), game_version, sfxPath);
}

/** \brief reads the level.
  *
  * Takes a SDL_RWop and the game_version of the file and reads the structures into the members of TR_Level.
  */
std::unique_ptr<TR_Level> TR_Level::createLoader(SDL_RWops * const src, int32_t game_version, const std::string& sfxPath)
{
    if (!src)
        Sys_extError("Invalid SDL_RWops");

    std::unique_ptr<TR_Level> result;

    switch (game_version)
    {
        case TR_I:
            result.reset(new TR_Level(game_version, src));
            break;
        case TR_I_DEMO:
        case TR_I_UB:
            result.reset(new TR_Level(game_version, src));
            result->m_demoOrUb = true;
            break;
        case TR_II:
            result.reset(new TR_TR2Level(game_version, src));
            break;
        case TR_II_DEMO:
            result.reset(new TR_TR2Level(game_version, src));
            result->m_demoOrUb = true;
            break;
        case TR_III:
            result.reset(new TR_TR3Level(game_version, src));
            break;
        case TR_IV:
        case TR_IV_DEMO:
            result.reset(new TR_TR4Level(game_version, src));
            break;
        case TR_V:
            result.reset(new TR_TR5Level(game_version, src));
            break;
        default:
            Sys_extError("Invalid game version");
            break;
    }


    result->m_sfxPath = sfxPath;
    return result;
}
