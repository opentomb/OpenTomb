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

#include <assert.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#include "l_main.h"
#include "../system.h"

/** \brief reads signed 8-bit value.
  *
  * uses current position from src. throws TR_ReadError when not successful.
  */

int8_t TR_Level::read_bit8(SDL_RWops * const src)
{
    int8_t data;

    if (src == NULL)
        Sys_extError("read_bit8: src == NULL");

    if (SDL_RWread(src, &data, 1, 1) < 1)
        Sys_extError("read_bit8");

    return data;
}

/** \brief reads unsigned 8-bit value.
  *
  * uses current position from src. throws TR_ReadError when not successful.
  */
uint8_t TR_Level::read_bitu8(SDL_RWops * const src)
{
    uint8_t data;

    if (src == NULL)
        Sys_extError("read_bitu8: src == NULL");

    if (SDL_RWread(src, &data, 1, 1) < 1)
        Sys_extError("read_bitu8");

    return data;
}

/** \brief reads signed 16-bit value.
  *
  * uses current position from src. does endian correction. throws TR_ReadError when not successful.
  */
int16_t TR_Level::read_bit16(SDL_RWops * const src)
{
    int16_t data;

    if (src == NULL)
        Sys_extError("read_bit16: src == NULL");

    if (SDL_RWread(src, &data, 2, 1) < 1)
        Sys_extError("read_bit16");

    data = SDL_SwapLE16(data);

    return data;
}

/** \brief reads unsigned 16-bit value.
  *
  * uses current position from src. does endian correction. throws TR_ReadError when not successful.
  */
uint16_t TR_Level::read_bitu16(SDL_RWops * const src)
{
    uint16_t data;

    if (src == NULL)
        Sys_extError("read_bitu16: src == NULL");

    if (SDL_RWread(src, &data, 2, 1) < 1)
        Sys_extError("read_bitu16");

    data = SDL_SwapLE16(data);

    return data;
}

/** \brief reads signed 32-bit value.
  *
  * uses current position from src. does endian correction. throws TR_ReadError when not successful.
  */
int32_t TR_Level::read_bit32(SDL_RWops * const src)
{
    int32_t data;

    if (src == NULL)
        Sys_extError("read_bit32: src == NULL");

    if (SDL_RWread(src, &data, 4, 1) < 1)
        Sys_extError("read_bit32");

    data = SDL_SwapLE32(data);

    return data;
}

/** \brief reads unsigned 32-bit value.
  *
  * uses current position from src. does endian correction. throws TR_ReadError when not successful.
  */
uint32_t TR_Level::read_bitu32(SDL_RWops * const src)
{
    uint32_t data;

    if (src == NULL)
        Sys_extError("read_bitu32: src == NULL");

    if (SDL_RWread(src, &data, 4, 1) < 1)
        Sys_extError("read_bitu32");

    data = SDL_SwapLE32(data);

    return data;
}

/** \brief reads float value.
  *
  * uses current position from src. does endian correction. throws TR_ReadError when not successful.
  */
float TR_Level::read_float(SDL_RWops * const src)
{
    float data;

    if (src == NULL)
        Sys_extError("read_float: src == NULL");

    if (SDL_RWread(src, &data, 4, 1) < 1)
        Sys_extError("read_float");

    data = SDL_SwapLE32(data);

    return data;
}
