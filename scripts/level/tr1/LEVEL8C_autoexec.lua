
-- OPENTOMB TRIGGER SCRIPT
-- FOR TOMB RAIDER 1, LEVEL12

-- autoopen doors (in level start)
setEntityActivity(58, 0);
setEntityState(58, 1);
setEntityActivity(59, 0);
setEntityState(59, 1);

playstream(59)