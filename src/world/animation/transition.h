#pragma once

#include "common.h"
#include "world/statecontroller.h"

#include <vector>

namespace world
{
namespace animation
{
/**
 * Describes the target of an animation transition.
 */
struct AnimationState
{
    AnimationId animation = 0;
    size_t frame = 0;
    LaraState state = LaraState::WalkForward;
};

/**
 * A transition case defines a frame range for which it is valid and a target animation state.
 */
struct TransitionCase
{
    size_t firstFrame;
    size_t lastFrame;
    AnimationState target;
};

/**
 * A collections of transition cases for a specific state ID.
 */
struct Transition
{
    LaraState id;
    std::vector<TransitionCase> cases;
};
}
}
