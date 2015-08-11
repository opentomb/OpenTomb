function autoexec_PostLoad()
    moveEntityLocal(player,0,0,256);
    setEntityMoveType(player, MOVE_UNDERWATER);
    setEntityAnim(player, 108);
    playStream(59);
    print("LEVEL6_autoexec loaded");
end;