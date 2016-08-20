addCharacterHair(player, HAIR_TR3);

moveEntityLocal(player,0,0,256);            -- Raise Lara from the floor
setEntityMoveType(player, MOVE_UNDERWATER); -- Change to underwater state
setEntityAnim(player, ANIM_TYPE_BASE, 108, 0); -- Force underwater animation

playStream(32)