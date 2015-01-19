-- OPENTOMB FONT GENERATION SCRIPT
-- by Lwmte, Jan 2015

--------------------------------------------------------------------------------
-- In this script, we define all in-game fonts and styles. You can define as
-- much styles as you want (out of pre-defined bounds), but you won't be able
-- to generate more than three pre-defined types of fonts.
--------------------------------------------------------------------------------

-- This is pre-defined font index.

FONT_PRIMARY    = 0;
FONT_SECONDARY  = 1;
FONT_CONSOLE    = 2;

--------------------------------------------------------------------------------

-- This is pre-defined style index.

FONTSTYLE_CONSOLE_INFO          = 0;
FONTSTYLE_CONSOLE_WARNING       = 1;
FONTSTYLE_CONSOLE_EVENT         = 2;
FONTSTYLE_CONSOLE_NOTIFY        = 3;
FONTSTYLE_MENU_TITLE            = 4;
FONTSTYLE_MENU_HEADING1         = 5;
FONTSTYLE_MENU_HEADING2         = 6;
FONTSTYLE_MENU_ITEM_ACTIVE      = 7;
FONTSTYLE_MENU_ITEM_INACTIVE    = 8;
FONTSTYLE_MENU_CONTENT          = 9;
FONTSTYLE_STATS_TITLE           = 10;
FONTSTYLE_STATS_CONTENT         = 11;
FONTSTYLE_NOTIFIER              = 12;
FONTSTYLE_SAVEGAMELIST          = 13;
FONTSTYLE_GENERIC               = 14;

--------------------------------------------------------------------------------

-- If you want to change a font, change it here.
-- Syntax is: addFont(FONT_INDEX, FONT_PATH, FONT_SIZE);
-- NB: All fonts are open-source, free and GPL-compatible, see licenses in fonts
-- folder.

addFont(FONT_PRIMARY,   "resource/fonts/RobotoCondensed-Regular.ttf",   20);
addFont(FONT_SECONDARY, "resource/fonts/Roboto-Regular.ttf",            18);
addFont(FONT_CONSOLE,   "resource/fonts/DroidSansMono.ttf",             16);

--------------------------------------------------------------------------------

-- If you want to modify font style, change it here.
-- Syntax is:
-- addFontStyle(STYLE, R, G, B, A, SHD, FAD, RCT, RBD, RR, RG, RB, RA, HIDDEN)
-- where STYLE is unique style index,
--       R, G, B are font colours, A is font transparency,
--       SHD is shadowed flag, FAD is TR4-5 style fading flag,
--       RCT is auto-rect flag, RBD is rect border flag, RR, RG and RB are rect colours, RA is rect transparency.
--       HIDE is hide flag. Only text is hidden, rect is left as is.

---------------------------------------------------------------------------------------------------------------------
--------- STYLE ---------------------------- R -- G -- B -- A -- SHD -- FAD -- RCT - RBD - RR - RG - RB - RA - HIDE -
---------------------------------------------------------------------------------------------------------------------
addFontStyle(FONTSTYLE_CONSOLE_INFO,        0.9, 0.9, 0.9, 1.0, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_CONSOLE_WARNING,     1.0, 0.7, 0.7, 1.0, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_CONSOLE_EVENT,       0.7, 0.7, 1.0, 1.0, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_CONSOLE_NOTIFY,      0.9, 0.9, 0.4, 1.0, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_MENU_TITLE,          1.0, 0.9, 0.7, 1.0, true , false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_MENU_HEADING1,       0.9, 0.9, 0.9, 1.0, true , false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_MENU_HEADING2,       0.8, 0.8, 0.8, 1.0, true , false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_MENU_ITEM_ACTIVE,    1.0, 0.9, 0.8, 1.0, true , true , false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_MENU_ITEM_INACTIVE,  0.9, 0.9, 0.9, 1.0, true , false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_MENU_CONTENT,        0.8, 0.8, 0.8, 1.0, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_STATS_TITLE,         0.7, 0.7, 0.7, 1.0, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_STATS_CONTENT,       0.8, 0.8, 0.8, 1.0, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_NOTIFIER,            1.0, 1.0, 1.0, 1.0, true , false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_SAVEGAMELIST,        0.9, 0.9, 0.9, 1.0, true , false, false, 0.0, 0.0, 0.0, 0.0, 0.0, false);
addFontStyle(FONTSTYLE_GENERIC,             1.0, 1.0, 1.0, 0.9, true , false, false, 0.0, 0.0, 0.0, 0.0, 0.4, false);
---------------------------------------------------------------------------------------------------------------------
