-- OPENTOMB FLIPEFFECT TRIGGER FUNCTION SCRIPT
-- by Lwmte, April 2015

--------------------------------------------------------------------------------
-- In this file you can put extra flipeffect functions which act in the same way
-- as Flipeffect Editor in TREP - all flipeffects above 47 are hardcoded, and
-- remaining ones are refering to this function table. Only difference is, if
-- there IS flipeffect above 47 in the table, then table function is used
-- instead of hardcoded one; it is needed for some version-specific flipeffects.
--------------------------------------------------------------------------------

flipeffects = {};   -- Initialize flipeffect function array.

-- Flipeffects implementation

-- Shake camera
flipeffects[1] = function(parameter) 
    camShake(15, 1);
end;

-- Play desired sound ID
flipeffects[10] = function(parameter) 
    playSound(parameter);
end;

-- Play explosion sound and effect
flipeffects[11] = function(parameter)
    flashSetup(255, 255,220,80, 10,600);
    flashStart();
    playSound(105);
end;

-- room flickering effect: WORKAROUND
flipeffects[16] = function(parameter)
    setFlipMap(0x00, 0x00, TRIGGER_OP_XOR);    --setFlipMap(flip_index, flip_mask, TRIGGER_OP_OR / XOR)
    setFlipState(0x00, 0);                     --setFlipState(flip_index, FLIP_STATE_ON)
end;