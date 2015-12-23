#pragma once

#include <cstdint>

namespace world
{
namespace animation
{
enum class AnimCommandOpcode : int
{
    SetPosition = 1,
    SetVelocity = 2,
    EmptyHands = 3,
    Kill = 4,
    PlaySound = 5,
    PlayEffect = 6,
    Interact = 7
};

//   ====== ANIMATION EFFECTS FLAGS ======

constexpr uint16_t TR_ANIMCOMMAND_CONDITION_LAND = 0x4000;
constexpr uint16_t TR_ANIMCOMMAND_CONDITION_WATER = 0X8000;
}
}
