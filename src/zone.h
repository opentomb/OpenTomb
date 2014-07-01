/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef ZONE_H
#define ZONE_H

#define ID_ZFREE 0x00                                                           // блок свободен
#define ID_ZMAIN 0x01                                                           // основные (обычные) блоки
#define ID_ZCACHE 0x02                                                          // блоки под файловую систему
#define ID_ZHEADERS 0x04                                                        // блоки под заголовки файловой системы. время жизни == время жизни программы
#define ID_ZTEMP 0x08                                                           // блоки под временную память
#define ID_ZVIDEO 0x10                                                          // блоки под видео память

void Memory_Init (void *buf, size_t size);                                      // инициализация памяти

void Z_Free (void *ptr);                                                        // освобождение указателя
void Z_FreeTag (int tag);                                                       // освобождение всех указателей с ptr->tag == tag
void *Z_Malloc (size_t size);                                                   // возвращает указатель на блок памяти, заполненный нулями
size_t Z_GetSize (void *ptr);                                                   // возвращает размер, запрошенный пользователем при выделении памяти
void *Z_TagMalloc (size_t size, int tag);                                       // возвращает указатель на блок памяти с block->tag == tag
void *Z_Realloc (void *ptr, size_t newsize);                                    // перевыделяет память для блока ptr, сохраняя его содержимое

void Z_CheckHeap (void);

void Z_Report (void);

#endif

