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

dofile("scripts/strings/generic/english.lua");

function getString(id)
   if(strings[id] ~= nil) then
    return strings[id];
   else
    return "MISSING";
   end;
end;