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

 * Modyfied by TeslaRus
*/
// Z_zone.c

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef unsigned char         byte;

#include "system.h"
#include "console.h"
#include "zone.h"

#define ZONEID          0x1d4a11
#define MINFRAGMENT     64

//TODO: добавить выравнивание структуры
typedef struct memblock_s
{
    size_t              size;                                               // размер, включая заголовок и по возможности мелкие фрагменты
    size_t              user_size;                                          // размер, запрошенный пользователем
    int                 tag;                                                // tag = 0 если блок свободен
    int                 id;                                                 // должно быть равно ZONEID
    struct memblock_s   *next;                                              // указатель на следующий блок
    struct memblock_s   *prev;                                              // указатель на предыдущий блок
} memblock_t, *memblock_p;

typedef struct
{
    size_t              size;                                               // total bytes malloced, including header
    memblock_t          blocklist;                                          // start / end cap for linked list
    memblock_t          *rover;                                             // current block
} memzone_t, *memzone_p;

/*
==============================================================================

            ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/

memzone_t *mainzone;

void Z_ClearZone (memzone_t *zone, size_t size);


/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone (memzone_t *zone, size_t size)                                 // уничтожаются все блоки, список обнуляется. теперь вся куча свободна.
{
    memblock_t *block;                                                          // новый добавляемый блок

// set the entire zone to one free block

    zone->blocklist.next = zone->blocklist.prev = block = (memblock_t *)( (byte *)zone + sizeof(memzone_t) );             // присваиваем указателям адрес памяти после хидера (zone + memzone_type_size)
    zone->blocklist.tag = 1;                                                    // in use block (заголовок списка не может быть свободен для выделения памяти)
    zone->blocklist.id = 0;                                                     // логично присвоить заголовку ID = 0
    zone->blocklist.size = 0;                                                   // заголовку не нужна память под выделение, поэтому ее размер - 0
    zone->rover = block;                                                        // ставим текущий (последний) блок на единственный свободный блок
    zone->size = size;                                                          // размер зоны, включая заголовок

    block->prev = block->next = &zone->blocklist;                               // список zone->blocklist указывает на block, block на zone->blocklist обоими ссылками
    block->tag = 0;                                                             // единственный свободный блок свободен для выделения в нем памяти
    block->id = ZONEID;                                                         // ID всех блоков памяти должен быть одинаков и равен определенной константе,
                                                                                // для проверки при освобождении памяти (т.е. нельзя освобождать левые адреса памяти)
    block->size = size - sizeof(memzone_t);                                     // размер свободной памяти под выделение = общий размер минус размер заголовка.
    block->user_size = 0;                                                       // у свободного блока usersize = 0
}


/*
========================
Z_Free
========================
*/
void Z_Free (void *ptr)
{
    memblock_t *block, *other;

    if (!ptr)
    {
        Sys_Error ("Z_Free: NULL pointer");
    }

    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
    if (block->id != ZONEID)
    {
        Sys_Error ("Z_Free: freed a pointer without ZONEID");
    }
    if (block->tag == 0)
    {
        Sys_Error ("Z_Free: freed a freed pointer");
    }

    block->tag = 0;                                                             // обозначить блок как свободный

    other = block->prev;                                                        // other = начало предыдущего блока
    if (!other->tag)                                                            // если предыдущий блок свободен,
    {                                                                           // то объединить с текущим
        other->size += block->size;                                             // добавить к размеру предыдущего блока размер текущего освобождаемого.
        other->next = block->next;
        other->next->prev = other;                                              // теперь данный освобожденный блок объединен с прилегающим пустым
        if (block == mainzone->rover)                                           // если освобождаемый блок является последним, то сделать последним предыдущий объединенный
        {
            mainzone->rover = other;
        }
        block = other;
    }

    other = block->next;                                                        // other - следующий за освобожденным блок.
    if (!other->tag)                                                            // если он свободен, то объединим
    {
        block->size += other->size;                                             // увеличить размер блока на тпкущий освобождаемый
        block->next = other->next;                                              // объединяем свободные блоки
        block->next->prev = block;                                              // объединяем свободные блоки
        if (other == mainzone->rover)
        {
            mainzone->rover = block;                                            // если освобождаемый блок является последним, то сделать последним предыдущий объединенный
        }
    }
}

void Z_FreeTag (int tag)
{
    memblock_t *head, *p, *p_temp;

    head = &mainzone->blocklist;

    if (tag == 0)
    {
        Sys_Error ("Z_Free: freed a freed pointer");
    }

    p = head->next;

    while (p != head)                                                           // пока не пройдем весь список
    {
        if (p->tag == tag)                                                      // нашли нужный tag
        {
            p_temp = p;
            if(p->next->tag == 0)                                               // если область после найденного указателя свободна, то она будет объединена с текущей
            {                                                                   // поэтому ее следует пропустить во избежание коллизий
                p = p->next;
            }
            p = p->next;
            Z_Free((void *)((byte *)p_temp + sizeof(memblock_t)));
        }
        else
        {
            p = p->next;
        }
    }
}

size_t Z_GetSize (void *ptr)
{
    memblock_t *block;

    if (!ptr)
    {
        return 0;
    }

    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));

    if(block->id != ZONEID)
    {
        Sys_extError("Z_GetSize: ptr is not ZONEID memory!\n");
    }

    return block->user_size;
}

/*
========================
Z_Malloc
========================
*/
void *Z_Malloc (size_t size)
{
    void *buf;
#ifdef PARANOID
    Z_CheckHeap ();                                                             // DEBUG
#endif
    buf = Z_TagMalloc (size, ID_ZMAIN);                                         // выделяем память с установкой флага ID_ZMAIN

    if(!buf)
    {
        Z_FreeTag(ID_ZTEMP);
        buf = Z_TagMalloc (size, ID_ZMAIN);                                     // выделяем память с установкой флага ID_ZMAIN
    }

    if (!buf)
    {
        Sys_Error ("Z_Malloc: failed on allocation of %i bytes",size);          // если не удалось выделить память, то закрываемся и выходим
    }
    memset (buf, 0, size);                                                      // обнуляем содержимое выделенной памяти

    return buf;                                                                 // возвращаем выделенный блок
}

void *Z_TagMalloc (size_t size, int tag)
{
    size_t extra;                                                               // размер выданного блока минус запрашиваемый размер
    size_t usersize = size;
    memblock_t *start, *rover, *new_block, *base;

    if (!tag)                                                                   // если флаг 0 (т.е. обозначение свободного блока) то
    {
        Sys_Error ("Z_TagMalloc: tried to use a 0 tag");                        // выходим
    }

//
// scan through the block list looking for the first free block
// of sufficient size
//
    size += sizeof(memblock_t);                                                 // account for size of block header
    //size += 4;                                                                // space for memory trash tester
    size = (size + 7) & ~7;                                                     // align to 8-byte boundary

    base = rover = mainzone->rover;                                             // присваиваем указателям адрес текущего указателя блока памяти
    start = base->prev;                                                         // старт - предыдущий блок памяти в списке

    do
    {
        if (rover == start)                                                     // список замкнулся, обошли все
        {
            return NULL;
        }
        if (rover->tag)                                                         // блок занят, идем дальше
        {
            base = rover = rover->next;
        }
        else                                                                    // блок свободен, не меняем указатель base, т.к. его должно проверить условие цикла
        {
            rover = rover->next;
        }
    } while (base->tag || base->size < size);                                   // пока не попадется достаточно большой свободный блок

    base->user_size = usersize;
//
// found a block big enough
//
    extra = base->size - size;

    if (extra >  MINFRAGMENT)                                                   // если extra больше минимального предела, то создадим в незанятой памяти свободный блок
    {
        new_block = (memblock_t *) ((byte *)base + size );
        new_block->size = extra;
        new_block->user_size = 0;
        new_block->tag = 0;                                                     // освободим блок
        new_block->prev = base;
        new_block->id = ZONEID;
        new_block->next = base->next;
        new_block->next->prev = new_block;
        base->next = new_block;
        base->size = size;
    }

    base->tag = tag;                                                            // обозначим блок как занятый

    mainzone->rover = base->next;                                               // установим указатель rover правее последнего выделенного блока

    base->id = ZONEID;                                                          // установим ID для выделенного блока памяти (для идентификации способа ее выделения)

    return (void *) ((byte *)base + sizeof(memblock_t));                        // вернем указатель на выделенный блок
}


/*
========================
Z_Realloc
========================
*/

void *Z_Realloc (void *ptr, size_t newsize)
{
    memblock_t *block, *p;
    size_t          usersize = newsize;

    if ( !ptr )                                                                 // если на входе нулевой указатель
    {
        return Z_TagMalloc(newsize, ID_ZMAIN);                                  // то просто выделим новый блок
    }

    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));

    if( ZONEID != block->id )
    {
        Sys_extError("Z_Realloc: ptr is not ZONEID memory!\n");
    }

    if( !block->tag )
    {
        Sys_extError("Z_Realloc: input ptr is a free!\n");
    }

    newsize += sizeof(memblock_t);                                              // account for size of block header
    //newsize += 4;                                                             // space for memory trash tester
    newsize = (newsize + 7) & ~7;                                               // align to 8-byte boundary

    if( block->size >= newsize)                                                 // если запрашиваемый размер не превосходит реального размера блока
    {                                                                           // или помечен как свободный
        //printf("case: NO changes\n");
        block->user_size = usersize;
        return ptr;                                                             // все можно оставить как есть, и ничего не менять.
    }

    size_t extra = block->size + block->next->size  - newsize;                  // т.е. рассматриваем заголовок следующего блока под снос.
    if( (!block->next->tag) && (extra >= sizeof(memblock_t)) )                  // если следующий блок свободен, и в нем достаточно места
    {
        if( extra < MINFRAGMENT )                                               // если памяти достаточно, но запас меньше минимального фрагмента,
        {                                                                       // то просто объединим текущий блок с пустым
            printf("case: concat blocks\n");
            memblock_t *old = block->next;                                      // сохраним указатель на присоединяемый блок
            p = old->next;                                                      // получим указатель на блок следующий после присоединяемого
            block->size += block->next->size;                                   // увеличим размер блока на размер присоединяемого блока
            block->user_size = usersize;                                        // запишем в заголовок размер запрошенный пользователем (т.е. без выравнивания и размера заголовка)
            block->next = p;                                                    // присоединим к текущему блоку блок следующий после присоединенного
            p->prev = block;                                                    // присоединим к блоку следующий после присоединенного текущий блок
            memset(old, 0, sizeof(memblock_t));                                 // затрем старый заголовок в куче
            return ptr;                                                         // вернем указатель на расширенный блок
        }
        if( extra >= MINFRAGMENT )                                              // если памяти достаточно, и запас больше минимального фрагмента,
        {                                                                       // то передвинем начало следующего блока
            printf("case: concat and create blocks\n");
            memblock_t *next;
            next = block->next->next;                                           // сохраним указатель на следующий после сдвигаемого блок
            memset(block->next, 0, sizeof(memblock_t));                         // затрем старый заголовок в куче
            p = (memblock_t *) ((byte*)(block) + newsize);                      // создадим указатель на новый (сдвинутый) заголовок следующего блока
            p->prev = block;                                                    // установим связь сдвинутого заголовка с окружающими
            p->next = next;                                         
            next->prev = p;                                                     // установим связь следующего заголовка со сдвинутым
            block->next = p;                                                     // установим связь предыдущего(текущего) заголовка со сдвинутым
            p->size = extra;                                                    // установим размер сдвинутого блока
            p->user_size = 0;                                                   // т.к. данный блок свободен, то usersize == 0
            p->id = ZONEID;                                                     // установим ID сдвинутого блока
            block->size = newsize;                                              // установим размер нового блока
            block->user_size = usersize;
            return ptr;
        }
    }

    //printf("case: realloc new memory\n");
    p = (memblock_t *)Z_TagMalloc(usersize, ID_ZMAIN);                          // выделим память под новый блок
    if( NULL == p )                                                             // если невозможно выделить достаточно памяти для нового блока
    {
        //printf("Not enough memory!\n");
        return NULL;                                                            // селяви, вернем нулевой указатель
    }

    memcpy(p, ptr, block->size - sizeof(memblock_t));                           // копируем в новый блок содержимое старого, без заголовка (т.к. блоки не пересекаются)

    Z_Free(ptr);                                                                // освободим старый блок
    return p;                                                                   // вернем указатель на новый блок
}


/*
========================
Z_Print
========================
*/
void Z_Print (memzone_t *zone)
{
    memblock_t *block;

    Con_Printf ("zone size: %d  location: %p\n",mainzone->size,mainzone);

    for (block = zone->blocklist.next ; ; block = block->next)
    {
        Con_Printf ("block:%p    size:%7i    usersize:%7i    tag:%3i\n", block, block->size, block->user_size, block->tag);

        if (block->next == &zone->blocklist)
            break;      // all blocks have been hit
        if ( (byte *)block + block->size != (byte *)block->next)
            Con_Printf ("ERROR: block size does not touch the next block\n");
        if ( block->next->prev != block)
            Con_Printf ("ERROR: next block doesn't have proper back link\n");
        if (!block->tag && !block->next->tag)
            Con_Printf ("ERROR: two consecutive free blocks\n");
        if ( block->id != ZONEID )
            Con_Printf ("ERROR: block ID != ZONEID!\n");
    }
}


/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap (void)
{
    memblock_t *block;

    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
        if (block->next == &mainzone->blocklist)
            break;                  // all blocks have been hit
        if ( (byte *)block + block->size != (byte *)block->next)
            Sys_Error ("Z_CheckHeap: block size does not touch the next block\n");
        if ( block->next->prev != block)
            Sys_Error ("Z_CheckHeap: next block doesn't have proper back link\n");
        if (!block->tag && !block->next->tag)
            Sys_Error ("Z_CheckHeap: two consecutive free blocks\n");
        if ( block->id != ZONEID )
            Con_Printf ("Z_CheckHeap: block ID != ZONEID!\n");
    }
}



/*
============
Z_Report

============
*/
void Z_Report (void)
{
    Con_Printf ("%d byte data free\n", (mainzone->blocklist.prev->size - sizeof(memblock_t)));
}


//============================================================================

/*
========================
Memory_Init
========================
*/
void Memory_Init (void *buf, size_t size)                                       //buf - уже выделенная операционной системой куча, size - ее размер.
{
    mainzone = (memzone_t*)buf;                                                 // выделяем для основной памяти size байт
    Z_ClearZone (mainzone, size);                                               // уничтожаются все блоки, список обнуляется. теперь вся mainzone свободна для выделения памяти.
}


