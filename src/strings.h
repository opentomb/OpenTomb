#pragma once

#include <cstdint>

// Generic strings enumeration

#define STR_GEN_NEWGAME             0
#define STR_GEN_SELECTGAME          1
#define STR_GEN_SELECTLEVEL         2
#define STR_GEN_LARAHOME            3
#define STR_GEN_SAVEGAME            4
#define STR_GEN_LOADGAME            5
#define STR_GEN_OPTIONS             6
#define STR_GEN_QUIT                7
#define STR_GEN_RESTARTLEVEL        8
#define STR_GEN_EXITTOTITLE         9
#define STR_GEN_YES                 10
#define STR_GEN_NO                  11
#define STR_GEN_APPLY               12
#define STR_GEN_CANCEL              13
#define STR_GEN_PREVIOUS            14
#define STR_GEN_NEXT                15
#define STR_GEN_OK                  16
#define STR_GEN_DISCARD             17
#define STR_GEN_INVENTORY           18
#define STR_GEN_ITEMS               19
#define STR_GEN_PAUSED              20
#define STR_GEN_OPTIONS_TITLE       21
#define STR_GEN_STATISTICS          22
#define STR_GEN_EXITGAME            23
#define STR_GEN_SELECTTOLOAD        24
#define STR_GEN_SELECTTOSAVE        25
#define STR_GEN_SELECTTOCOMBINE     26
#define STR_GEN_EQUIP               27
#define STR_GEN_CHOOSEAMMO          28
#define STR_GEN_CHOOSEFIREMODE      29
#define STR_GEN_USE                 30
#define STR_GEN_COMBINE             31
#define STR_GEN_SEPARATE            32
#define STR_GEN_EXAMINE             33
#define STR_GEN_THROWAWAY           34
#define STR_GEN_HINT_ACCEPT         35
#define STR_GEN_HINT_CANCEL         36
#define STR_GEN_HINT_TOUCHINV       37
#define STR_GEN_HINT_SCROLLINV      38
#define STR_GEN_HINT_EXAMINEINV     39

#define STR_GEN_STATS_LOC           40
#define STR_GEN_STATS_SECRETS       41
#define STR_GEN_STATS_DISTANCE      42
#define STR_GEN_STATS_AMMOUSED      43
#define STR_GEN_STATS_KILLS         44
#define STR_GEN_STATS_MEDIUSED      45

#define STR_GEN_TIP_1               46
#define STR_GEN_TIP_2               47
#define STR_GEN_TIP_3               48
#define STR_GEN_TIP_4               49

#define STR_GEN_PRESSTOSKIP         50

#define STR_GEN_MASK_INVHEADER      90
#define STR_GEN_MASK_AMMOHEADER     91
#define STR_GEN_MASK_TIMER          92
#define STR_GEN_MASK_TIMECOUNT      93

#define STR_GEN_GAMENAME_TR1        100
#define STR_GEN_GAMENAME_TR1GOLD    101
#define STR_GEN_GAMENAME_TR2        200
#define STR_GEN_GAMENAME_TR2GOLD    201
#define STR_GEN_GAMENAME_TR3        300
#define STR_GEN_GAMENAME_TR3GOLD    301
#define STR_GEN_GAMENAME_TR4        400
#define STR_GEN_GAMENAME_TR4TIMES   401
#define STR_GEN_GAMENAME_TR5        500
#define STR_GEN_GAMENAME_CUSTOM     600


// Generic system warnings enumeration

#define SYSWARN_ENTER_ENTITY_ID          0
#define SYSWARN_WRONG_ARGS               1
#define SYSWARN_WRONG_ARGS_COUNT         2
#define SYSWARN_NO_ENTITY                3
#define SYSWARN_WRONG_OPTION_INDEX       4
#define SYSWARN_NO_CHARACTER             5
#define SYSWARN_WRONG_ROOM               6
#define SYSWARN_MODELID_OVERFLOW         7
#define SYSWARN_WRONG_ACTION_NUMBER      8
#define SYSWARN_CANT_CREATE_FONT         9
#define SYSWARN_CANT_CREATE_STYLE        10
#define SYSWARN_CANT_REMOVE_FONT         11
#define SYSWARN_CANT_REMOVE_STYLE        12
#define SYSWARN_NO_SKELETAL_MODEL        13
#define SYSWARN_WRONG_ANIM_NUMBER        14
#define SYSWARN_WRONG_DISPATCH_NUMBER    15
#define SYSWARN_WRONG_FRAME_NUMBER       16
#define SYSWARN_WRONG_STREAM_ID          17
#define SYSWARN_WRONG_SOUND_ID           18
#define SYSWARN_AS_NOCHANNEL             19
#define SYSWARN_AS_NOSAMPLE              20
#define SYSWARN_AS_IGNORED               21
#define SYSWARN_AK_NOTPLAYED             22
#define SYSWARN_NOT_ACTIVE_FLIPMAP       23
#define SYSWARN_FILE_NOT_FOUND           24
#define SYSWARN_NOT_TRUECOLOR_IMG        25
#define SYSWARN_IMG_NOT_LOADED_SDL       26
#define SYSWARN_BAD_FRAME_OFFSET         27
#define SYSWARN_CANT_OPEN_FILE           28
#define SYSWARN_BAD_FILE_FORMAT          29
#define SYSWARN_INVALID_LINECOUNT        30
#define SYSWARN_WRONG_FLIPMAP_INDEX      31
#define SYSWARN_NO_HAIR_SETUP            32
#define SYSWARN_CANT_CREATE_HAIR         33
#define SYSWARN_CANT_RESET_HAIR          34
#define SYSWARN_NO_RAGDOLL_SETUP         35
#define SYSWARN_CANT_CREATE_RAGDOLL      36
#define SYSWARN_CANT_REMOVE_RAGDOLL      37
#define SYSWARN_WAD_OUT_OF_BOUNDS        38
#define SYSWARN_WAD_SEEK_FAILED          39
#define SYSWARN_TRACK_OUT_OF_BOUNDS      40
#define SYSWARN_TRACK_ALREADY_PLAYING    41
#define SYSWARN_TRACK_WRONG_INDEX        42
#define SYSWARN_NO_FREE_STREAM           43
#define SYSWARN_STREAM_LOAD_ERROR        44
#define SYSWARN_STREAM_PLAY_ERROR        45
#define SYSWARN_WRONG_SECTOR_INFO        46
#define SYSWARN_WRONG_BONE_NUMBER        47
#define SYSWARN_IMAGE_NOT_LOADED         48
#define SYSWARN_WRONG_MODEL_ID           49
#define SYSWARN_WRONG_ENTITY_OR_BODY     50
#define SYSWARN_CANT_APPLY_FORCE         51



#define SYSNOTE_TRACK_OPENED             1000
#define SYSNOTE_READING_FILE             1001
#define SYSNOTE_GIVING_ITEM              1002
#define SYSNOTE_CHANGING_LEVEL           1003
#define SYSNOTE_CHANGING_GAME            1004
#define SYSNOTE_ENGINE_VERSION           1005
#define SYSNOTE_NUM_ROOMS                1006
#define SYSNOTE_NUM_TEXTURES             1007
#define SYSNOTE_CONSOLE_SPACING          1008
#define SYSNOTE_CONSOLE_LINECOUNT        1009
#define SYSNOTE_TRIGGER_INFO             1010
#define SYSNOTE_ACTIVATE_OBJECT          1011
#define SYSNOTE_LOADED_FADER             1012
#define SYSNOTE_TRIGGERS_CLEANED         1013
#define SYSNOTE_ENTFUNCS_CLEANED         1014
#define SYSNOTE_LOADED_PC_LEVEL          1015
#define SYSNOTE_LOADED_PSX_LEVEL         1016
#define SYSNOTE_LOADED_DC_LEVEL          1017
#define SYSNOTE_LOADED_OT_LEVEL          1018
#define SYSNOTE_WAD_PLAYING              1019
#define SYSNOTE_ENGINE_INITED            1020
#define SYSNOTE_LUA_STACK_INDEX          1021
#define SYSNOTE_LOADING_MAP              1022

#define SYSNOTE_COMMAND_HELP1            1023
#define SYSNOTE_COMMAND_HELP2            1024
#define SYSNOTE_COMMAND_HELP3            1025
#define SYSNOTE_COMMAND_HELP4            1026
#define SYSNOTE_COMMAND_HELP5            1027
#define SYSNOTE_COMMAND_HELP6            1028
#define SYSNOTE_COMMAND_HELP7            1029
#define SYSNOTE_COMMAND_HELP8            1030
#define SYSNOTE_COMMAND_HELP9            1031
#define SYSNOTE_COMMAND_HELP10           1032
#define SYSNOTE_COMMAND_HELP11           1033
#define SYSNOTE_COMMAND_HELP12           1034
#define SYSNOTE_COMMAND_HELP13           1035
#define SYSNOTE_COMMAND_HELP14           1036
#define SYSNOTE_COMMAND_HELP15           1037
