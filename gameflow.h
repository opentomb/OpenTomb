#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct gameflow_manager_s
{
    const char* Script;
    const char* CurrentLevelName;
    uint8_t     CurrentLevelID;
    
    uint8_t     Opcode;
    uint8_t     Operand;
    
    bool        NextAction;
} gameflow_manager_t, *gameflow_manager_p;

enum TR_GAMEFLOW_OP
{
    TR_GAMEFLOW_OP_UNKNOWN1,
    TR_GAMEFLOW_OP_UNKNOWN2,
    TR_GAMEFLOW_OP_UNKNOWN3,
    TR_GAMEFLOW_OP_PLAYFMV,
    TR_GAMEFLOW_OP_PLAYLEVEL,
    TR_GAMEFLOW_OP_PLAYCUTSCENE,
    TR_GAMEFLOW_OP_SHOWSTATS,
    TR_GAMEFLOW_OP_PLAYDEMO,
    TR_GAMEFLOW_OP_UNKNOWN4,
    TR_GAMEFLOW_OP_ENDSET,
    TR_GAMEFLOW_OP_PLAYSOUNDTRACK,
    TR_GAMEFLOW_OP_LARASTARTANIM,
    TR_GAMEFLOW_OP_SHOWCHAPTERS,
    TR_GAMEFLOW_OP_UNKNOWN5,
    TR_GAMEFLOW_OP_TAKEWEAPONS,
    TR_GAMEFLOW_OP_ENDGAME,
    TR_GAMEFLOW_OP_PREPARECUTSCENE,
    TR_GAMEFLOW_OP_UNKNOWN6,
    TR_GAMEFLOW_OP_GIVEITEM,
    TR_GAMEFLOW_OP_SETITEMSTATE,
    TR_GAMEFLOW_OP_SETNUMSECRETS,
    TR_GAMEFLOW_OP_UNKNOWN7,
    TR_GAMEFLOW_OP_TAKESUPPLIES,
    TR_GAMEFLOW_OP_LASTINDEX
};

extern gameflow_manager_s gameflow_manager;

void Gameflow_Do();
