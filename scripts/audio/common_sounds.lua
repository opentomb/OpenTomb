-- OPENTOMB GLOBAL SOUND ID MAPPER SCRIPT
-- by Lwmte, May 2014

tr1_global_sounds = {};
tr2_global_sounds = {};
tr3_global_sounds = {};
tr4_global_sounds = {};
tr5_global_sounds = {};

tr1_global_sounds[GlobalSoundId.MenuOpen]    = SoundId.TR123MenuOpen;
tr1_global_sounds[GlobalSoundId.MenuClose]   = SoundId.TR123MenuClose;
tr1_global_sounds[GlobalSoundId.MenuPage]    = SoundId.TR123MenuPage;
tr1_global_sounds[GlobalSoundId.MenuWeapon]  = SoundId.TR123MenuWeapon;
tr1_global_sounds[GlobalSoundId.MenuClang]   = SoundId.TR1MenuClang;
tr1_global_sounds[GlobalSoundId.MenuSelect]  = SoundId.TR13MenuSelect;

tr2_global_sounds[GlobalSoundId.MenuOpen]    = SoundId.TR123MenuOpen;
tr2_global_sounds[GlobalSoundId.MenuClose]   = SoundId.TR123MenuClose;
tr2_global_sounds[GlobalSoundId.MenuPage]    = SoundId.TR123MenuPage;
tr2_global_sounds[GlobalSoundId.MenuWeapon]  = SoundId.TR123MenuWeapon;
tr2_global_sounds[GlobalSoundId.MenuClang]   = SoundId.TR2345MenuClang;
tr2_global_sounds[GlobalSoundId.MenuSelect]  = SoundId.TR2MenuSelect;
tr2_global_sounds[GlobalSoundId.MovingWall]  = SoundId.TR2MovingWall;
tr2_global_sounds[GlobalSoundId.SpikeHit]    = SoundId.TR2SpikeHit;

tr3_global_sounds[GlobalSoundId.MenuOpen]    = SoundId.TR123MenuOpen;
tr3_global_sounds[GlobalSoundId.MenuClose]   = SoundId.TR123MenuClose;
tr3_global_sounds[GlobalSoundId.MenuPage]    = SoundId.TR123MenuPage;
tr3_global_sounds[GlobalSoundId.MenuWeapon]  = SoundId.TR123MenuWeapon;
tr3_global_sounds[GlobalSoundId.MenuClang]   = SoundId.TR2345MenuClang;
tr3_global_sounds[GlobalSoundId.MenuSelect]  = SoundId.TR13MenuSelect;
tr3_global_sounds[GlobalSoundId.MovingWall]  = SoundId.TR3MovingWall;
tr3_global_sounds[GlobalSoundId.SpikeHit]    = SoundId.TR3SpikeHit;

tr4_global_sounds[GlobalSoundId.MenuOpen]    = SoundId.TR45MenuOpenClose;
tr4_global_sounds[GlobalSoundId.MenuClose]   = SoundId.TR45MenuOpenClose;
tr4_global_sounds[GlobalSoundId.MenuPage]    = SoundId.TR45MenuPage;
tr4_global_sounds[GlobalSoundId.MenuWeapon]  = SoundId.TR45MenuWeapon;
tr4_global_sounds[GlobalSoundId.MenuClang]   = SoundId.TR2345MenuClang;
tr4_global_sounds[GlobalSoundId.MenuSelect]  = SoundId.TR45MenuSelect;

tr5_global_sounds[GlobalSoundId.MenuOpen]    = SoundId.TR45MenuOpenClose;
tr5_global_sounds[GlobalSoundId.MenuClose]   = SoundId.TR45MenuOpenClose;
tr5_global_sounds[GlobalSoundId.MenuPage]    = SoundId.TR45MenuPage;
tr5_global_sounds[GlobalSoundId.MenuWeapon]  = SoundId.TR45MenuWeapon;
tr5_global_sounds[GlobalSoundId.MenuClang]   = SoundId.TR2345MenuClang;
tr5_global_sounds[GlobalSoundId.MenuSelect]  = SoundId.TR45MenuSelect;


function getGlobalSound(id)
    local engine = getEngineVersion();
    if(engine == Engine.I) then
        return tr1_global_sounds[id];
    elseif(engine == Engine.II) then
        return tr2_global_sounds[id];
    elseif(engine == Engine.III) then
        return tr3_global_sounds[id];
    elseif(engine == Engine.IV) then
        return tr4_global_sounds[id];
    elseif(engine == Engine.V) then
        return tr5_global_sounds[id];
    else
        return nil;
    end;
end;
