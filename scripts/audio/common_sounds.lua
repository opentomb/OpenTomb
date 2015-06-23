-- OPENTOMB GLOBAL SOUND ID MAPPER SCRIPT
-- by Lwmte, May 2014

--------------------------------------------------------------------------------
-- All TR games shared some sound sample indexes. Mostly, it's generic and menu
-- sounds. While sound samples themselves may have changed, index always remain
-- the same. Here, all these effects are explicitly defined, to ease out
-- future scripting life.
--------------------------------------------------------------------------------

SOUND_NO          = 2
SOUND_LANDING     = 4
SOUND_HOLSTEROUT  = 6
SOUND_HOLSTERIN   = 7
SOUND_SHOTPISTOLS = 8
SOUND_RELOAD      = 9
SOUND_RICOCHET    = 10
SOUND_LARASCREAM  = 30
SOUND_LARAINJURY  = 31
SOUND_SPLASH      = 33
SOUND_FROMWATER   = 34
SOUND_SWIM        = 35
SOUND_LARABREATH  = 36
SOUND_BUBBLE      = 37
SOUND_USEKEY      = 39
SOUND_GEN_DEATH   = 41
SOUND_SHOTUZI     = 43
SOUND_SHOTSHOTGUN = 45
SOUND_UNDERWATER  = 60
SOUND_MENUROTATE  = 108
SOUND_MENUSELECT  = 109
SOUND_MENUOPEN    = 111
SOUND_MENUCLOSE   = 112  -- Only used in TR1-3.
SOUND_MENUCLANG   = 114
SOUND_MENUPAGE    = 115
SOUND_MEDIPACK    = 116
SOUND_IMPALE      = 145

--------------------------------------------------------------------------------
-- Certain sound effect indexes were changed across different TR versions,
-- despite remaining the same - mostly, it happened with menu sounds and some
-- general sounds. For such effects, we need this remap script, which will
-- redirect engine to proper sound FX for each game version.
-- Please note that indexes passed from engine are hardcoded in audio.h file,
-- TR_AUDIO_SOUND_GLOBALID enum list. It's hardly not recommended to change it.
--------------------------------------------------------------------------------

GLOBALID_MENUOPEN       = 0;
GLOBALID_MENUCLOSE      = 1;
GLOBALID_MENUROTATE     = 2;
GLOBALID_MENUPAGE       = 3;
GLOBALID_MENUSELECT     = 4;
GLOBALID_MENUWEAPON     = 5;
GLOBALID_MENUCLANG      = 6;
GLOBALID_MENUTEST       = 7;
GLOBALID_MOVINGWALL     = 8;
GLOBALID_SPIKEHIT       = 9;

tr1_global_sounds = {};
tr2_global_sounds = {};
tr3_global_sounds = {};
tr4_global_sounds = {};
tr5_global_sounds = {};

tr1_global_sounds[GLOBALID_MENUOPEN]    = 111;
tr1_global_sounds[GLOBALID_MENUCLOSE]   = 112;
tr1_global_sounds[GLOBALID_MENUROTATE]  = 108;
tr1_global_sounds[GLOBALID_MENUPAGE]    = 115;
tr1_global_sounds[GLOBALID_MENUWEAPON]  = 114;
tr1_global_sounds[GLOBALID_MENUCLANG]   = 113;
tr1_global_sounds[GLOBALID_MENUSELECT]  = 110;
tr1_global_sounds[GLOBALID_MENUTEST]    = 8;
tr1_global_sounds[GLOBALID_MOVINGWALL]  = -1;     -- No sound
tr1_global_sounds[GLOBALID_SPIKEHIT]    = -1;

tr2_global_sounds[GLOBALID_MENUOPEN]    = 111;
tr2_global_sounds[GLOBALID_MENUCLOSE]   = 112;
tr2_global_sounds[GLOBALID_MENUROTATE]  = 108;
tr2_global_sounds[GLOBALID_MENUPAGE]    = 115;
tr2_global_sounds[GLOBALID_MENUWEAPON]  = 114;
tr2_global_sounds[GLOBALID_MENUCLANG]   = 114;    -- 113 is a looped clock sound.
tr2_global_sounds[GLOBALID_MENUSELECT]  = 112;
tr2_global_sounds[GLOBALID_MENUTEST]    = 8;
tr2_global_sounds[GLOBALID_MOVINGWALL]  = 204;
tr2_global_sounds[GLOBALID_SPIKEHIT]    = 205;

tr3_global_sounds[GLOBALID_MENUOPEN]    = 111;
tr3_global_sounds[GLOBALID_MENUCLOSE]   = 112;
tr3_global_sounds[GLOBALID_MENUROTATE]  = 108;
tr3_global_sounds[GLOBALID_MENUPAGE]    = 115;
tr3_global_sounds[GLOBALID_MENUWEAPON]  = 114;
tr3_global_sounds[GLOBALID_MENUCLANG]   = 114;    -- There is no CLANG sound in TR3.
tr3_global_sounds[GLOBALID_MENUSELECT]  = 109;    -- Lara's Home photo sound was replaced by this.
tr3_global_sounds[GLOBALID_MENUTEST]    = 8;
tr3_global_sounds[GLOBALID_MOVINGWALL]  = 147;
tr3_global_sounds[GLOBALID_SPIKEHIT]    = 56;

tr4_global_sounds[GLOBALID_MENUOPEN]    = 109; 
tr4_global_sounds[GLOBALID_MENUCLOSE]   = 109;
tr4_global_sounds[GLOBALID_MENUROTATE]  = 108;
tr4_global_sounds[GLOBALID_MENUPAGE]    = 111;    -- No passport in TR4/5 menu system, hence no sound.
tr4_global_sounds[GLOBALID_MENUWEAPON]  = 9;      -- There was no WEAPON sound for TR4/5 menu system.
tr4_global_sounds[GLOBALID_MENUCLANG]   = 114;    -- Now used for COMBINE item.
tr4_global_sounds[GLOBALID_MENUSELECT]  = 111;
tr4_global_sounds[GLOBALID_MENUTEST]    = 8;
tr4_global_sounds[GLOBALID_MOVINGWALL]  = -1;
tr4_global_sounds[GLOBALID_SPIKEHIT]    = -1;

tr5_global_sounds[GLOBALID_MENUOPEN]    = 109;
tr5_global_sounds[GLOBALID_MENUCLOSE]   = 109;
tr5_global_sounds[GLOBALID_MENUROTATE]  = 108;
tr5_global_sounds[GLOBALID_MENUPAGE]    = 111;
tr5_global_sounds[GLOBALID_MENUWEAPON]  = 9;
tr5_global_sounds[GLOBALID_MENUCLANG]   = 114;
tr5_global_sounds[GLOBALID_MENUSELECT]  = 111;
tr5_global_sounds[GLOBALID_MENUTEST]    = 8;
tr5_global_sounds[GLOBALID_MOVINGWALL]  = -1;
tr5_global_sounds[GLOBALID_SPIKEHIT]    = -1;


function getGlobalSound(ver, id)
    local result;
    if(ver < 3) then                    -- TR_I, TR_I_DEMO, TR_I_UB
        result = tr1_global_sounds[id];
    elseif(ver < 5) then                -- TR_II, TR_II_DEMO
        result = tr2_global_sounds[id];
    elseif(ver < 6) then                -- TR_III
        result = tr3_global_sounds[id];
    elseif(ver < 8) then                -- TR_IV, TR_IV_DEMO
        result = tr4_global_sounds[id];
    elseif(ver < 9) then                -- TR_V
        result = tr5_global_sounds[id];
    else
        result = -1;
    end;
    
    if(result ~= nil) then return result else return -1 end;
end;