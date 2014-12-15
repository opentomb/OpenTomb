-- OPENTOMB TRIGGER SCRIPT
-- FOR TOMB RAIDER 1, LEVEL2

-- disable / enable entities
-- wolves
setEntityActivity(6, 0);
setEntityActivity(13, 0);
setEntityActivity(14, 0);
setEntityActivity(17, 0);
setEntityActivity(18, 0);
setEntityActivity(21, 0);
setEntityActivity(40, 0);
setEntityActivity(41, 0);
setEntityActivity(42, 0);
setEntityActivity(43, 0);
setEntityActivity(44, 0);

-- trap axes
setEntityActivity(56, 0);
setEntityActivity(57, 0);
setEntityActivity(58, 0);
setEntityAnim(56, 2, 0);
setEntityAnim(57, 2, 0);
setEntityAnim(58, 2, 0);

-- autoopen doors (in level start)
setEntityActivity(92, 0);
setEntityState(92, 1);
setEntityActivity(94, 0);
setEntityState(94, 1);

--------------------------------------------------------------------------------
-------------------- ADDITIONAL SWITCHES FUNCTIONS -----------------------------
--------------------------------------------------------------------------------

print("LEVEL2_autoexec.lua loaded");
