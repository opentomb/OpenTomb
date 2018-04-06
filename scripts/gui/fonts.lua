-- OPENTOMB FONT GENERATION SCRIPT
-- by Lwmte, Jan 2015

--------------------------------------------------------------------------------
-- In this script, we define all in-game fonts and styles. You can define as
-- much styles as you want (out of pre-defined bounds), but you won't be able
-- to generate more than three pre-defined types of fonts.
--------------------------------------------------------------------------------

-- This is pre-defined font index.

FONT_CONSOLE    = 0;
FONT_PRIMARY    = 1;
FONT_SECONDARY  = 2;

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

addFont(FONT_CONSOLE,   base_path .. "resource/fonts/DroidSansMono.ttf",             12);
addFont(FONT_PRIMARY,   base_path .. "resource/fonts/RobotoCondensed-Regular.ttf",   18);
addFont(FONT_SECONDARY, base_path .. "resource/fonts/Roboto-Regular.ttf",            14);

--------------------------------------------------------------------------------

-- If you want to modify font style, change it here.
-- Syntax is:
-- addFontStyle(STYLE, R, G, B, A, SHD)
-- where STYLE is unique style index,
--       R, G, B are font colours, A is font transparency,
--       SHD is shadowed flag, FAD is TR4-5 style fading flag

-----------------------------------------------------------------------
--------- STYLE ---------------------------- R -- G -- B -- A -- SHD 
-----------------------------------------------------------------------
addFontStyle(FONTSTYLE_CONSOLE_INFO,        0.9, 0.9, 0.9, 1.0, false);
addFontStyle(FONTSTYLE_CONSOLE_WARNING,     1.0, 0.7, 0.7, 1.0, false);
addFontStyle(FONTSTYLE_CONSOLE_EVENT,       0.7, 0.7, 1.0, 1.0, false);
addFontStyle(FONTSTYLE_CONSOLE_NOTIFY,      0.9, 0.9, 0.4, 1.0, false);
addFontStyle(FONTSTYLE_MENU_TITLE,          1.0, 0.9, 0.7, 1.0, true );
addFontStyle(FONTSTYLE_MENU_HEADING1,       0.9, 0.9, 0.9, 1.0, true );
addFontStyle(FONTSTYLE_MENU_HEADING2,       0.8, 0.8, 0.8, 1.0, true );
addFontStyle(FONTSTYLE_MENU_ITEM_ACTIVE,    1.0, 0.9, 0.8, 1.0, true );
addFontStyle(FONTSTYLE_MENU_ITEM_INACTIVE,  0.9, 0.9, 0.9, 1.0, true );
addFontStyle(FONTSTYLE_MENU_CONTENT,        0.8, 0.8, 0.8, 1.0, false);
addFontStyle(FONTSTYLE_STATS_TITLE,         0.7, 0.7, 0.7, 1.0, false);
addFontStyle(FONTSTYLE_STATS_CONTENT,       0.8, 0.8, 0.8, 1.0, false);
addFontStyle(FONTSTYLE_NOTIFIER,            1.0, 1.0, 1.0, 1.0, true );
addFontStyle(FONTSTYLE_SAVEGAMELIST,        0.9, 0.9, 0.9, 1.0, true );
addFontStyle(FONTSTYLE_GENERIC,             1.0, 1.0, 1.0, 0.9, true );
-----------------------------------------------------------------------
