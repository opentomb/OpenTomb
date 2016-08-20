
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

#define GF_MAX_ACTIONS 32
#define GF_MAX_SECRETS 256

#define GF_NOENTRY     -1

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

    uint16_t            getGameState();
    void                setGameState(uint16_t state);

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

    gameflow_action     m_actions[GF_MAX_ACTIONS];
    char                m_secretsTriggerMap[GF_MAX_SECRETS];
};

enum GF_OP
{
    GF_OP_PICTURE,         // Unknown possibly TR1?
    GF_OP_LISTSTART,       // Unknown possibly TR1?
    GF_OP_LISTEND,         // Unknown possibly TR1?
    GF_OP_STARTFMV,        // Start a FMV
    GF_OP_STARTLEVEL,      // Start a level
    GF_OP_STARTCINE,       // Start a cutscene
    GF_OP_LEVELCOMPLETE,   // Trigger level completion display
    GF_OP_STARTDEMO,       // Start a demo level
    GF_OP_JUMPTOSEQUENCE,  // Jump to an existing sequence
    GF_OP_ENDSEQUENCE,     // End current sequence
    GF_OP_SETTRACK,        // Set audio track
    GF_OP_ENABLESUNSET,    // ??? Used on Bartoli's hideout!
    GF_OP_LOADINGPIC,      // Set loading screen picture
    GF_OP_DEADLYWATER,     // Set water kills lara (Used on that Rig level, Temple of Xian etc..)
    GF_OP_REMOVEWEAPONS,   // Remove Lara's weapons
    GF_OP_GAMECOMPLETE,    // Trigger game completion display
    GF_OP_CUTANGLE,        // Cutscene start angle? Possibly rotation flags? Unknown!
    GF_OP_NOFLOOR,         // Makes Lara infinitely fall at the bottom of the level
    GF_OP_ADDTOINVENTORY,  // Add an item to inventory
    GF_OP_LARASTARTANIM,   // Change Lara's start anim or the state? (Used on levels where Lara starts in water)
    GF_OP_NUMSECRETS,      // Change the number of secrets?
    GF_OP_KILLTOCOMPLETE,  // Kill to complete, used on levels like IcePalace, Nightmare in Vegas so killing the boss ends the level!
    GF_OP_REMOVEAMMO,      // Remove Ammo
    GF_OP_LASTINDEX
};

extern class CGameflow gameflow;

#endif //GAMEFLOW_H
