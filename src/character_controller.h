#pragma once

#include <cstdint>
#include <list>
#include <vector>

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <LinearMath/btVector3.h>

/*------ Lara's model-------
             .=.
            | 14|
             \ / \
         / |     | \
    11  / |   7   | \  8
       /   |     |   \
       |    =====    |
    12 |    =====    | 9
       |   /  0  \   |
    13 0  /_______\  0 10
          |  | |  |
          |1 | |4 |
          |  | |  |
          |__| |__|
          |  | |  |
          |2 | |5 |
          |  | |  |
          |__| |__|
       3  |__| |__|  6
--------------------------*/

#define BODY_PART_BODY_LOW      (0x00000001)            // 0
#define BODY_PART_BODY_UPPER    (0x00000002)            // 7
#define BODY_PART_HEAD          (0x00000004)            // 14

#define BODY_PART_LEFT_HAND_1   (0x00000008)            // 11
#define BODY_PART_LEFT_HAND_2   (0x00000010)            // 12
#define BODY_PART_LEFT_HAND_3   (0x00000020)            // 13
#define BODY_PART_RIGHT_HAND_1  (0x00000040)            // 8
#define BODY_PART_RIGHT_HAND_2  (0x00000080)            // 9
#define BODY_PART_RIGHT_HAND_3  (0x00000100)            // 10

#define BODY_PART_LEFT_LEG_1    (0x00000200)            // 1
#define BODY_PART_LEFT_LEG_2    (0x00000400)            // 2
#define BODY_PART_LEFT_LEG_3    (0x00000800)            // 3
#define BODY_PART_RIGHT_LEG_1   (0x00001000)            // 4
#define BODY_PART_RIGHT_LEG_2   (0x00002000)            // 5
#define BODY_PART_RIGHT_LEG_3   (0x00004000)            // 6

#define BODY_PART_LEGS_1        (BODY_PART_LEFT_LEG_1 | BODY_PART_RIGHT_LEG_1)
#define BODY_PART_LEGS_2        (BODY_PART_LEFT_LEG_2 | BODY_PART_RIGHT_LEG_2)
#define BODY_PART_LEGS_3        (BODY_PART_LEFT_LEG_3 | BODY_PART_RIGHT_LEG_3)

#define BODY_PART_HANDS_1        (BODY_PART_LEFT_HAND_1 | BODY_PART_RIGHT_HAND_1)
#define BODY_PART_HANDS_2        (BODY_PART_LEFT_HAND_2 | BODY_PART_RIGHT_HAND_2)
#define BODY_PART_HANDS_3        (BODY_PART_LEFT_HAND_3 | BODY_PART_RIGHT_HAND_3)

#define BODY_PART_HANDS          (BODY_PART_HANDS_1 | BODY_PART_HANDS_2 | BODY_PART_HANDS_3)
#define BODY_PART_LEGS           (BODY_PART_LEGS_1 | BODY_PART_LEGS_2 | BODY_PART_LEGS_3)

/*
 * default legs offsets
 */
#define LEFT_LEG                    (3)
#define RIGHT_LEG                   (6)
#define LEFT_HAND                   (13)
#define RIGHT_HAND                  (10)

#define CHARACTER_USE_COMPLEX_COLLISION         (1)

namespace
{
// Speed limits
constexpr float FREE_FALL_SPEED_1        = 2000.0f;
constexpr float FREE_FALL_SPEED_2        = 4500.0f;
constexpr float FREE_FALL_SPEED_MAXSAFE  = 5500.0f;
constexpr float FREE_FALL_SPEED_CRITICAL = 7500.0f;
constexpr float FREE_FALL_SPEED_MAXIMUM  = 7800.0f;

constexpr float MAX_SPEED_UNDERWATER     = 64.0f;
constexpr float MAX_SPEED_ONWATER        = 24.0f;
constexpr float MAX_SPEED_QUICKSAND      = 5.0f;

constexpr float ROT_SPEED_UNDERWATER     = 2.0f;
constexpr float ROT_SPEED_ONWATER        = 3.0f;
constexpr float ROT_SPEED_LAND           = 4.5f;
constexpr float ROT_SPEED_FREEFALL       = 0.5f;
constexpr float ROT_SPEED_MONKEYSWING    = 3.5f;

constexpr float INERTIA_SPEED_UNDERWATER = 1.0f;
constexpr float INERTIA_SPEED_ONWATER    = 1.5f;

// Lara's character behavior constants
constexpr int   DEFAULT_MAX_MOVE_ITERATIONS             = 3;                              //!< @fixme magic
constexpr float DEFAULT_MIN_STEP_UP_HEIGHT              = 128.0f;                         //!< @fixme check original
constexpr float DEFAULT_MAX_STEP_UP_HEIGHT              = 256.0f + 32.0f;                 //!< @fixme check original
constexpr float DEFAULT_FALL_DOWN_HEIGHT                = 320.0f;                         //!< @fixme check original
constexpr float DEFAULT_CLIMB_UP_HEIGHT                 = 1920.0f;                        //!< @fixme check original
constexpr float DEFAULT_CRITICAL_SLANT_Z_COMPONENT      = 0.810f;                         //!< @fixme cos(alpha = 30 deg)
constexpr float DEFAULT_CRITICAL_WALL_COMPONENT         = -0.707f;                        //!< @fixme cos(alpha = 45 deg)
constexpr float DEFAULT_CHARACTER_SLIDE_SPEED_MULT      = 75.0f;                          //!< @fixme magic - not like in original
constexpr float DEFAULT_CHARACTER_CLIMB_R               = 32.0f;
constexpr float DEFAULT_CHARACTER_WADE_DEPTH            = 256.0f;

//! If less than this much of Lara is looking out of the water, she goes from wading to swimming.
//! @fixme Guess
constexpr float DEFAULT_CHARACTER_SWIM_DEPTH            = 100.0f;

// CHARACTER PARAMETERS DEFAULTS
constexpr float PARAM_ABSOLUTE_MAX                = -1;

constexpr float LARA_PARAM_HEALTH_MAX             = 1000.0f;      //!< 1000 HP
constexpr float LARA_PARAM_AIR_MAX                = 3600.0f;      //!< 60 secs of air
constexpr float LARA_PARAM_STAMINA_MAX            = 120.0f;       //!< 4  secs of sprint
constexpr float LARA_PARAM_WARMTH_MAX             = 240.0f;       //!< 8  secs of freeze
constexpr float LARA_PARAM_POISON_MAX             = 5.0f;

constexpr float CHARACTER_BOX_HALF_SIZE = 128.0f;
constexpr float CHARACTER_BASE_RADIUS   = 128.0f;
constexpr float CHARACTER_BASE_HEIGHT   = 512.0f;
}
