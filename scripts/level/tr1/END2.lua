-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER UB, END2

print("Level script loaded (END2.lua)");

level_PostLoad = function()
    addRoomToOverlappedList(29, 0);
    addRoomToOverlappedList(0, 29);
end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;
