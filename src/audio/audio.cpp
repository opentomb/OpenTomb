#include "audio.h"

#include <chrono>

#include <SDL2/SDL.h>

#include "engine/engine.h"
#include "engine/system.h"
#include "gui/console.h"

using gui::Console;

namespace audio
{

// ======== Audio source global methods ========

bool checkALError(const char* error_marker)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        engine::Sys_DebugLog(LOG_FILENAME, "OpenAL error: %s / %s", alGetString(err), error_marker);
        return true;
    }
    return false;
}

void logSndfileError(int code)
{
    engine::Sys_DebugLog(LOG_FILENAME, sf_error_number(code));
}

} // namespace audio
