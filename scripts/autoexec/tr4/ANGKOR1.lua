function autoexec_PreLoad()
    UVRotate = 8;
    tr4_entity_tbl[453] = {coll = COLLISION_TYPE_NONE};
    tr4_entity_tbl[455] = {coll = COLLISION_TYPE_NONE};
end;

function autoexec_PostLoad()

    addCharacterHair(player, HAIR_TR4_KID_1);
    addCharacterHair(player, HAIR_TR4_KID_2);

    playStream(110);
end;