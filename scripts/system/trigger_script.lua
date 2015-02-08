trigger_list = {};

function tlist_Clear()
    for k,v in pairs(trigger_list) do trigger_list[k] = nil end;
end

function tlist_AddTrigger(i,f)
    trigger_list[i] = {func = f};
end

function tlist_RunTrigger(index)
    if((trigger_list[index] ~= nil) and (trigger_list[index].func ~= nil)) then
        trigger_list[index].done = 1;
        return trigger_list[index].func();
    else
        retutn 0;
    end;
end;

function tlist_StopTrigger(index)
    if((trigger_list[index] ~= nil) and (trigger_list[index].antifunc ~= nil)) then
        trigger_list[index].done = 0;
        return trigger_list[index].antifunc();
end;