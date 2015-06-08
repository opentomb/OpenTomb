-- OPENTOMB STRING ACCESS SCRIPT
-- by Lwmte, Jan 2015

--------------------------------------------------------------------------------
-- In this script, we define desired string array and function to get specific
-- string from this string array. Currently, english language is hardcoded here,
-- but later it could be converted to fully-functional switch language script.

-- IMPORTANT!!! If you want to make generic strings translation to your language
-- you MUST save your string array file in Unicode (UTF-8) format, or else it
-- won't be processed correctly.
--------------------------------------------------------------------------------

strings         = {};   -- Define string array.
sys_notify      = {};   -- Define system warnings array.

-- Load default (english) resources at first.

dofile("scripts/strings/english/generic.lua");
dofile("scripts/strings/english/global_items.lua");
dofile("scripts/strings/english/sys_notify.lua");


function setLanguage(lang_name)
    dofile("scripts/strings/" .. lang_name .. "/generic.lua");
    dofile("scripts/strings/" .. lang_name .. "/global_items.lua");
    dofile("scripts/strings/" .. lang_name .. "/sys_notify.lua");
    print("Language changed to " .. lang_name);
end;

function getString(id)
   if(strings[id] ~= nil) then
    return strings[id];
   else
    return "MISSING";
   end;
end;

function getSysNotify(id)
   if(sys_notify[id] ~= nil) then
    return sys_notify[id];
   else
    return "MISSING CONSOLE MESSAGE!";
   end;
end;