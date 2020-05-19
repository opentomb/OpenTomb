project(OpenAL)

find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)

set(OPENAL_SRCS
    Alc/alstring.h
    Alc/hrtf.h
    Alc/vector.h
    Alc/midi/base.h
    Alc/compat.h
    Alc/mixer_defs.h
    Alc/backends/base.h
    Alc/evtqueue.h
    alEffect.h
    alMain.h
    alAuxEffectSlot.h
    static_assert.h
    alu.h
    alListener.h
    threads.h
    AL/efx.h
    AL/efx-creative.h
    AL/efx-presets.h
    AL/alc.h
    AL/al.h
    AL/alext.h
    alMidi.h
    alSource.h
    uintmap.h
    sample_cvt.h
    config.h
    bool.h
    align.h
    alBuffer.h
    alThunk.h
    atomic.h
    bs2b.h
    alFilter.h
    rwlock.h
    alError.h
    Alc/alcRing.c
    Alc/mixer_sse2.c
    Alc/mixer.c
    Alc/ALu.c
    Alc/ALc.c
    Alc/mixer_c.c
    Alc/mixer_neon.c
    Alc/midi/soft.c
    Alc/midi/dummy.c
    Alc/midi/sf2load.c
    Alc/midi/fluidsynth.c
    Alc/midi/base.c
    Alc/effects/chorus.c
    Alc/effects/equalizer.c
    Alc/effects/reverb.c
    Alc/effects/modulator.c
    Alc/effects/dedicated.c
    Alc/effects/echo.c
    Alc/effects/null.c
    Alc/effects/autowah.c
    Alc/effects/distortion.c
    Alc/effects/compressor.c
    Alc/effects/flanger.c
    Alc/panning.c
    Alc/mixer_sse41.c
    Alc/mixer_sse.c
    Alc/helpers.c
    Alc/alcConfig.c
    Alc/hrtf.c
    Alc/bs2b.c
    Alc/backends/loopback.c
    Alc/backends/backend.c
    Alc/backends/wave.c
    Alc/backends/null.c
    Alc/backends/base.c
    common/atomic.c
    common/threads.c
    common/rwlock.c
    common/uintmap.c
    OpenAL32/alThunk.c
    OpenAL32/alEffect.c
    OpenAL32/alFontsound.c
    OpenAL32/alState.c
    OpenAL32/alFilter.c
    OpenAL32/alExtension.c
    OpenAL32/alSoundfont.c
    OpenAL32/alListener.c
    OpenAL32/alMidi.c
    OpenAL32/alError.c
    OpenAL32/alSource.c
    OpenAL32/alAuxEffectSlot.c
    OpenAL32/sample_cvt.c
    OpenAL32/alBuffer.c
    OpenAL32/alPreset.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SDL2_INCLUDE_DIR})

add_library(${PROJECT_NAME} STATIC ${OPENAL_SRCS})

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
