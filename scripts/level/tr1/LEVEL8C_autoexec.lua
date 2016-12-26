playStream(59)

sectorAddTrigger(62, 9, 3, 0, TR_FD_TRIGTYPE_PICKUP, 0x1F, 0 , 0);              -- (room_id, index_x, index_y, function, sub_function, mask, once, timer)" 
sectorAddTriggerCommand(62, 9, 3, TR_FD_TRIGFUNC_OBJECT, 0x49, 0);
sectorAddTriggerCommand(62, 9, 3, TR_FD_TRIGFUNC_ENDLEVEL, 0, 0);               -- WORKAROUND: play cutscene first!
