#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "engine.h"

#define TR_GAMEFLOW_MAX_ACTIONS 32
#define TR_GAMEFLOW_MAX_SECRETS 256

#define TR_GAMEFLOW_NOENTRY     -1

typedef struct gameflow_action_s
{
    int8_t      opcode;
    uint8_t     operand;
} gameflow_action_t, *gameflow_action_p;

typedef struct gameflow_manager_s
{
    std::string CurrentLevelName;
    std::string CurrentLevelPath;
    uint8_t             CurrentGameID;
    int CurrentLevelID;

    bool                NextAction;
    gameflow_action_s   Actions[TR_GAMEFLOW_MAX_ACTIONS];

    bool SecretsTriggerMap[TR_GAMEFLOW_MAX_SECRETS];                     //Info for what secrets have been triggered in a level
    
} gameflow_manager_t, *gameflow_manager_p;

enum TR_GAMEFLOW_OP
{
    TR_GAMEFLOW_OP_PICTURE,         // Unknown possibly TR1?
    TR_GAMEFLOW_OP_LISTSTART,       // Unknown possibly TR1?
    TR_GAMEFLOW_OP_LISTEND,         // Unknown possibly TR1?
    TR_GAMEFLOW_OP_STARTFMV,        // Start a FMV
    TR_GAMEFLOW_OP_STARTLEVEL,      // Start a level
    TR_GAMEFLOW_OP_STARTCINE,       // Start a cutscene
    TR_GAMEFLOW_OP_LEVELCOMPLETE,   // Trigger level completion display
    TR_GAMEFLOW_OP_STARTDEMO,       // Start a demo level
    TR_GAMEFLOW_OP_JUMPTOSEQUENCE,  // Jump to an existing sequence
    TR_GAMEFLOW_OP_ENDSEQUENCE,     // End current sequence
    TR_GAMEFLOW_OP_SETTRACK,        // Set audio track
    TR_GAMEFLOW_OP_ENABLESUNSET,    // ??? Used on Bartoli's hideout!
    TR_GAMEFLOW_OP_LOADINGPIC,      // Set loading screen picture
    TR_GAMEFLOW_OP_DEADLYWATER,     // Set water kills lara (Used on that Rig level, Temple of Xian etc..)
    TR_GAMEFLOW_OP_REMOVEWEAPONS,   // Remove Lara's weapons
    TR_GAMEFLOW_OP_GAMECOMPLETE,    // Trigger game completion display
    TR_GAMEFLOW_OP_CUTANGLE,        // Cutscene start angle? Possibly rotation flags? Unknown!
    TR_GAMEFLOW_OP_NOFLOOR,         // Makes Lara infinitely fall at the bottom of the level
    TR_GAMEFLOW_OP_ADDTOINVENTORY,  // Add an item to inventory
    TR_GAMEFLOW_OP_LARASTARTANIM,   // Change Lara's start anim or the state? (Used on levels where Lara starts in water)
    TR_GAMEFLOW_OP_NUMSECRETS,      // Change the number of secrets?
    TR_GAMEFLOW_OP_KILLTOCOMPLETE,  // Kill to complete, used on levels like IcePalace, Nightmare in Vegas so killing the boss ends the level!
    TR_GAMEFLOW_OP_REMOVEAMMO,      // Remove Ammo
    TR_GAMEFLOW_OP_LASTINDEX
};

extern gameflow_manager_s gameflow_manager;

void Gameflow_Init();
void Gameflow_Do();
bool Gameflow_Send(int opcode, int operand = -1);
