#pragma once

#include "util/helpers.h"

namespace world
{
namespace animation
{
//! Default fixed TR framerate needed for animation calculation
constexpr float AnimationFrameRate = 30;
constexpr util::Duration AnimationFrameTime = util::fromSeconds(1.0f / AnimationFrameRate);

// This is the global game logic refresh interval (physics timestep)
// All game logic should be refreshed at this rate, including
// enemy AI, values processing and audio update.
// This should be a multiple of animation::FrameRate (1/30,60,90,120,...)
constexpr float GameLogicFrameRate = 2 * AnimationFrameRate;
constexpr util::Duration GameLogicFrameTime = util::fromSeconds(1.0f / GameLogicFrameRate);

enum class AnimUpdate
{
    None,
    NewFrame,
    NewAnim
};

enum class AnimationMode
{
    NormalControl,
    LoopLastFrame,
    WeaponCompat,
    Locked
};

using AnimationId = uint32_t;
}
}
