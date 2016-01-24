#include "audio.h"

#include <SDL2/SDL.h>

#include "engine/engine.h"
#include "gui/console.h"

#include <boost/log/trivial.hpp>

using gui::Console;

namespace audio
{
// ======== Audio source global methods ========

bool checkALError(const char* error_marker)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        BOOST_LOG_TRIVIAL(warning) << "OpenAL error: " << alGetString(err) << " (" << error_marker << ")";
        return true;
    }
    return false;
}

void logSndfileError(int code)
{
    BOOST_LOG_TRIVIAL(error) << "Sndfile error: " << sf_error_number(code);
}
} // namespace audio