#include "audio.h"

#include <SDL2/SDL.h>

#include "engine/engine.h"
#include "gui/console.h"

#include <boost/log/trivial.hpp>

using gui::Console;

namespace audio
{
// ======== Audio source global methods ========

bool checkALError(const char* func, int line)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        const char* errStr = alGetString(err);
        if(errStr == nullptr)
            errStr = "<unknown>";

        BOOST_LOG_TRIVIAL(warning) << "OpenAL error (in " << func << ":" << line << "): " << errStr;
        return true;
    }
    return false;
}

void logSndfileError(int code)
{
    BOOST_LOG_TRIVIAL(error) << "Sndfile error: " << sf_error_number(code);
}
} // namespace audio
