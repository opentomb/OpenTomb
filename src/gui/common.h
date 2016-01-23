#pragma once

namespace gui
{

constexpr int MaxTempLines = 256;

// Screen metering resolution specifies user-friendly relative dimensions of screen,
// which are not dependent on screen resolution. They're primarily used to parse
// bar and string dimensions.

constexpr float ScreenMeteringResolution = 1000.0f;

constexpr int MaxFonts = 8;     // 8 fonts is PLENTY.

constexpr int MinFontSize = 1;
constexpr int MaxFontSize = 72;

constexpr float FontFadeSpeed = 1.0f;                 // Global fading style speed.
constexpr float FontFadeMin   = 0.3f;                 // Minimum fade multiplier.

constexpr float FontShadowTransparency    =  0.7f;
constexpr float FontShadowVerticalShift   = -0.9f;
constexpr float FontShadowHorizontalShift =  0.7f;

// Default line size is generally used for static in-game strings. Strings
// that are created dynamically may have variable string sizes.

constexpr int LineDefaultSize = 128;

// Anchoring is needed to link specific GUI element to specific screen position,
// independent of screen resolution and aspect ratio. Vertical and horizontal
// anchorings are seperated, so you can link element at any place - top, bottom,
// center, left or right.
enum class VerticalAnchor
{
    Top,
    Bottom,
    Center
};

enum class HorizontalAnchor
{
    Left,
    Right,
    Center
};

// Offscreen divider specifies how far item notifier will be placed from
// the final slide position. Usually it's enough to be 1/8 of the screen
// width, but if you want to increase or decrease notifier size, you must
// change this value properly.

constexpr float NotifierOffscreenDivider = 8.0f;

} // namespace gui
