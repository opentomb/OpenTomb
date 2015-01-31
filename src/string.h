#ifndef ENGINE_STRINGS_H
#define ENGINE_STRINGS_H

#include <stdint.h>

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
#define STR_GEN_DIARY               19
#define STR_GEN_PAUSED              20
#define STR_GEN_STATISTICS          21
#define STR_GEN_LOADING             22
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


// String conversion preload size is used when decoding UTF-8 string
// to UTF-32 string. To avoid reallocating memory zone each parsed
// symbol, we specify preload buffer size after which memory will be
// indeed reallocated. Average string size in TR generic strings is
// 16 symbols - hence, 16 dwords is an average preload size.

#define STR_CONV_PRELOAD_SIZE       16

// Makes UTF-32 string out of UTF-8 string and returns pointer to
// new created UTF-32 string. Also passes absolute string length
// into string_length argument.

uint32_t* String_MakeUTF32(const char* utf8_string, uint32_t* string_length);

// Converts UTF-8 character in sequence to UTF32 character, also returning
// a pointer to the next UTF-8 character in sequence.

char*     String_UTF8char_to_UTF32char(char* utf8, uint32_t* utf32);

#endif
