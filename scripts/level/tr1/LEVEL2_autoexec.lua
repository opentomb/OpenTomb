-- OPENTOMB TRIGGER SCRIPT
-- FOR TOMB RAIDER 1, LEVEL2

create_trapfloor_func(23);
create_trapfloor_func(24);

create_trapfloor_func(64);
create_trapfloor_func(65);
create_trapfloor_func(66);
create_trapfloor_func(67);
create_trapfloor_func(68);
create_trapfloor_func(69);
create_trapfloor_func(70);

create_trapfloor_func(73);
create_trapfloor_func(74);
create_trapfloor_func(75);
create_trapfloor_func(76);
create_trapfloor_func(77);

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
