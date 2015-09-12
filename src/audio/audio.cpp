#include "audio.h"

#include <chrono>
#include <cstdio>

#include <SDL2/SDL.h>

#include "engine/engine.h"
#include "engine/system.h"
#include "fxmanager.h"
#include "gui/console.h"
#include "render/render.h"
#include "script/script.h"
#include "settings.h"
#include "strings.h"
#include "util/helpers.h"
#include "util/vmath.h"
#include "world/camera.h"
#include "world/character.h"
#include "world/entity.h"
#include "world/room.h"

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
