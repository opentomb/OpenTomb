
#ifndef GAMEFLOW_H
#define GAMEFLOW_H

#include "engine.h"

#define     GAME_1      (0)
#define     GAME_1_1    (1)
#define     GAME_1_5    (2)
#define     GAME_2      (3)
#define     GAME_2_1    (4)
#define     GAME_2_5    (5)
#define     GAME_3      (6)
#define     GAME_3_5    (7)
#define     GAME_4      (8)
#define     GAME_4_1    (9)
#define     GAME_5      (10)

#define TR_GAMEFLOW_MAX_ACTIONS 32
#define TR_GAMEFLOW_MAX_SECRETS 256

#define TR_GAMEFLOW_NOENTRY     -1

struct gameflow_action
{
    int8_t      m_opcode;
    uint8_t     m_operand;
};

class CGameflow
{

public:

    CGameflow();
    ~CGameflow();

    void                Do();
    bool                Send(int opcode, int operand = -1);

    uint8_t             getCurrentGameID();
    uint8_t             getCurrentLevelID();
    const char*         getCurrentLevelName();
    const char*         getCurrentLevelPath();
    char                getSecretStateAtIndex(int index);

    void                setCurrentGameID(int gameID);
    void                setCurrentLevelID(int levelID);
    void                setCurrentLevelName(const char* levelName);
    void                setCurrentLevelPath(const char* filePath);
    void                setSecretStateAtIndex(int index, int value);

    void                resetSecrets();

private:

    uint8_t             m_currentGameID;
    uint8_t             m_currentLevelID;

    char                m_currentLevelName[LEVEL_NAME_MAX_LEN];
    char                m_currentLevelPath[MAX_ENGINE_PATH];

    bool                m_nextAction;
    gameflow_action     m_actions[TR_GAMEFLOW_MAX_ACTIONS];
    char                m_secretsTriggerMap[TR_GAMEFLOW_MAX_SECRETS];
};

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

extern class CGameflow gameflow;

#endif //GAMEFLOW_H
