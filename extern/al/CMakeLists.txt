cmake_minimum_required(VERSION 3.2)

project(al1.1)

set(AL_SRCS
    OpenAL32/alAuxEffectSlot.c
    OpenAL32/alMidi.c
    OpenAL32/alBuffer.c
    OpenAL32/alPreset.c
    OpenAL32/alEffect.c
    OpenAL32/alSoundfont.c
    OpenAL32/alError.c
    OpenAL32/alSource.c
    OpenAL32/alExtension.c
    OpenAL32/alState.c
    OpenAL32/alFilter.c
    OpenAL32/alThunk.c
    OpenAL32/alFontsound.c
    OpenAL32/sample_cvt.c
    OpenAL32/alListener.c
    common/atomic.c
    common/rwlock.c
    common/threads.c
    common/uintmap.c
    Alc/ALc.c
    Alc/ALu.c
    Alc/hrtf.c
    Alc/mixer_neon.c
    Alc/mixer_sse41.c
    Alc/alcConfig.c
    Alc/bs2b.c
    Alc/mixer.c
    Alc/mixer_sse.c
    Alc/panning.c
    Alc/alcRing.c
    Alc/helpers.c
    Alc/mixer_c.c
    Alc/mixer_sse2.c
    Alc/backends/backend.c
    Alc/effects/chorus.c
    Alc/effects/flanger.c
    Alc/midi/fluidsynth.c
    Alc/backends/base.c
    Alc/effects/compressor.c
    Alc/effects/modulator.c
    Alc/midi/sf2load.c
    Alc/backends/loopback.c
    Alc/effects/dedicated.c
    Alc/effects/null.c
    Alc/midi/soft.c
    Alc/backends/null.c
    Alc/effects/distortion.c
    Alc/effects/reverb.c
    Alc/backends/wave.c
    Alc/effects/echo.c
    Alc/midi/base.c
    Alc/effects/autowah.c
    Alc/effects/equalizer.c
    Alc/midi/dummy.c
)

add_library(${PROJECT_NAME} STATIC ${AL_SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}"/AL")
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}"/Alc")
