/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2007 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "../config.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "../alMain.h"
#include "../Alc/compat.h"


typedef struct ConfigEntry {
    char *key;
    char *value;
} ConfigEntry;

typedef struct ConfigBlock {
    ConfigEntry *entries;
    unsigned int entryCount;
} ConfigBlock;
static ConfigBlock cfgBlock;


void ReadALConfig(void)
{
}

void FreeALConfig(void)
{
    unsigned int i;

    for(i = 0;i < cfgBlock.entryCount;i++)
    {
        free(cfgBlock.entries[i].key);
        free(cfgBlock.entries[i].value);
    }
    free(cfgBlock.entries);
}

const char *GetConfigValue(const char *blockName, const char *keyName, const char *def)
{
    unsigned int i;
    char key[256];

    if(!keyName)
        return def;

    if(blockName && strcasecmp(blockName, "general") != 0)
        snprintf(key, sizeof(key), "%s/%s", blockName, keyName);
    else
    {
        strncpy(key, keyName, sizeof(key)-1);
        key[sizeof(key)-1] = 0;
    }

    for(i = 0;i < cfgBlock.entryCount;i++)
    {
        if(strcasecmp(cfgBlock.entries[i].key, key) == 0)
        {
            TRACE("Found %s = \"%s\"\n", key, cfgBlock.entries[i].value);
            if(cfgBlock.entries[i].value[0])
                return cfgBlock.entries[i].value;
            return def;
        }
    }

    TRACE("Key %s not found\n", key);
    return def;
}

int ConfigValueExists(const char *blockName, const char *keyName)
{
    const char *val = GetConfigValue(blockName, keyName, "");
    return !!val[0];
}

int ConfigValueStr(const char *blockName, const char *keyName, const char **ret)
{
    const char *val = GetConfigValue(blockName, keyName, "");
    if(!val[0]) return 0;

    *ret = val;
    return 1;
}

int ConfigValueInt(const char *blockName, const char *keyName, int *ret)
{
    const char *val = GetConfigValue(blockName, keyName, "");
    if(!val[0]) return 0;

    *ret = strtol(val, NULL, 0);
    return 1;
}

int ConfigValueUInt(const char *blockName, const char *keyName, unsigned int *ret)
{
    const char *val = GetConfigValue(blockName, keyName, "");
    if(!val[0]) return 0;

    *ret = strtoul(val, NULL, 0);
    return 1;
}

int ConfigValueFloat(const char *blockName, const char *keyName, float *ret)
{
    const char *val = GetConfigValue(blockName, keyName, "");
    if(!val[0]) return 0;

#ifdef HAVE_STRTOF
    *ret = strtof(val, NULL);
#else
    *ret = (float)strtod(val, NULL);
#endif
    return 1;
}

int GetConfigValueBool(const char *blockName, const char *keyName, int def)
{
    const char *val = GetConfigValue(blockName, keyName, "");

    if(!val[0]) return !!def;
    return (strcasecmp(val, "true") == 0 || strcasecmp(val, "yes") == 0 ||
            strcasecmp(val, "on") == 0 || atoi(val) != 0);
}
