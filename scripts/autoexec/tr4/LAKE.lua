function autoexec_PostLoad()
    setEntityCollision(88, false);  -- model 457

    addCharacterHair(player, HAIR_TR4_OLD);
    playStream(110);

    print("LAKE_autoexec.lua loaded");
end;