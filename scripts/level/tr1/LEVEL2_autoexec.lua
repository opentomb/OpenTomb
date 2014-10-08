-- OPENTOMB TRIGGER SCRIPT
-- FOR TOMB RAIDER 1, LEVEL2 - 

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

function switch_84(state)                       -- trapdoors switch
    if(state == 1) then                         -- switch is turned on
        setEntityAnim(60, 3, 0);                -- turn up switch
        setEntityAnim(61, 1, 0);                -- start door closing
        setEntityAnim(62, 1, 0);                -- start door closing
        setEntityAnim(59, 3, 0);                -- start door opening

        setEntityAnim(56, 1, 0);                -- deactivate axes
        setEntityAnim(57, 1, 0);
        setEntityAnim(58, 1, 0);
    end
end


create_switch_func(84, {59}, switch_84); -- stop traps switch; there are additional funcs

entity_funcs[6] =  {}; entity_funcs[6].activate  = activate_wolves;
entity_funcs[13] = {}; entity_funcs[13].activate = activate_wolves;
entity_funcs[14] = {}; entity_funcs[14].activate = activate_wolves;
entity_funcs[17] = {}; entity_funcs[17].activate = activate_wolves;
entity_funcs[18] = {}; entity_funcs[18].activate = activate_wolves;
entity_funcs[21] = {}; entity_funcs[21].activate = activate_wolves;
entity_funcs[40] = {}; entity_funcs[40].activate = activate_wolves;
entity_funcs[41] = {}; entity_funcs[41].activate = activate_wolves;
entity_funcs[42] = {}; entity_funcs[42].activate = activate_wolves;
entity_funcs[43] = {}; entity_funcs[43].activate = activate_wolves;
entity_funcs[44] = {}; entity_funcs[44].activate = activate_wolves;

entity_funcs[56] = {}; entity_funcs[56].activate = activate_trap_axes;
entity_funcs[57] = {}; entity_funcs[57].activate = activate_trap_axes;
entity_funcs[58] = {}; entity_funcs[58].activate = activate_trap_axes;

print("LEVEL2_autoexec.lua loaded");
