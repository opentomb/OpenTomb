#pragma once

#include <cstdint>
#include <string>

#define GF_MAX_ACTIONS 32
#define GF_MAX_SECRETS 256
#define GF_NOENTRY     -1

namespace engine
{

struct GameflowAction
{
    int8_t      opcode;
    uint8_t     operand;
};

class Gameflow
{

public:
    void Init();
    void Do();
    bool Send(int opcode, int operand = -1);

    bool SecretsTriggerMap[GF_MAX_SECRETS+1];                     //Info for what secrets have been triggered in a level

    std::string getLevelPath()
    {
        return m_currentLevelPath;
    }

    void setLevelPath(std::string path)
    {
        m_currentLevelPath = path;
    }

    uint8_t getGameID()
    {
        return m_currentGameID;
    }

    void setGameID(int8_t gameID)
    {
        m_currentGameID = gameID;
    }

    uint32_t getLevelID()
    {
        return m_currentLevelID;
    }

    void setLevelID(uint32_t levelID)
    {
        m_currentLevelID = levelID;
    }

private:
    std::string       m_currentLevelPath;       //Level path from script example: DATA/TR1/DATA/LEVEL1.PHD
    std::string       m_currentLevelName;       //Level name from script example: Caves
    uint8_t           m_currentGameID;          //
    uint32_t          m_currentLevelID;         //Level ID from script example: 1 = Caves.

    bool              m_nextAction;             //Should gameflow do next action?
    GameflowAction    Actions[GF_MAX_ACTIONS+1];
};

enum
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

extern Gameflow Gameflow_Manager;

} // namespace engine
