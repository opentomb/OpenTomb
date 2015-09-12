#pragma once

#include <array>
#include <cstdint>

namespace engine
{

enum class Opcode
{
    Picture,         // Unknown possibly TR1?
    ListStart,       // Unknown possibly TR1?
    ListEnd,         // Unknown possibly TR1?
    StartFMV,        // Start a FMV
    StartLevel,      // Start a level
    StartCine,       // Start a cutscene
    LevelComplete,   // Trigger level completion display
    StartDemo,       // Start a demo level
    JumpToSequence,  // Jump to an existing sequence
    EndSequence,     // End current sequence
    SetTrack,        // Set audio track
    EnableSunset,    // ??? Used on Bartoli's hideout!
    LoadingPic,      // Set loading screen picture
    DeadlyWater,     // Set water kills lara (Used on that Rig level, Temple of Xian etc..)
    RemoveWeapons,   // Remove Lara's weapons
    GameComplete,    // Trigger game completion display
    CutAngle,        // Cutscene start angle? Possibly rotation flags? Unknown!
    NoFloor,         // Makes Lara infinitely fall at the bottom of the level
    AddToInventory,  // Add an item to inventory
    LaraStartAnim,   // Change Lara's start anim or the state? (Used on levels where Lara starts in water)
    NumSecrets,      // Change the number of secrets?
    KillToComplete,  // Kill to complete, used on levels like IcePalace, Nightmare in Vegas so killing the boss ends the level!
    RemoveAmmo,      // Remove Ammo
    Sentinel
};

struct GameflowAction
{
    Opcode      opcode = Opcode::Sentinel;
    uint8_t     operand;
};

class Gameflow
{

public:
    void init();
    void execute();
    bool send(Opcode opcode, int operand = -1);

    const std::string& getLevelPath() const
    {
        return m_currentLevelPath;
    }

    void setLevelPath(std::string path)
    {
        m_currentLevelPath = path;
    }

    uint8_t getGameID() const
    {
        return m_currentGameID;
    }

    void setGameID(int8_t gameID)
    {
        m_currentGameID = gameID;
    }

    uint32_t getLevelID() const
    {
        return m_currentLevelID;
    }

    void setLevelID(uint32_t levelID)
    {
        m_currentLevelID = levelID;
    }

    void resetSecretStatus()
    {
        m_secretsTriggerMap.fill(false);
    }

    bool getSecretStatus(int secret) const
    {
        if(secret < 0 || secret >= static_cast<int>(m_secretsTriggerMap.size()))
            return false;

        return m_secretsTriggerMap[secret];
    }

    void setSecretStatus(int secret, bool status)
    {
        if(secret < 0 || secret >= static_cast<int>(m_secretsTriggerMap.size()))
            return;

        m_secretsTriggerMap[secret] = status;
    }

private:
    std::string       m_currentLevelPath;       //Level path from script example: DATA/TR1/DATA/LEVEL1.PHD
    std::string       m_currentLevelName;       //Level name from script example: Caves
    uint8_t           m_currentGameID;          //
    uint32_t          m_currentLevelID;         //Level ID from script example: 1 = Caves.

    bool              m_nextAction;             //Should gameflow do next action?
    std::array<GameflowAction, 32> m_actions;
    std::array<bool, 256> m_secretsTriggerMap;                     //Info for what secrets have been triggered in a level
};

extern Gameflow Gameflow_Manager;

} // namespace engine
