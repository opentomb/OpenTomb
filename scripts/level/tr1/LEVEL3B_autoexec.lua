-- OPENTOMB TRIGGER SCRIPT
-- FOR TOMB RAIDER 1, LEVEL3B

-- disable / enable entities
-- wolves
--setEntityActivity(6, 0);

-- trap axes


-- autoopen doors (in level start)
setEntityActivity(75, 0);
setEntityState(75, 1);

--------------------------------------------------------------------------------
-------------------- ADDITIONAL SWITCHES FUNCTIONS -----------------------------
--------------------------------------------------------------------------------

print("LEVEL3B_autoexec.lua loaded");
