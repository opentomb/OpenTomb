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

#include "level.h"
#include "tr1level.h"
#include "tr2level.h"
#include "tr3level.h"
#include "tr4level.h"
#include "tr5level.h"

#include <algorithm>

using namespace loader;

/// \brief reads the mesh data.
void Level::readMeshData(io::SDLReader& reader)
{
    uint32_t meshDataWords = reader.readU32();
    const auto basePos = reader.tell();

    const auto meshDataSize = meshDataWords * 2;
    reader.skip(meshDataSize);

    reader.readVector(m_meshIndices, reader.readU32());
    const auto endPos = reader.tell();

    m_meshes.clear();

    uint32_t meshDataPos = 0;
    for (size_t i = 0; i < m_meshIndices.size(); i++)
    {
        std::replace(m_meshIndices.begin(), m_meshIndices.end(), meshDataPos, i);

        reader.seek(basePos + meshDataPos);

        if (m_gameVersion >= Game::TR4)
            m_meshes.emplace_back( *Mesh::readTr4(reader) );
        else
            m_meshes.emplace_back( *Mesh::readTr1(reader) );

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
void Level::readFrameMoveableData(io::SDLReader& reader)
{
    m_frameData.resize(reader.readU32());
    const auto frameDataPos = reader.tell();
    reader.readVector(m_frameData, m_frameData.size());

    m_moveables.resize(reader.readU32() );
    for (size_t i = 0; i < m_moveables.size(); i++)
    {
        if(m_gameVersion < Game::TR5)
        {
            m_moveables[i] = Moveable::readTr1(reader);
            // Disable unused skybox polygons.
            if(m_gameVersion == Game::TR3 && m_moveables[i]->object_id == 355)
            {
                m_meshes[m_meshIndices[m_moveables[i]->starting_mesh]].coloured_triangles.resize(16);
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
            if (m_moveables[j]->frame_offset == pos)
            {
                m_moveables[j]->frame_index = static_cast<uint32_t>(i);
                m_moveables[j]->frame_offset = 0;
            }

        reader.seek(frameDataPos + pos);

        pos = 0;
        for(size_t j = 0; j < m_moveables.size(); j++)
        {
            if(m_moveables[j]->frame_offset > pos)
            {
                pos = m_moveables[j]->frame_offset;
                break;
            }
        }
    }
    reader.seek(endPos);
}

std::unique_ptr<Level> Level::createLoader(const std::string& filename, Game game_version)
{
    size_t len2 = 0;

    for(size_t i = 0; i < filename.length(); i++)
    {
        if(filename[i] == '/' || filename[i] == '\\')
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

    io::SDLReader reader(filename);
    if(!reader.isOpen())
        return nullptr;

    if(game_version == Game::Unknown)
        game_version = probeVersion(reader, filename);
    if(game_version == Game::Unknown)
        return nullptr;

    reader.seek(0);
    return createLoader(std::move(reader), game_version, sfxPath);
}

/** \brief reads the level.
  *
  * Takes a SDL_RWop and the game_version of the file and reads the structures into the members of TR_Level.
  */
std::unique_ptr<Level> Level::createLoader(io::SDLReader&& reader, Game game_version, const std::string& sfxPath)
{
    if (!reader.isOpen())
        return nullptr;

    std::unique_ptr<Level> result;

    switch (game_version)
    {
        case Game::TR1:
            result.reset(new TR1Level(game_version, std::move(reader)));
            break;
        case Game::TR1Demo:
        case Game::TR1UnfinishedBusiness:
            result.reset(new TR1Level(game_version, std::move(reader)));
            result->m_demoOrUb = true;
            break;
        case Game::TR2:
            result.reset(new TR2Level(game_version, std::move(reader)));
            break;
        case Game::TR2Demo:
            result.reset(new TR2Level(game_version, std::move(reader)));
            result->m_demoOrUb = true;
            break;
        case Game::TR3:
            result.reset(new TR3Level(game_version, std::move(reader)));
            break;
        case Game::TR4:
        case Game::TR4Demo:
            result.reset(new TR4Level(game_version, std::move(reader)));
            break;
        case Game::TR5:
            result.reset(new TR5Level(game_version, std::move(reader)));
            break;
        default:
            BOOST_THROW_EXCEPTION( std::runtime_error("Invalid game version") );
    }

    result->m_sfxPath = sfxPath;
    return result;
}

Game Level::probeVersion(io::SDLReader& reader, const std::string& filename)
{
    if(!reader.isOpen() || filename.length()<5)
        return Game::Unknown;

    std::string ext;
    ext += filename[filename.length()-4];
    ext += toupper(filename[filename.length()-3]);
    ext += toupper(filename[filename.length()-2]);
    ext += toupper(filename[filename.length()-1]);

    reader.seek(0);
    uint8_t check[4];
    reader.readBytes(check, 4);

    Game ret = Game::Unknown;
    if(ext == ".PHD")
    {
        if(check[0] == 0x20 &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = Game::TR1;
        }
    }
    else if(ext == ".TUB")
    {
        if(check[0] == 0x20 &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = loader::Game::TR1UnfinishedBusiness;
        }
    }
    else if(ext == ".TR2")
    {
        if(check[0] == 0x2D &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = loader::Game::TR2;
        }
        else if((check[0] == 0x38 || check[0] == 0x34) &&
                check[1] == 0x00 &&
                (check[2] == 0x18 || check[2] == 0x08) &&
                check[3] == 0xFF)
        {
            ret = loader::Game::TR3;
        }
    }
    else if(ext == ".TR4")
    {
        if(check[0] == 0x54 &&                                         // T
           check[1] == 0x52 &&                                         // R
           check[2] == 0x34 &&                                         // 4
           check[3] == 0x00)
        {
            ret = loader::Game::TR4;
        }
        else if(check[0] == 0x54 &&                                         // T
                check[1] == 0x52 &&                                         // R
                check[2] == 0x34 &&                                         // 4
                check[3] == 0x63)                                           //
        {
            ret = loader::Game::TR4;
        }
        else if(check[0] == 0xF0 &&                                         // T
                check[1] == 0xFF &&                                         // R
                check[2] == 0xFF &&                                         // 4
                check[3] == 0xFF)
        {
            ret = loader::Game::TR4;
        }
    }
    else if(ext == ".TRC")
    {
        if(check[0] == 0x54 &&                                              // T
           check[1] == 0x52 &&                                              // R
           check[2] == 0x34 &&                                              // C
           check[3] == 0x00)
        {
            ret = loader::Game::TR5;
        }
    }

    return ret;
}

StaticMesh *Level::findStaticMeshById(uint32_t object_id)
{
    for (size_t i = 0; i < m_staticMeshes.size(); i++)
        if (m_staticMeshes[i].object_id == object_id && m_meshIndices[m_staticMeshes[i].mesh])
            return &m_staticMeshes[i];

    return nullptr;
}

Item *Level::fineItemById(int32_t object_id)
{
    for (size_t i = 0; i < m_items.size(); i++)
        if (m_items[i].object_id == object_id)
            return &m_items[i];

    return nullptr;
}

Moveable *Level::findMoveableById(uint32_t object_id)
{
    for (size_t i = 0; i < m_moveables.size(); i++)
        if (m_moveables[i]->object_id == object_id)
            return m_moveables[i].get();

    return nullptr;
}

void Level::convertTexture(ByteTexture & tex, Palette & pal, DWordTexture & dst)
{
    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            int col = tex.pixels[y][x];

            if (col > 0)
                dst.pixels[y][x] = static_cast<int>(pal.color[col].r) | (static_cast<int>(pal.color[col].g) << 8) | (static_cast<int>(pal.color[col].b) << 16) | (0xff << 24);
            else
                dst.pixels[y][x] = 0x00000000;
        }
    }
}

void Level::convertTexture(WordTexture & tex, DWordTexture & dst)
{
    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            int col = tex.pixels[y][x];

            if (col & 0x8000)
                dst.pixels[y][x] = ((col & 0x00007c00) >> 7) | (((col & 0x000003e0) >> 2) << 8) | (((col & 0x0000001f) << 3) << 16) | 0xff000000;
            else
                dst.pixels[y][x] = 0x00000000;
        }
    }
}
