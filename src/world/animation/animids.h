#pragma once

#include "animation.h"

namespace world
{
namespace animation
{
//  ====== LARA'S ANIMATIONS ======
//  NOTE: In case of conflicting animations, there will be tr* prefix inside name.

//  TR1 AND ABOVE (0-159)

constexpr AnimationId TR_ANIMATION_LARA_RUN = 0;
constexpr AnimationId TR_ANIMATION_LARA_WALK_FORWARD = 1;
constexpr AnimationId TR_ANIMATION_LARA_END_WALK_RIGHT = 2;
constexpr AnimationId TR_ANIMATION_LARA_END_WALK_LEFT = 3;
constexpr AnimationId TR_ANIMATION_LARA_WALK_TO_RUN_RIGHT = 4;
constexpr AnimationId TR_ANIMATION_LARA_WALK_TO_RUN_LEFT = 5;
constexpr AnimationId TR_ANIMATION_LARA_STAY_TO_RUN = 6;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_WALK_RIGHT = 7;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_STAY_LEFT = 8;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_WALK_LEFT = 9;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_STAY_RIGHT = 10;
constexpr AnimationId TR_ANIMATION_LARA_STAY_SOLID = 11;                         // Intermediate animation used to reset flags and states.
constexpr AnimationId TR_ANIMATION_LARA_TURN_RIGHT_SLOW = 12;                    // Used once before the fast one if all weapon are in holsters
constexpr AnimationId TR_ANIMATION_LARA_TURN_LEFT_SLOW = 13;                     // Used once before the fast one if all weapon are in holsters
constexpr AnimationId TR_ANIMATION_LARA_LANDING_FORWARD_BOTH = 14;               // Original landing animation in the TR1 betas... But removed
constexpr AnimationId TR_ANIMATION_LARA_LANDING_FORWARD_BOTH_CONTINUE = 15;      // Original landing animation in the TR1 betas... But removed
constexpr AnimationId TR_ANIMATION_LARA_JUMPING_FORWARD_RIGHT = 16;              // OK
constexpr AnimationId TR_ANIMATION_LARA_START_FLY_FORWARD_RIGHT = 17;            // OK
constexpr AnimationId TR_ANIMATION_LARA_JUMPING_FORWARD_LEFT = 18;               // OK
constexpr AnimationId TR_ANIMATION_LARA_START_FLY_FORWARD_LEFT = 19;             // OK
constexpr AnimationId TR_ANIMATION_LARA_WALK_FORWARD_BEGIN = 20;
constexpr AnimationId TR_ANIMATION_LARA_WALK_FORWARD_BEGIN_CONTINUE = 21;
constexpr AnimationId TR_ANIMATION_LARA_START_FREE_FALL = 22;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_LONG = 23;
constexpr AnimationId TR_ANIMATION_LARA_LANDING_HARD = 24;
constexpr AnimationId TR_ANIMATION_LARA_LANDING_DEATH = 25;
constexpr AnimationId TR_ANIMATION_LARA_STAY_TO_GRAB = 26;
constexpr AnimationId TR_ANIMATION_LARA_STAY_TO_GRAB_CONTINUE = 27;
constexpr AnimationId TR_ANIMATION_LARA_TRY_HANG_VERTICAL = 28;
constexpr AnimationId TR_ANIMATION_LARA_BEGIN_HANGING_VERTICAL = 29;
constexpr AnimationId TR_ANIMATION_LARA_STOP_HANG_VERTICAL = 30;
constexpr AnimationId TR_ANIMATION_LARA_LANDING_LIGHT = 31;
constexpr AnimationId TR_ANIMATION_LARA_SMASH_JUMP = 32;
constexpr AnimationId TR_ANIMATION_LARA_SMASH_JUMP_CONTINUE = 33;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_FORWARD = 34;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_MIDDLE = 35;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_LONG_NO_HURT = 36;
constexpr AnimationId TR_ANIMATION_LARA_HANG_TO_RELEASE = 37;                    // Was meant to play when Lara is hanging at a ledge and the player releases the Action key
constexpr AnimationId TR_ANIMATION_LARA_STOP_WALK_BACK_RIGHT = 38;
constexpr AnimationId TR_ANIMATION_LARA_STOP_WALK_BACK_LEFT = 39;
constexpr AnimationId TR_ANIMATION_LARA_WALK_BACK = 40;
constexpr AnimationId TR_ANIMATION_LARA_START_WALK_BACK = 41;
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_3CLICK = 42;
constexpr AnimationId TR_ANIMATION_LARA_UNKNOWN2 = 43;                           // Was meant to be used like the = 52; but finally it got removed
constexpr AnimationId TR_ANIMATION_LARA_ROTATE_RIGHT = 44;
constexpr AnimationId TR_ANIMATION_LARA_JUMPING_FORWARD_TO_FREEFALL = 45;        // Used after the forward jump if she keeps falling
constexpr AnimationId TR_ANIMATION_LARA_FLY_FORWARD_TRY_TO_HANG = 46;
constexpr AnimationId TR_ANIMATION_LARA_ROLL_ALTERNATE = 47;                     // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROLL_END_ALTERNATE = 48;                 // Unused
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_NO_HURT = 49;
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_2CLICK = 50;
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_2CLICK_END = 51;
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_2CLICK_END_RUNNING = 52;           // Used if the player keeps pressing the UP cursor key

constexpr AnimationId TR_ANIMATION_LARA_WALL_SMASH_LEFT = 53;
constexpr AnimationId TR_ANIMATION_LARA_WALL_SMASH_RIGHT = 54;
constexpr AnimationId TR_ANIMATION_LARA_RUN_UP_STEP_RIGHT = 55;
constexpr AnimationId TR_ANIMATION_LARA_RUN_UP_STEP_LEFT = 56;
constexpr AnimationId TR_ANIMATION_LARA_WALK_UP_STEP_RIGHT = 57;
constexpr AnimationId TR_ANIMATION_LARA_WALK_UP_STEP_LEFT = 58;
constexpr AnimationId TR_ANIMATION_LARA_WALK_DOWN_LEFT = 59;
constexpr AnimationId TR_ANIMATION_LARA_WALK_DOWN_RIGHT = 60;
constexpr AnimationId TR_ANIMATION_LARA_WALK_DOWN_BACK_LEFT = 61;
constexpr AnimationId TR_ANIMATION_LARA_WALK_DOWN_BACK_RIGHT = 62;

constexpr AnimationId TR_ANIMATION_LARA_PULL_SWITCH_DOWN = 63;
constexpr AnimationId TR_ANIMATION_LARA_PULL_SWITCH_UP = 64;

constexpr AnimationId TR_ANIMATION_LARA_WALK_LEFT = 65;
constexpr AnimationId TR_ANIMATION_LARA_WALK_LEFT_END = 66;
constexpr AnimationId TR_ANIMATION_LARA_WALK_RIGHT = 67;
constexpr AnimationId TR_ANIMATION_LARA_WALK_RIGHT_END = 68;
constexpr AnimationId TR_ANIMATION_LARA_ROTATE_LEFT = 69;
constexpr AnimationId TR_ANIMATION_LARA_SLIDE_FORWARD = 70;
constexpr AnimationId TR_ANIMATION_LARA_SLIDE_FORWARD_END = 71;
constexpr AnimationId TR_ANIMATION_LARA_SLIDE_FORWARD_STOP = 72;
constexpr AnimationId TR_ANIMATION_LARA_STAY_JUMP_SIDES = 73;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_BACK_BEGIN = 74;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_BACK = 75;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN = 76;
constexpr AnimationId TR_ANIMATION_LARA_CONTINUE_FLY_FORWARD = 77;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_LEFT_BEGIN = 78;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_LEFT = 79;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_RIGHT_BEGIN = 80;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_RIGHT = 81;
constexpr AnimationId TR_ANIMATION_LARA_LANDING_MIDDLE = 82;
constexpr AnimationId TR_ANIMATION_LARA_FORWARD_TO_FREE_FALL = 83;
constexpr AnimationId TR_ANIMATION_LARA_LEFT_TO_FREE_FALL = 84;
constexpr AnimationId TR_ANIMATION_LARA_RIGHT_TO_FREE_FALL = 85;

constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_SWIM_FORWARD = 86;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_SWIM_SOLID = 87;
constexpr AnimationId TR_ANIMATION_LARA_RUN_BACK_BEGIN = 88;
constexpr AnimationId TR_ANIMATION_LARA_RUN_BACK = 89;
constexpr AnimationId TR_ANIMATION_LARA_RUN_BACK_END = 90;
constexpr AnimationId TR_ANIMATION_LARA_TRY_HANG_VERTICAL_BEGIN = 91;              // Native bug: glitchy intermediate animation.
constexpr AnimationId TR_ANIMATION_LARA_LANDING_FROM_RUN = 92;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_BACK = 93;
constexpr AnimationId TR_ANIMATION_LARA_FLY_FORWARD_TRY_HANG = 94;
constexpr AnimationId TR_ANIMATION_LARA_TRY_HANG_SOLID = 95;
constexpr AnimationId TR_ANIMATION_LARA_HANG_IDLE = 96;                            // Main climbing animation... triggers
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_ON = 97;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_TO_LONG = 98;
constexpr AnimationId TR_ANIMATION_LARA_FALL_CROUCHING_LANDING = 99;               // Unused
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_TO_SIDE_LANDING = 100;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_TO_SIDE_LANDING_ALTERNATE = 101; // Maybe it was used at the beginning of a forward jump when the player presses Action? Maybe it was used like this with the original beta anim = 73;
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_ON_END = 102;
constexpr AnimationId TR_ANIMATION_LARA_STAY_IDLE = 103;
constexpr AnimationId TR_ANIMATION_LARA_START_SLIDE_BACKWARD = 104;
constexpr AnimationId TR_ANIMATION_LARA_SLIDE_BACKWARD = 105;
constexpr AnimationId TR_ANIMATION_LARA_SLIDE_BACKWARD_END = 106;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_SWIM_TO_IDLE = 107;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_IDLE = 108;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWARER_IDLE_TO_SWIM = 109;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_IDLE = 110;

constexpr AnimationId TR_ANIMATION_LARA_CLIMB_OUT_OF_WATER = 111;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER = 112;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_DIVE_ALTERNATE = 113;              // This one is not used
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_TO_ONWATER = 114;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_DIVE = 115;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_SWIM_FORWARD = 116;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_SWIM_FORWARD_TO_IDLE = 117;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_IDLE_TO_SWIM = 118;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_TO_UNDERWATER_ALTERNATE = 119;   // This one is used
constexpr AnimationId TR_ANIMATION_LARA_START_OBJECT_MOVING = 120;
constexpr AnimationId TR_ANIMATION_LARA_STOP_OBJECT_MOVING = 121;
constexpr AnimationId TR_ANIMATION_LARA_OBJECT_PULL = 122;
constexpr AnimationId TR_ANIMATION_LARA_OBJECT_PUSH = 123;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_DEATH = 124;
constexpr AnimationId TR_ANIMATION_LARA_AH_FORWARD = 125;
constexpr AnimationId TR_ANIMATION_LARA_AH_BACKWARD = 126;
constexpr AnimationId TR_ANIMATION_LARA_AH_LEFT = 127;
constexpr AnimationId TR_ANIMATION_LARA_AH_RIGHT = 128;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_SWITCH = 129;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_PICKUP = 130;
constexpr AnimationId TR_ANIMATION_LARA_USE_KEY = 131;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_DEATH = 132;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_DIE = 133;
constexpr AnimationId TR_ANIMATION_LARA_USE_PUZZLE = 134;
constexpr AnimationId TR_ANIMATION_LARA_PICKUP = 135;
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_LEFT = 136;
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_RIGHT = 137;
constexpr AnimationId TR_ANIMATION_LARA_STAY_TO_DEATH = 138;
constexpr AnimationId TR_ANIMATION_LARA_SQUASH_BOULDER = 139;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_IDLE_TO_SWIM_BACK = 140;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_SWIM_BACK = 141;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_SWIM_BACK_TO_IDLE = 142;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_SWIM_LEFT = 143;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_SWIM_RIGHT = 144;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_TO_DEATH = 145;
constexpr AnimationId TR_ANIMATION_LARA_ROLL_BEGIN = 146;
constexpr AnimationId TR_ANIMATION_LARA_ROLL_CONTINUE = 147;
constexpr AnimationId TR_ANIMATION_LARA_ROLL_END = 148;
constexpr AnimationId TR_ANIMATION_LARA_SPIKED = 149;
constexpr AnimationId TR_ANIMATION_LARA_OSCILLATE_HANG_ON = 150;
constexpr AnimationId TR_ANIMATION_LARA_LANDING_ROLL = 151;
constexpr AnimationId TR_ANIMATION_LARA_FISH_TO_UNDERWATER1 = 152;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_FISH = 153;
constexpr AnimationId TR_ANIMATION_LARA_FISH_TO_UNDERWATER2 = 154;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_FISH_TO_DEATH = 155;
constexpr AnimationId TR_ANIMATION_LARA_START_FLY_LIKE_FISH_LEFT = 156;
constexpr AnimationId TR_ANIMATION_LARA_START_FLY_LIKE_FISH_RIGHT = 157;
constexpr AnimationId TR_ANIMATION_LARA_FREE_FALL_FISH_START = 158;
constexpr AnimationId TR_ANIMATION_LARA_CLIMB_ON2 = 159;

// TR2 AND ABOVE (160-216)

constexpr AnimationId TR_ANIMATION_LARA_STAND_TO_LADDER = 160;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_UP = 161;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_UP_STOP_RIGHT = 162;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_UP_STOP_LEFT = 163;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_IDLE = 164;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_UP_START = 165;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_DOWN_STOP_LEFT = 166;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_DOWN_STOP_RIGHT = 167;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_DOWN = 168;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_DOWN_START = 169;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_RIGHT = 170;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_LEFT = 171;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_HANG = 172;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_HANG_TO_IDLE = 173;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_TO_STAND = 174;
// constexpr AnimationId TR_ANIMATION_LARA_UNKNOWN5 = 175;                   // Unknown use
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_TO_WADE_SHALLOW = 176;
constexpr AnimationId TR_ANIMATION_LARA_WADE = 177;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_WADE_LEFT = 178;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_WADE_RIGHT = 179;
constexpr AnimationId TR_ANIMATION_LARA_WADE_TO_RUN_LEFT = 180;
constexpr AnimationId TR_ANIMATION_LARA_WADE_TO_RUN_RIGHT = 181;

constexpr AnimationId TR_ANIMATION_LARA_LADDER_BACKFLIP_START = 182;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_BACKFLIP_END = 183;
constexpr AnimationId TR_ANIMATION_LARA_WADE_TO_STAY_RIGHT = 184;
constexpr AnimationId TR_ANIMATION_LARA_WADE_TO_STAY_LEFT = 185;
constexpr AnimationId TR_ANIMATION_LARA_STAY_TO_WADE = 186;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_UP_HANDS = 187;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_DOWN_HANDS = 188;
constexpr AnimationId TR_ANIMATION_LARA_FLARE_THROW = 189;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_TO_WADE_DEEP = 190;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_TO_LAND_LOW = 191;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_TO_WADE = 192;
constexpr AnimationId TR_ANIMATION_LARA_ONWATER_TO_WADE = 193;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_TO_HANDS_DOWN = 194;
constexpr AnimationId TR_ANIMATION_LARA_SWITCH_SMALL_DOWN = 195;
constexpr AnimationId TR_ANIMATION_LARA_SWITCH_SMALL_UP = 196;
constexpr AnimationId TR_ANIMATION_LARA_BUTTON_PUSH = 197;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_SWIM_TO_STILL_HUDDLE = 198;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_SWIM_TO_STILL_SPRAWL = 199;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_SWIM_TO_STILL_MEDIUM = 200;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_TO_HANDS_RIGHT = 201;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_TO_HANDS_LEFT = 202;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_ROLL_BEGIN = 203;
constexpr AnimationId TR_ANIMATION_LARA_FLARE_PICKUP = 204;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_ROLL_END = 205;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_FLARE_PICKUP = 206;
constexpr AnimationId TR_ANIMATION_LARA_RUNNING_JUMP_ROLL_BEGIN = 207;
constexpr AnimationId TR_ANIMATION_LARA_SOMERSAULT = 208;
constexpr AnimationId TR_ANIMATION_LARA_RUNNING_JUMP_ROLL_END = 209;
constexpr AnimationId TR_ANIMATION_LARA_STANDING_JUMP_ROLL_BEGIN = 210;
constexpr AnimationId TR_ANIMATION_LARA_STANDING_JUMP_ROLL_END = 211;
constexpr AnimationId TR_ANIMATION_LARA_BACKWARDS_JUMP_ROLL_BEGIN = 212;
constexpr AnimationId TR_ANIMATION_LARA_BACKWARDS_JUMP_ROLL_END = 213;

constexpr AnimationId TR_ANIMATION_LARA_TR2_KICK = 214;
constexpr AnimationId TR_ANIMATION_LARA_TR2_ZIPLINE_GRAB = 215;
constexpr AnimationId TR_ANIMATION_LARA_TR2_ZIPLINE_RIDE = 216;
constexpr AnimationId TR_ANIMATION_LARA_TR2_ZIPLINE_FALL = 217;

// TR3 AND ABOVE (214-312)

constexpr AnimationId TR_ANIMATION_LARA_TR345_ZIPLINE_GRAB = 214;
constexpr AnimationId TR_ANIMATION_LARA_TR345_ZIPLINE_RIDE = 215;
constexpr AnimationId TR_ANIMATION_LARA_TR345_ZIPLINE_FALL = 216;
constexpr AnimationId TR_ANIMATION_LARA_TR345_STAND_TO_CROUCH = 217;

constexpr AnimationId TR_ANIMATION_LARA_SLIDE_FORWARD_TO_RUN = 246;      // Slide to run!

constexpr AnimationId TR_ANIMATION_LARA_JUMP_FORWARD_BEGIN_TO_GRAB = 248;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_FORWARD_END_TO_GRAB = 249;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_GRAB_RIGHT = 250;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_GRAB_LEFT = 251;

constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_SPRINT_LEFT = 224;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_SPRINT_RIGHT = 225;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT = 223;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_SLIDE_STAND_RIGHT = 226;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_SLIDE_STAND_RIGHT_BETA = 227;     // BETA SPRINT-SLIDE STAND
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_SLIDE_STAND_LEFT = 228;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_SLIDE_STAND_LEFT_BETA = 229;      // BETA SPRINT-SLIDE STAND
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_ROLL_LEFT = 230;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_ROLL_LEFT_BETA = 231;          // BETA SPRINT ROLL
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_ROLL_LEFT_TO_RUN = 232;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_ROLL_RIGHT = 308;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_ROLL_RIGHT_TO_RUN = 309;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_ROLL_ALTERNATE_BEGIN = 240;     // Not used natively
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_ROLL_ALTERNATE_CONTINUE = 241;  // Not used natively
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_ROLL_ALTERNATE_END = 242;       // Not used natively
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_RUN_LEFT = 243;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_RUN_RIGHT = 244;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_CROUCH_LEFT = 310;
constexpr AnimationId TR_ANIMATION_LARA_SPRINT_TO_CROUCH_RIGHT = 311;

constexpr AnimationId TR_ANIMATION_LARA_MONKEY_GRAB = 233;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_IDLE = 234;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_FALL = 235;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_FORWARD = 236;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_STOP_LEFT = 237;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_STOP_RIGHT = 238;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_IDLE_TO_FORWARD_LEFT = 239;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_IDLE_TO_FORWARD_RIGHT = 252;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_STRAFE_LEFT = 253;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_STRAFE_LEFT_END = 254;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_STRAFE_RIGHT = 255;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_STRAFE_RIGHT_END = 256;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TURN_AROUND = 257;                 // Use Titak's animation from TREP patch
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TURN_LEFT = 271;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TURN_RIGHT = 272;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TURN_LEFT_EARLY_END = 283;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TURN_LEFT_LATE_END = 284;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TURN_RIGHT_EARLY_END = 285;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TURN_RIGHT_LATE_END = 286;

constexpr AnimationId TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN = 218;     // Not used natively
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_BEGIN_ALTERNATE = 247;    // Not used
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_CONTINUE = 219;  // Not used natively
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_ROLL_FORWARD_END = 220;       // Not used natively
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_TO_STAND = 221;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_IDLE = 222;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_PREPARE = 245;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_IDLE_SMASH = 265;             // Not used natively
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_TO_CRAWL_BEGIN = 258;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_TO_CRAWL_CONTINUE = 273;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_TO_CRAWL_END = 264;

constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TO_CROUCH_BEGIN = 259;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TO_CROUCH_END = 274;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_FORWARD = 260;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_IDLE_TO_FORWARD = 261;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_BACKWARD = 276;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_IDLE_TO_BACKWARD = 275;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_IDLE = 263;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_FORWARD_TO_IDLE_BEGIN_RIGHT = 262;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_FORWARD_TO_IDLE_END_RIGHT = 266;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_FORWARD_TO_IDLE_BEGIN_LEFT = 267;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_FORWARD_TO_IDLE_END_LEFT = 268;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_BACKWARD_TO_IDLE_BEGIN_RIGHT = 277;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_BACKWARD_TO_IDLE_END_RIGHT = 278;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_BACKWARD_TO_IDLE_BEGIN_LEFT = 279;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_BACKWARD_TO_IDLE_END_LEFT = 280;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TURN_LEFT = 269;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TURN_LEFT_END = 281;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TURN_RIGHT = 270;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TURN_RIGHT_END = 282;

constexpr AnimationId TR_ANIMATION_LARA_HANG_TO_CROUCH_BEGIN = 287;
constexpr AnimationId TR_ANIMATION_LARA_HANG_TO_CROUCH_END = 288;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TO_HANG_BEGIN = 289;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TO_HANG_CONTINUE = 290;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_TO_HANG_END = 302;

constexpr AnimationId TR_ANIMATION_LARA_CROUCH_PICKUP = 291;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_PICKUP_FLARE = 312;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_PICKUP = 292;           // Not natively used - make it work

constexpr AnimationId TR_ANIMATION_LARA_CROUCH_SMASH_FORWARD = 293;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_SMASH_BACKWARD = 294;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_SMASH_RIGHT = 295;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_SMASH_LEFT = 296;

constexpr AnimationId TR_ANIMATION_LARA_CRAWL_SMASH_FORWARD = 297;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_SMASH_BACKWARD = 298;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_SMASH_RIGHT = 299;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_SMASH_LEFT = 300;

constexpr AnimationId TR_ANIMATION_LARA_CRAWL_DEATH = 301;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_ABORT = 303;

constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_CROUCH_LEFT_BEGIN = 304;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_CROUCH_RIGHT_BEGIN = 305;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_CROUCH_LEFT_END = 306;
constexpr AnimationId TR_ANIMATION_LARA_RUN_TO_CROUCH_RIGHT_END = 307;

// TR4 AND ABOVE (313-444)

constexpr AnimationId TR_ANIMATION_LARA_DOOR_OPEN_FORWARD = 313;
constexpr AnimationId TR_ANIMATION_LARA_DOOR_OPEN_BACK = 314;
constexpr AnimationId TR_ANIMATION_LARA_DOOR_KICK = 315;
constexpr AnimationId TR_ANIMATION_LARA_GIANT_BUTTON_PUSH = 316;
constexpr AnimationId TR_ANIMATION_LARA_FLOOR_TRAPDOOR_OPEN = 317;
constexpr AnimationId TR_ANIMATION_LARA_CEILING_TRAPDOOR_OPEN = 318;
constexpr AnimationId TR_ANIMATION_LARA_ROUND_HANDLE_GRAB_CLOCKWISE = 319;
constexpr AnimationId TR_ANIMATION_LARA_ROUND_HANDLE_GRAB_COUNTERCLOCKWISE = 320;
constexpr AnimationId TR_ANIMATION_LARA_COGWHEEL_PULL = 321;
constexpr AnimationId TR_ANIMATION_LARA_COGWHEEL_GRAB = 322;
constexpr AnimationId TR_ANIMATION_LARA_COGWHEEL_UNGRAB = 323;
constexpr AnimationId TR_ANIMATION_LARA_LEVERSWITCH_PUSH = 324;
constexpr AnimationId TR_ANIMATION_LARA_HOLE_GRAB = 325;
constexpr AnimationId TR_ANIMATION_LARA_STAY_TO_POLE_GRAB = 326;
constexpr AnimationId TR_ANIMATION_LARA_POLE_JUMP = 327;
constexpr AnimationId TR_ANIMATION_LARA_POLE_IDLE = 328;
constexpr AnimationId TR_ANIMATION_LARA_POLE_CLIMB_UP = 329;
constexpr AnimationId TR_ANIMATION_LARA_POLE_FALL = 330;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_FORWARD_TO_POLE_GRAB = 331;
constexpr AnimationId TR_ANIMATION_LARA_POLE_TURN_LEFT_BEGIN = 332;
constexpr AnimationId TR_ANIMATION_LARA_POLE_TURN_RIGHT_BEGIN = 333;
constexpr AnimationId TR_ANIMATION_LARA_POLE_IDLE_TO_CLIMB_DOWN = 334;
constexpr AnimationId TR_ANIMATION_LARA_POLE_CLIMB_DOWN = 335;
constexpr AnimationId TR_ANIMATION_LARA_POLE_CLIMB_DOWN_TO_IDLE = 336;
constexpr AnimationId TR_ANIMATION_LARA_JUMP_UP_TO_POLE_GRAB = 337;
constexpr AnimationId TR_ANIMATION_LARA_POLE_CLIMB_UP_INBETWEEN = 338;
constexpr AnimationId TR_ANIMATION_LARA_PULLEY_GRAB = 339;
constexpr AnimationId TR_ANIMATION_LARA_PULLEY_PULL = 340;
constexpr AnimationId TR_ANIMATION_LARA_PULLEY_UNGRAB = 341;
constexpr AnimationId TR_ANIMATION_LARA_POLE_GRAB_TO_STAY = 342;
// constexpr AnimationId TR_ANIMATION_LARA_UNKNOWN8 = 343;
constexpr AnimationId TR_ANIMATION_LARA_POLE_TURN_LEFT_END = 344;
// constexpr AnimationId TR_ANIMATION_LARA_UNKNOWN9 = 345;
constexpr AnimationId TR_ANIMATION_LARA_POLE_TURN_RIGHT_END = 346;
constexpr AnimationId TR_ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_BEGIN = 347;
constexpr AnimationId TR_ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_CONTINUE = 348;
constexpr AnimationId TR_ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_END = 349;
constexpr AnimationId TR_ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_BEGIN = 350;
constexpr AnimationId TR_ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_CONTINUE = 351;
constexpr AnimationId TR_ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_END = 352;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_TURN_LEFT = 353;
constexpr AnimationId TR_ANIMATION_LARA_CROUCH_TURN_RIGHT = 354;
constexpr AnimationId TR_ANIMATION_LARA_HANG_AROUND_LEFT_OUTER_BEGIN = 355;
constexpr AnimationId TR_ANIMATION_LARA_HANG_AROUND_LEFT_OUTER_END = 356;
constexpr AnimationId TR_ANIMATION_LARA_HANG_AROUND_RIGHT_OUTER_BEGIN = 357;
constexpr AnimationId TR_ANIMATION_LARA_HANG_AROUND_RIGHT_OUTER_END = 358;
constexpr AnimationId TR_ANIMATION_LARA_HANG_AROUND_LEFT_INNER_BEGIN = 359;
constexpr AnimationId TR_ANIMATION_LARA_HANG_AROUND_LEFT_INNER_END = 360;
constexpr AnimationId TR_ANIMATION_LARA_HANG_AROUND_RIGHT_INNER_BEGIN = 361;
constexpr AnimationId TR_ANIMATION_LARA_HANG_AROUND_RIGHT_INNER_END = 362;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_AROUND_LEFT_OUTER_BEGIN = 363;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_AROUND_LEFT_OUTER_END = 364;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_AROUND_RIGHT_OUTER_BEGIN = 365;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_AROUND_RIGHT_OUTER_END = 366;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_AROUND_LEFT_INNER_BEGIN = 367;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_AROUND_LEFT_INNER_END = 368;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_AROUND_RIGHT_INNER_BEGIN = 369;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_AROUND_RIGHT_INNER_END = 370;
constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TO_ROPE_BEGIN = 371;
constexpr AnimationId TR_ANIMATION_LARA_TRAIN_DEATH = 372;

constexpr AnimationId TR_ANIMATION_LARA_MONKEY_TO_ROPE_END = 373;
constexpr AnimationId TR_ANIMATION_LARA_ROPE_IDLE = 374;              // Review all rope animations!
constexpr AnimationId TR_ANIMATION_LARA_ROPE_DOWN_BEGIN = 375;
constexpr AnimationId TR_ANIMATION_LARA_ROPE_UP = 376;
constexpr AnimationId TR_ANIMATION_LARA_ROPE_IDLE_TO_SWING_SOFT = 377;                  // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_GRAB_TO_FALL = 378;                        // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_JUMP_TO_GRAB = 379;
constexpr AnimationId TR_ANIMATION_LARA_ROPE_IDLE_TO_BACKFLIP = 380;                    // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_TO_FALL_SEMIFRONT = 381;             // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_TO_FALL_MIDDLE = 382;                // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_TO_FALL_BACK = 383;                  // Unused

constexpr AnimationId TR_ANIMATION_LARA_ROPE_IDLE_TO_SWING_SEMIMIDDLE = 388;            // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_IDLE_TO_SWING_HALFMIDDLE = 389;            // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_TO_FALL_FRONT = 390;                 // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_GRAB_TO_FALL_ALTERNATE = 391;              // Unused

constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_FORWARD_SEMIHARD = 394;              // The only one used!
constexpr AnimationId TR_ANIMATION_LARA_ROPE_LADDER_TO_HANDS_DOWN_ALTERNATE = 395;      // Unused, make it work? (used in the TR4 demo if I'm right?) (then you will need to remove all the StateID changes related to the rope animations)
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_BACK_CONTINUE = 396;                 // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_BACK_END = 397;                      // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_BACK_BEGIN = 398;                    // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_FORWARD_SOFT = 399;                  // Unused

constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_FORWARD_HARD = 404;                   // Not found... Uhh, unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_CHANGE_ROPE = 405;                          // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_TO_TRY_HANG_FRONT2 = 406;             // Not sure it's used?
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_TO_TRY_HANG_MIDDLE = 407;             // Not sure it's used?
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_BLOCK = 408;                          // Unused
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_TO_TRY_HANG_SEMIMIDDLE = 409;         // Not sure it's used?
constexpr AnimationId TR_ANIMATION_LARA_ROPE_SWING_TO_TRY_HANG_FRONT3 = 410;             // Not sure it's used?

constexpr AnimationId TR_ANIMATION_LARA_DOUBLEDOORS_PUSH = 412;
constexpr AnimationId TR_ANIMATION_LARA_BIG_BUTTON_PUSH = 413;
constexpr AnimationId TR_ANIMATION_LARA_JUMPSWITCH = 414;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_PULLEY = 415;
constexpr AnimationId TR_ANIMATION_LARA_UNDERWATER_DOOR_OPEN = 416;
constexpr AnimationId TR_ANIMATION_LARA_PUSHABLE_PUSH_TO_STAND = 417;
constexpr AnimationId TR_ANIMATION_LARA_PUSHABLE_PULL_TO_STAND = 418;
constexpr AnimationId TR_ANIMATION_LARA_CROWBAR_USE_ON_WALL = 419;
constexpr AnimationId TR_ANIMATION_LARA_CROWBAR_USE_ON_FLOOR = 420;
constexpr AnimationId TR_ANIMATION_LARA_CRAWL_JUMP_DOWN = 421;
constexpr AnimationId TR_ANIMATION_LARA_HARP_PLAY = 422;
constexpr AnimationId TR_ANIMATION_LARA_PUT_TRIDENT = 423;
constexpr AnimationId TR_ANIMATION_LARA_PICKUP_PEDESTAL_HIGH = 424;
constexpr AnimationId TR_ANIMATION_LARA_PICKUP_PEDESTAL_LOW = 425;
constexpr AnimationId TR_ANIMATION_LARA_ROTATE_SENET = 426;
constexpr AnimationId TR_ANIMATION_LARA_TORCH_LIGHT_1 = 427;
constexpr AnimationId TR_ANIMATION_LARA_TORCH_LIGHT_2 = 428;
constexpr AnimationId TR_ANIMATION_LARA_TORCH_LIGHT_3 = 429;
constexpr AnimationId TR_ANIMATION_LARA_TORCH_LIGHT_4 = 430;
constexpr AnimationId TR_ANIMATION_LARA_TORCH_LIGHT_5 = 431;
constexpr AnimationId TR_ANIMATION_LARA_DETONATOR_USE = 432;

constexpr AnimationId TR_ANIMATION_LARA_CORRECT_POSITION_FRONT = 433;           // Unused
constexpr AnimationId TR_ANIMATION_LARA_CORRECT_POSITION_LEFT = 434;            // Unused
constexpr AnimationId TR_ANIMATION_LARA_CORRECT_POSITION_RIGHT = 435;           // Unused

constexpr AnimationId TR_ANIMATION_LARA_CROWBAR_USE_ON_FLOOR_FAIL = 436;        // Unused
constexpr AnimationId TR_ANIMATION_LARA_tr4_DEATH_MAGIC_tr5_USE_KEYCARD = 437;  // Unused?
constexpr AnimationId TR_ANIMATION_LARA_DEATH_BLOWUP = 438;
constexpr AnimationId TR_ANIMATION_LARA_PICKUP_SARCOPHAGUS = 439;
constexpr AnimationId TR_ANIMATION_LARA_DRAG = 440;
constexpr AnimationId TR_ANIMATION_LARA_BINOCULARS = 441;
constexpr AnimationId TR_ANIMATION_LARA_DEATH_BIG_SCORPION = 442;
constexpr AnimationId TR_ANIMATION_LARA_tr4_DEATH_SETH_tr5_ELEVATOR_SMASH = 443;
constexpr AnimationId TR_ANIMATION_LARA_BEETLE_PUT = 444;

// TR5 AND ABOVE (445-473)

constexpr AnimationId TR_ANIMATION_LARA_ELEVATOR_RECOVER = 443;
constexpr AnimationId TR_ANIMATION_LARA_DOZY = 445;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_WALK = 446;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_WALK_TO_STAND = 447;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_STAND = 448;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_WALK_TO_STAND_CAREFUL = 449;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_STAND_TO_WALK = 450;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_TURN = 451;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_LOOSE_LEFT = 452;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_RECOVER_LEFT = 453;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_FALL_LEFT = 454;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_LOOSE_RIGHT = 455;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_RECOVER_RIGHT = 456;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_FALL_RIGHT = 457;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_START = 458;
constexpr AnimationId TR_ANIMATION_LARA_TIGHTROPE_FINISH = 459;
constexpr AnimationId TR_ANIMATION_LARA_DOVESWITCH_TURN = 460;
constexpr AnimationId TR_ANIMATION_LARA_BARS_GRAB = 461;
constexpr AnimationId TR_ANIMATION_LARA_BARS_SWING = 462;
constexpr AnimationId TR_ANIMATION_LARA_BARS_JUMP = 463;
constexpr AnimationId TR_ANIMATION_LARA_LOOT_CABINET = 464;
constexpr AnimationId TR_ANIMATION_LARA_LOOT_DRAWER = 465;
constexpr AnimationId TR_ANIMATION_LARA_LOOT_SHELF = 466;
constexpr AnimationId TR_ANIMATION_LARA_RADIO_BEGIN = 467;
constexpr AnimationId TR_ANIMATION_LARA_RADIO_IDLE = 468;
constexpr AnimationId TR_ANIMATION_LARA_RADIO_END = 469;
constexpr AnimationId TR_ANIMATION_LARA_VALVE_TURN = 470;
constexpr AnimationId TR_ANIMATION_LARA_CROWBAR_USE_ON_WALL2 = 471;
constexpr AnimationId TR_ANIMATION_LARA_LOOT_CHEST = 472;
constexpr AnimationId TR_ANIMATION_LARA_LADDER_TO_CROUCH = 473;
}
}
