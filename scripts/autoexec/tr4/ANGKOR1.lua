function autoexec_PreLoad()

    UVRotate = 8;

end;

function autoexec_PostLoad()
    setEntityCollision(2, false);  -- model 455 - rays

    addCharacterHair(player, HAIR_TR4_KID_1);
    addCharacterHair(player, HAIR_TR4_KID_2);

    playStream(110);
end;