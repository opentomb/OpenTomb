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
#include <string.h>

#include "l_main.h"
#include "../core/system.h"

#define RCSID "$Id: l_main.cpp,v 1.10 2002/09/20 15:59:02 crow Exp $"

/// \brief reads the mesh data.
void TR_Level::read_mesh_data(SDL_RWops * const src)
{
    uint8_t *buffer;
    SDL_RWops *newsrc = NULL;
    uint32_t size;
    uint32_t pos = 0;
    int mesh = 0;
    uint32_t i;
    uint32_t num_mesh_data;

    num_mesh_data = read_bitu32(src);

    size = num_mesh_data * 2;
    buffer = new uint8_t[size];

    if (SDL_RWread(src, buffer, 1, size) < size)
        Sys_extError("read_tr_mesh_data: SDL_RWread(buffer)");

    if ((newsrc = SDL_RWFromMem(buffer, size)) == NULL)
        Sys_extError("read_tr_mesh_data: SDL_RWFromMem");

    this->mesh_indices_count = read_bitu32(src);
    this->mesh_indices = (uint32_t*)malloc(this->mesh_indices_count * sizeof(uint32_t));
    for (i = 0; i < this->mesh_indices_count; i++)
        this->mesh_indices[i] = read_bitu32(src);

    this->meshes_count = this->mesh_indices_count;
    this->meshes = (tr4_mesh_t*)calloc(this->meshes_count, sizeof(tr4_mesh_t));

    for (i = 0; i < this->mesh_indices_count; i++)
    {
        uint32_t j;

        for (j = 0; j < this->mesh_indices_count; j++)
            if (this->mesh_indices[j] == pos)
                this->mesh_indices[j] = mesh;

        SDL_RWseek(newsrc, pos, RW_SEEK_SET);

        if (this->game_version >= TR_IV)
            read_tr4_mesh(newsrc, this->meshes[mesh]);
        else
            read_tr_mesh(newsrc, this->meshes[mesh]);

        mesh++;

        for (j = 0; j < this->mesh_indices_count; j++)
            if (this->mesh_indices[j] > pos)
            {
                pos = this->mesh_indices[j];
                break;
            }
    }
    SDL_RWclose(newsrc);
    newsrc = NULL;
    delete [] buffer;
}

/// \brief reads frame and moveable data.
void TR_Level::read_frame_moveable_data(SDL_RWops * const src)
{
    uint32_t i;
    SDL_RWops *newsrc = NULL;
    uint32_t pos = 0;
    uint32_t frame = 0;

    this->frame_data_size = read_bitu32(src);
    this->frame_data = (uint16_t*)malloc(this->frame_data_size * sizeof(uint16_t));

    if (SDL_RWread(src, this->frame_data, sizeof(uint16_t), this->frame_data_size) < frame_data_size)
        Sys_extError("read_tr_level: frame_data: SDL_RWread(buffer)");

    if ((newsrc = SDL_RWFromMem(this->frame_data, this->frame_data_size)) == NULL)
        Sys_extError("read_tr_level: frame_data: SDL_RWFromMem");

    this->moveables_count = read_bitu32(src);
    this->moveables = (tr_moveable_t*)calloc(this->moveables_count, sizeof(tr_moveable_t));
    for (i = 0; i < this->moveables_count; i++)
    {
        if (this->game_version < TR_V)
            read_tr_moveable(src, this->moveables[i]);
        else
            read_tr5_moveable(src, this->moveables[i]);
    }

    for (i = 0; i < this->moveables_count; i++)
    {
        uint32_t j;

        for (j = 0; j < this->moveables_count; j++)
            if (this->moveables[j].frame_offset == pos)
            {
                this->moveables[j].frame_index = frame;
                this->moveables[j].frame_offset = 0;
            }

        SDL_RWseek(newsrc, pos, RW_SEEK_SET);

        frame++;

        pos = 0;
        for (j = 0; j < this->moveables_count; j++)
            if (this->moveables[j].frame_offset > pos)
            {
                pos = this->moveables[j].frame_offset;
                break;
            }
    }

    SDL_RWclose(newsrc);
    newsrc = NULL;
}

void TR_Level::read_level(const char *filename, int32_t game_version)
{
    int len, i, len2;
    SDL_RWops *src = SDL_RWFromFile(filename, "rb");

    if(src == NULL)
    {
        return;
    }

    len = strlen(filename);
    len2 = 0;
    for(i = 0; i < len; i++)
    {
        if((filename[i] == '/') || (filename[i] == '\\'))
        {
            len2 = i;
        }
    }

    if((len2 > 0) && (len2 < 256))
    {
        memcpy(this->sfx_path, filename, len2 + 1);
        this->sfx_path[len2+1] = 0;
        strncat(this->sfx_path, "MAIN.SFX", 256);
    }

    this->read_level(src, game_version);
    SDL_RWclose(src);
}

/** \brief reads the level.
  *
  * Takes a SDL_RWop and the game_version of the file and reads the structures into the members of TR_Level.
  */
void TR_Level::read_level(SDL_RWops * const src, int32_t game_version)
{
    if (!src)
        Sys_extError("Invalid SDL_RWops");

    this->game_version = game_version;

    switch (game_version)
    {
        case TR_I:
            read_tr_level(src, 0);
            break;
        case TR_I_DEMO:
        case TR_I_UB:
            read_tr_level(src, 1);
            break;
        case TR_II:
            read_tr2_level(src, 0);
            break;
        case TR_II_DEMO:
            read_tr2_level(src, 1);
            break;
        case TR_III:
            read_tr3_level(src);
            break;
        case TR_IV:
        case TR_IV_DEMO:
            read_tr4_level(src);
            break;
        case TR_V:
            read_tr5_level(src);
            break;
        default:
            Sys_extError("Invalid game version");
            break;
    }
}

tr_mesh_thee_tag_t TR_Level::get_mesh_tree_tag_for_model(tr_moveable_t *model, int index)
{
    tr_mesh_thee_tag_t ret;
    
    ret.dx = 0.0f;
    ret.dy = 0.0f;
    ret.dz = 0.0f;
    ret.flag_data = 0x02;
    
    if((index > 0) && (index < model->num_meshes))
    {
        uint32_t *tr_mesh_tree = mesh_tree_data + model->mesh_tree_index + (index - 1) * 4;
        ret.flag_data = tr_mesh_tree[0];
        ret.dx = (float)((int32_t)tr_mesh_tree[1]);
        ret.dy = (float)((int32_t)tr_mesh_tree[2]);
        ret.dz = (float)((int32_t)tr_mesh_tree[3]);
    }
    
    return ret;
}

void TR_Level::get_anim_frame_data(tr5_vertex_t min_max_pos[3], tr5_vertex_t *rotations, int meshes_count, tr_animation_t *anim, int frame)
{
    uint16_t temp1, temp2;
    uint16_t offset = (game_version < TR_II) ? (0x0A) : (0x09);
    uint32_t frame_offset = (anim->frame_offset / 2) + (uint32_t)anim->frame_size * frame;
    float ang;
    
    if(frame_offset >= frame_data_size)
    {
        min_max_pos[0].x = 0.0f;
        min_max_pos[0].y = 0.0f;
        min_max_pos[0].z = 0.0f;

        min_max_pos[1].x = 0.0f;
        min_max_pos[1].y = 0.0f;
        min_max_pos[1].z = 0.0f;

        min_max_pos[2].x = 0.0f;
        min_max_pos[2].y = 0.0f;
        min_max_pos[2].z = 0.0f;
        
        for(int mesh_it = 0; mesh_it < meshes_count; ++mesh_it, ++rotations)
        {
            rotations->x = 0.0f;
            rotations->y = 0.0f;
            rotations->z = 0.0f;
        }
        return;
    }
    
    /*
     * - first 9 words are bounding box and frame offset coordinates.
     * - 10's word is a rotations count, must be equal to number of meshes in model.
     *   BUT! only in TR1. In TR2 - TR5 after first 9 words begins next section.
     * - in the next follows rotation's data. one word - one rotation, if rotation is one-axis (one angle).
     *   two words in 3-axis rotations (3 angles). angles are calculated with bit mask.
     */
    
    {
        uint16_t *frame = frame_data + frame_offset;
        min_max_pos[0].x = (short int)frame[0];
        min_max_pos[0].y = (short int)frame[2];
        min_max_pos[0].z = (short int)frame[4];

        min_max_pos[1].x = (short int)frame[1];
        min_max_pos[1].y = (short int)frame[3];
        min_max_pos[1].z = (short int)frame[5];

        min_max_pos[2].x = (short int)frame[6];
        min_max_pos[2].y = (short int)frame[7];
        min_max_pos[2].z = (short int)frame[8];
    }
    
    for(int mesh_it = 0; mesh_it < meshes_count; ++mesh_it, ++rotations)
    {
        switch(game_version)
        {
            case TR_I:
            case TR_I_UB:
            case TR_I_DEMO:
                temp2 = frame_data[frame_offset + offset++];
                temp1 = frame_data[frame_offset + offset++];
                rotations->x = (float)((temp1 & 0x3ff0) >> 4);
                rotations->y = (float)(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                rotations->z = (float)(temp2 & 0x03ff);
                rotations->x *= 360.0f / 1024.0f;
                rotations->y *= 360.0f / 1024.0f;
                rotations->z *= 360.0f / 1024.0f;
                break;

            default: /* TR_II + */
                temp1 = frame_data[frame_offset + offset++];
                if(game_version >= TR_IV)
                {
                    ang = (float)(temp1 & 0x0fff);
                    ang *= 360.0f / 4096.0f;
                }
                else
                {
                    ang = (float)(temp1 & 0x03ff);
                    ang *= 360.0f / 1024.0f;
                }

                switch (temp1 & 0xc000)
                {
                    case 0x4000:    // x only
                        rotations->x = ang;
                        rotations->y = 0.0f;
                        rotations->z = 0.0f;
                        break;

                    case 0x8000:    // y only
                        rotations->x = 0.0f;
                        rotations->y = ang;
                        rotations->z = 0.0f;
                        break;

                    case 0xc000:    // z only
                        rotations->x = 0.0f;
                        rotations->y = 0.0f;
                        rotations->z = ang;
                        break;

                    default:        // all three
                        temp2 = frame_data[frame_offset + offset++];
                        rotations->x = (float)((temp1 & 0x3ff0) >> 4);
                        rotations->y = (float)(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
                        rotations->z = (float)(temp2 & 0x03ff);
                        rotations->x *= 360.0f / 1024.0f;
                        rotations->y *= 360.0f / 1024.0f;
                        rotations->z *= 360.0f / 1024.0f;
                        break;
                };
                break;
        };
    }
}
