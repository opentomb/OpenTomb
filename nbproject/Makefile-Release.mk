#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include ENGINE-Makefile.mk

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/al/Alc/ALc.o \
	${OBJECTDIR}/src/al/Alc/ALu.o \
	${OBJECTDIR}/src/al/Alc/alcConfig.o \
	${OBJECTDIR}/src/al/Alc/alcRing.o \
	${OBJECTDIR}/src/al/Alc/alcThread.o \
	${OBJECTDIR}/src/al/Alc/backends/backend.o \
	${OBJECTDIR}/src/al/Alc/backends/loopback.o \
	${OBJECTDIR}/src/al/Alc/backends/null.o \
	${OBJECTDIR}/src/al/Alc/backends/wave.o \
	${OBJECTDIR}/src/al/Alc/bs2b.o \
	${OBJECTDIR}/src/al/Alc/effects/chorus.o \
	${OBJECTDIR}/src/al/Alc/effects/dedicated.o \
	${OBJECTDIR}/src/al/Alc/effects/distortion.o \
	${OBJECTDIR}/src/al/Alc/effects/echo.o \
	${OBJECTDIR}/src/al/Alc/effects/effect_null.o \
	${OBJECTDIR}/src/al/Alc/effects/equalizer.o \
	${OBJECTDIR}/src/al/Alc/effects/flanger.o \
	${OBJECTDIR}/src/al/Alc/effects/modulator.o \
	${OBJECTDIR}/src/al/Alc/effects/reverb.o \
	${OBJECTDIR}/src/al/Alc/helpers.o \
	${OBJECTDIR}/src/al/Alc/hrtf.o \
	${OBJECTDIR}/src/al/Alc/mixer.o \
	${OBJECTDIR}/src/al/Alc/mixer_c.o \
	${OBJECTDIR}/src/al/Alc/mixer_inc.o \
	${OBJECTDIR}/src/al/Alc/mixer_neon.o \
	${OBJECTDIR}/src/al/Alc/mixer_sse.o \
	${OBJECTDIR}/src/al/Alc/panning.o \
	${OBJECTDIR}/src/al/OpenAL32/alAuxEffectSlot.o \
	${OBJECTDIR}/src/al/OpenAL32/alBuffer.o \
	${OBJECTDIR}/src/al/OpenAL32/alEffect.o \
	${OBJECTDIR}/src/al/OpenAL32/alError.o \
	${OBJECTDIR}/src/al/OpenAL32/alExtension.o \
	${OBJECTDIR}/src/al/OpenAL32/alFilter.o \
	${OBJECTDIR}/src/al/OpenAL32/alListener.o \
	${OBJECTDIR}/src/al/OpenAL32/alSource.o \
	${OBJECTDIR}/src/al/OpenAL32/alState.o \
	${OBJECTDIR}/src/al/OpenAL32/alThunk.o \
	${OBJECTDIR}/src/anim_state_control.o \
	${OBJECTDIR}/src/audio.o \
	${OBJECTDIR}/src/bordered_texture_atlas.o \
	${OBJECTDIR}/src/bounding_volume.o \
	${OBJECTDIR}/src/bsp_tree_2d.o \
	${OBJECTDIR}/src/cache.o \
	${OBJECTDIR}/src/camera.o \
	${OBJECTDIR}/src/character_controller.o \
	${OBJECTDIR}/src/common.o \
	${OBJECTDIR}/src/console.o \
	${OBJECTDIR}/src/controls.o \
	${OBJECTDIR}/src/engine.o \
	${OBJECTDIR}/src/entity.o \
	${OBJECTDIR}/src/frustum.o \
	${OBJECTDIR}/src/ftgl/FTBitmapGlyph.o \
	${OBJECTDIR}/src/ftgl/FTCharmap.o \
	${OBJECTDIR}/src/ftgl/FTContour.o \
	${OBJECTDIR}/src/ftgl/FTExtrdGlyph.o \
	${OBJECTDIR}/src/ftgl/FTFace.o \
	${OBJECTDIR}/src/ftgl/FTFont.o \
	${OBJECTDIR}/src/ftgl/FTGLBitmapFont.o \
	${OBJECTDIR}/src/ftgl/FTGLExtrdFont.o \
	${OBJECTDIR}/src/ftgl/FTGLOutlineFont.o \
	${OBJECTDIR}/src/ftgl/FTGLPixmapFont.o \
	${OBJECTDIR}/src/ftgl/FTGLPolygonFont.o \
	${OBJECTDIR}/src/ftgl/FTGLTextureFont.o \
	${OBJECTDIR}/src/ftgl/FTGlyph.o \
	${OBJECTDIR}/src/ftgl/FTGlyphContainer.o \
	${OBJECTDIR}/src/ftgl/FTLibrary.o \
	${OBJECTDIR}/src/ftgl/FTOutlineGlyph.o \
	${OBJECTDIR}/src/ftgl/FTPixmapGlyph.o \
	${OBJECTDIR}/src/ftgl/FTPoint.o \
	${OBJECTDIR}/src/ftgl/FTPolyGlyph.o \
	${OBJECTDIR}/src/ftgl/FTSize.o \
	${OBJECTDIR}/src/ftgl/FTTextureGlyph.o \
	${OBJECTDIR}/src/ftgl/FTVectoriser.o \
	${OBJECTDIR}/src/game.o \
	${OBJECTDIR}/src/gameflow.o \
	${OBJECTDIR}/src/gl_util.o \
	${OBJECTDIR}/src/gui.o \
	${OBJECTDIR}/src/lua/lapi.o \
	${OBJECTDIR}/src/lua/lauxlib.o \
	${OBJECTDIR}/src/lua/lbaselib.o \
	${OBJECTDIR}/src/lua/lbitlib.o \
	${OBJECTDIR}/src/lua/lcode.o \
	${OBJECTDIR}/src/lua/lcorolib.o \
	${OBJECTDIR}/src/lua/lctype.o \
	${OBJECTDIR}/src/lua/ldblib.o \
	${OBJECTDIR}/src/lua/ldebug.o \
	${OBJECTDIR}/src/lua/ldo.o \
	${OBJECTDIR}/src/lua/ldump.o \
	${OBJECTDIR}/src/lua/lfunc.o \
	${OBJECTDIR}/src/lua/lgc.o \
	${OBJECTDIR}/src/lua/linit.o \
	${OBJECTDIR}/src/lua/liolib.o \
	${OBJECTDIR}/src/lua/llex.o \
	${OBJECTDIR}/src/lua/lmathlib.o \
	${OBJECTDIR}/src/lua/lmem.o \
	${OBJECTDIR}/src/lua/loadlib.o \
	${OBJECTDIR}/src/lua/lobject.o \
	${OBJECTDIR}/src/lua/lopcodes.o \
	${OBJECTDIR}/src/lua/loslib.o \
	${OBJECTDIR}/src/lua/lparser.o \
	${OBJECTDIR}/src/lua/lstate.o \
	${OBJECTDIR}/src/lua/lstring.o \
	${OBJECTDIR}/src/lua/lstrlib.o \
	${OBJECTDIR}/src/lua/ltable.o \
	${OBJECTDIR}/src/lua/ltablib.o \
	${OBJECTDIR}/src/lua/ltm.o \
	${OBJECTDIR}/src/lua/lundump.o \
	${OBJECTDIR}/src/lua/lvm.o \
	${OBJECTDIR}/src/lua/lzio.o \
	${OBJECTDIR}/src/main_SDL.o \
	${OBJECTDIR}/src/mesh.o \
	${OBJECTDIR}/src/ogg/libogg/bitwise.o \
	${OBJECTDIR}/src/ogg/libogg/framing.o \
	${OBJECTDIR}/src/ogg/libvorbis/analysis.o \
	${OBJECTDIR}/src/ogg/libvorbis/bitrate.o \
	${OBJECTDIR}/src/ogg/libvorbis/block.o \
	${OBJECTDIR}/src/ogg/libvorbis/codebook.o \
	${OBJECTDIR}/src/ogg/libvorbis/envelope.o \
	${OBJECTDIR}/src/ogg/libvorbis/floor0.o \
	${OBJECTDIR}/src/ogg/libvorbis/floor1.o \
	${OBJECTDIR}/src/ogg/libvorbis/info.o \
	${OBJECTDIR}/src/ogg/libvorbis/lookup.o \
	${OBJECTDIR}/src/ogg/libvorbis/lpc.o \
	${OBJECTDIR}/src/ogg/libvorbis/lsp.o \
	${OBJECTDIR}/src/ogg/libvorbis/mapping0.o \
	${OBJECTDIR}/src/ogg/libvorbis/mdct.o \
	${OBJECTDIR}/src/ogg/libvorbis/psy.o \
	${OBJECTDIR}/src/ogg/libvorbis/registry.o \
	${OBJECTDIR}/src/ogg/libvorbis/res0.o \
	${OBJECTDIR}/src/ogg/libvorbis/sharedbook.o \
	${OBJECTDIR}/src/ogg/libvorbis/smallft.o \
	${OBJECTDIR}/src/ogg/libvorbis/synthesis.o \
	${OBJECTDIR}/src/ogg/libvorbis/vorbisfile.o \
	${OBJECTDIR}/src/ogg/libvorbis/window.o \
	${OBJECTDIR}/src/polygon.o \
	${OBJECTDIR}/src/portal.o \
	${OBJECTDIR}/src/redblack.o \
	${OBJECTDIR}/src/render.o \
	${OBJECTDIR}/src/resource.o \
	${OBJECTDIR}/src/script.o \
	${OBJECTDIR}/src/system.o \
	${OBJECTDIR}/src/vmath.o \
	${OBJECTDIR}/src/vt/l_common.o \
	${OBJECTDIR}/src/vt/l_main.o \
	${OBJECTDIR}/src/vt/l_tr1.o \
	${OBJECTDIR}/src/vt/l_tr2.o \
	${OBJECTDIR}/src/vt/l_tr3.o \
	${OBJECTDIR}/src/vt/l_tr4.o \
	${OBJECTDIR}/src/vt/l_tr5.o \
	${OBJECTDIR}/src/vt/scaler.o \
	${OBJECTDIR}/src/vt/vt_level.o \
	${OBJECTDIR}/src/world.o \
	${OBJECTDIR}/src/zone.o


# C Compiler Flags
CFLAGS=-I"src\bullet" -I"src\freetype2" -O2 -march=prescott

# CC Compiler Flags
CCFLAGS=-I"src\bullet" -I"src\freetype2" -O2 -march=prescott
CXXFLAGS=-I"src\bullet" -I"src\freetype2" -O2 -march=prescott

# Fortran Compiler Flags
FFLAGS=-s

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/opentomb-code.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/opentomb-code.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/opentomb-code ${OBJECTFILES} ${LDLIBSOPTIONS} -static -lmingw32 -limagee -lSDL2main -lSDL2.dll -L"lib\." -lbullete -lfreetype2e -lglu32 -lopengl32 -limm32 -lkernel32 -lole32 -loleaut32 -luuid -lversion -lwinmm -lgdi32 -lz -lpthread "res/resource.o" -s

${OBJECTDIR}/src/al/Alc/ALc.o: src/al/Alc/ALc.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/ALc.o src/al/Alc/ALc.c

${OBJECTDIR}/src/al/Alc/ALu.o: src/al/Alc/ALu.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/ALu.o src/al/Alc/ALu.c

${OBJECTDIR}/src/al/Alc/alcConfig.o: src/al/Alc/alcConfig.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/alcConfig.o src/al/Alc/alcConfig.c

${OBJECTDIR}/src/al/Alc/alcRing.o: src/al/Alc/alcRing.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/alcRing.o src/al/Alc/alcRing.c

${OBJECTDIR}/src/al/Alc/alcThread.o: src/al/Alc/alcThread.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/alcThread.o src/al/Alc/alcThread.c

${OBJECTDIR}/src/al/Alc/backends/backend.o: src/al/Alc/backends/backend.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/backends
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/backends/backend.o src/al/Alc/backends/backend.c

${OBJECTDIR}/src/al/Alc/backends/loopback.o: src/al/Alc/backends/loopback.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/backends
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/backends/loopback.o src/al/Alc/backends/loopback.c

${OBJECTDIR}/src/al/Alc/backends/null.o: src/al/Alc/backends/null.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/backends
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/backends/null.o src/al/Alc/backends/null.c

${OBJECTDIR}/src/al/Alc/backends/wave.o: src/al/Alc/backends/wave.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/backends
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/backends/wave.o src/al/Alc/backends/wave.c

${OBJECTDIR}/src/al/Alc/bs2b.o: src/al/Alc/bs2b.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/bs2b.o src/al/Alc/bs2b.c

${OBJECTDIR}/src/al/Alc/effects/chorus.o: src/al/Alc/effects/chorus.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/chorus.o src/al/Alc/effects/chorus.c

${OBJECTDIR}/src/al/Alc/effects/dedicated.o: src/al/Alc/effects/dedicated.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/dedicated.o src/al/Alc/effects/dedicated.c

${OBJECTDIR}/src/al/Alc/effects/distortion.o: src/al/Alc/effects/distortion.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/distortion.o src/al/Alc/effects/distortion.c

${OBJECTDIR}/src/al/Alc/effects/echo.o: src/al/Alc/effects/echo.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/echo.o src/al/Alc/effects/echo.c

${OBJECTDIR}/src/al/Alc/effects/effect_null.o: src/al/Alc/effects/effect_null.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/effect_null.o src/al/Alc/effects/effect_null.c

${OBJECTDIR}/src/al/Alc/effects/equalizer.o: src/al/Alc/effects/equalizer.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/equalizer.o src/al/Alc/effects/equalizer.c

${OBJECTDIR}/src/al/Alc/effects/flanger.o: src/al/Alc/effects/flanger.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/flanger.o src/al/Alc/effects/flanger.c

${OBJECTDIR}/src/al/Alc/effects/modulator.o: src/al/Alc/effects/modulator.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/modulator.o src/al/Alc/effects/modulator.c

${OBJECTDIR}/src/al/Alc/effects/reverb.o: src/al/Alc/effects/reverb.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc/effects
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/effects/reverb.o src/al/Alc/effects/reverb.c

${OBJECTDIR}/src/al/Alc/helpers.o: src/al/Alc/helpers.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/helpers.o src/al/Alc/helpers.c

${OBJECTDIR}/src/al/Alc/hrtf.o: src/al/Alc/hrtf.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/hrtf.o src/al/Alc/hrtf.c

${OBJECTDIR}/src/al/Alc/mixer.o: src/al/Alc/mixer.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/mixer.o src/al/Alc/mixer.c

${OBJECTDIR}/src/al/Alc/mixer_c.o: src/al/Alc/mixer_c.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/mixer_c.o src/al/Alc/mixer_c.c

${OBJECTDIR}/src/al/Alc/mixer_inc.o: src/al/Alc/mixer_inc.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/mixer_inc.o src/al/Alc/mixer_inc.c

${OBJECTDIR}/src/al/Alc/mixer_neon.o: src/al/Alc/mixer_neon.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/mixer_neon.o src/al/Alc/mixer_neon.c

${OBJECTDIR}/src/al/Alc/mixer_sse.o: src/al/Alc/mixer_sse.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/mixer_sse.o src/al/Alc/mixer_sse.c

${OBJECTDIR}/src/al/Alc/panning.o: src/al/Alc/panning.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/Alc
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/Alc/panning.o src/al/Alc/panning.c

${OBJECTDIR}/src/al/OpenAL32/alAuxEffectSlot.o: src/al/OpenAL32/alAuxEffectSlot.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alAuxEffectSlot.o src/al/OpenAL32/alAuxEffectSlot.c

${OBJECTDIR}/src/al/OpenAL32/alBuffer.o: src/al/OpenAL32/alBuffer.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alBuffer.o src/al/OpenAL32/alBuffer.c

${OBJECTDIR}/src/al/OpenAL32/alEffect.o: src/al/OpenAL32/alEffect.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alEffect.o src/al/OpenAL32/alEffect.c

${OBJECTDIR}/src/al/OpenAL32/alError.o: src/al/OpenAL32/alError.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alError.o src/al/OpenAL32/alError.c

${OBJECTDIR}/src/al/OpenAL32/alExtension.o: src/al/OpenAL32/alExtension.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alExtension.o src/al/OpenAL32/alExtension.c

${OBJECTDIR}/src/al/OpenAL32/alFilter.o: src/al/OpenAL32/alFilter.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alFilter.o src/al/OpenAL32/alFilter.c

${OBJECTDIR}/src/al/OpenAL32/alListener.o: src/al/OpenAL32/alListener.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alListener.o src/al/OpenAL32/alListener.c

${OBJECTDIR}/src/al/OpenAL32/alSource.o: src/al/OpenAL32/alSource.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alSource.o src/al/OpenAL32/alSource.c

${OBJECTDIR}/src/al/OpenAL32/alState.o: src/al/OpenAL32/alState.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alState.o src/al/OpenAL32/alState.c

${OBJECTDIR}/src/al/OpenAL32/alThunk.o: src/al/OpenAL32/alThunk.c 
	${MKDIR} -p ${OBJECTDIR}/src/al/OpenAL32
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/al/OpenAL32/alThunk.o src/al/OpenAL32/alThunk.c

${OBJECTDIR}/src/anim_state_control.o: src/anim_state_control.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/anim_state_control.o src/anim_state_control.cpp

${OBJECTDIR}/src/audio.o: src/audio.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/audio.o src/audio.cpp

${OBJECTDIR}/src/bordered_texture_atlas.o: src/bordered_texture_atlas.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/bordered_texture_atlas.o src/bordered_texture_atlas.cpp

${OBJECTDIR}/src/bounding_volume.o: src/bounding_volume.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/bounding_volume.o src/bounding_volume.cpp

${OBJECTDIR}/src/bsp_tree_2d.o: src/bsp_tree_2d.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/bsp_tree_2d.o src/bsp_tree_2d.c

${OBJECTDIR}/src/cache.o: src/cache.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/cache.o src/cache.cpp

${OBJECTDIR}/src/camera.o: src/camera.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/camera.o src/camera.cpp

${OBJECTDIR}/src/character_controller.o: src/character_controller.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/character_controller.o src/character_controller.cpp

${OBJECTDIR}/src/common.o: src/common.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/common.o src/common.cpp

${OBJECTDIR}/src/console.o: src/console.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/console.o src/console.cpp

${OBJECTDIR}/src/controls.o: src/controls.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/controls.o src/controls.cpp

${OBJECTDIR}/src/engine.o: src/engine.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/engine.o src/engine.cpp

${OBJECTDIR}/src/entity.o: src/entity.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/entity.o src/entity.cpp

${OBJECTDIR}/src/frustum.o: src/frustum.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/frustum.o src/frustum.cpp

${OBJECTDIR}/src/ftgl/FTBitmapGlyph.o: src/ftgl/FTBitmapGlyph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTBitmapGlyph.o src/ftgl/FTBitmapGlyph.cpp

${OBJECTDIR}/src/ftgl/FTCharmap.o: src/ftgl/FTCharmap.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTCharmap.o src/ftgl/FTCharmap.cpp

${OBJECTDIR}/src/ftgl/FTContour.o: src/ftgl/FTContour.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTContour.o src/ftgl/FTContour.cpp

${OBJECTDIR}/src/ftgl/FTExtrdGlyph.o: src/ftgl/FTExtrdGlyph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTExtrdGlyph.o src/ftgl/FTExtrdGlyph.cpp

${OBJECTDIR}/src/ftgl/FTFace.o: src/ftgl/FTFace.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTFace.o src/ftgl/FTFace.cpp

${OBJECTDIR}/src/ftgl/FTFont.o: src/ftgl/FTFont.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTFont.o src/ftgl/FTFont.cpp

${OBJECTDIR}/src/ftgl/FTGLBitmapFont.o: src/ftgl/FTGLBitmapFont.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTGLBitmapFont.o src/ftgl/FTGLBitmapFont.cpp

${OBJECTDIR}/src/ftgl/FTGLExtrdFont.o: src/ftgl/FTGLExtrdFont.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTGLExtrdFont.o src/ftgl/FTGLExtrdFont.cpp

${OBJECTDIR}/src/ftgl/FTGLOutlineFont.o: src/ftgl/FTGLOutlineFont.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTGLOutlineFont.o src/ftgl/FTGLOutlineFont.cpp

${OBJECTDIR}/src/ftgl/FTGLPixmapFont.o: src/ftgl/FTGLPixmapFont.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTGLPixmapFont.o src/ftgl/FTGLPixmapFont.cpp

${OBJECTDIR}/src/ftgl/FTGLPolygonFont.o: src/ftgl/FTGLPolygonFont.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTGLPolygonFont.o src/ftgl/FTGLPolygonFont.cpp

${OBJECTDIR}/src/ftgl/FTGLTextureFont.o: src/ftgl/FTGLTextureFont.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTGLTextureFont.o src/ftgl/FTGLTextureFont.cpp

${OBJECTDIR}/src/ftgl/FTGlyph.o: src/ftgl/FTGlyph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTGlyph.o src/ftgl/FTGlyph.cpp

${OBJECTDIR}/src/ftgl/FTGlyphContainer.o: src/ftgl/FTGlyphContainer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTGlyphContainer.o src/ftgl/FTGlyphContainer.cpp

${OBJECTDIR}/src/ftgl/FTLibrary.o: src/ftgl/FTLibrary.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTLibrary.o src/ftgl/FTLibrary.cpp

${OBJECTDIR}/src/ftgl/FTOutlineGlyph.o: src/ftgl/FTOutlineGlyph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTOutlineGlyph.o src/ftgl/FTOutlineGlyph.cpp

${OBJECTDIR}/src/ftgl/FTPixmapGlyph.o: src/ftgl/FTPixmapGlyph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTPixmapGlyph.o src/ftgl/FTPixmapGlyph.cpp

${OBJECTDIR}/src/ftgl/FTPoint.o: src/ftgl/FTPoint.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTPoint.o src/ftgl/FTPoint.cpp

${OBJECTDIR}/src/ftgl/FTPolyGlyph.o: src/ftgl/FTPolyGlyph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTPolyGlyph.o src/ftgl/FTPolyGlyph.cpp

${OBJECTDIR}/src/ftgl/FTSize.o: src/ftgl/FTSize.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTSize.o src/ftgl/FTSize.cpp

${OBJECTDIR}/src/ftgl/FTTextureGlyph.o: src/ftgl/FTTextureGlyph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTTextureGlyph.o src/ftgl/FTTextureGlyph.cpp

${OBJECTDIR}/src/ftgl/FTVectoriser.o: src/ftgl/FTVectoriser.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/ftgl
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ftgl/FTVectoriser.o src/ftgl/FTVectoriser.cpp

${OBJECTDIR}/src/game.o: src/game.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/game.o src/game.cpp

${OBJECTDIR}/src/gameflow.o: src/gameflow.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/gameflow.o src/gameflow.cpp

${OBJECTDIR}/src/gl_util.o: src/gl_util.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/gl_util.o src/gl_util.cpp

${OBJECTDIR}/src/gui.o: src/gui.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/gui.o src/gui.cpp

${OBJECTDIR}/src/lua/lapi.o: src/lua/lapi.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lapi.o src/lua/lapi.c

${OBJECTDIR}/src/lua/lauxlib.o: src/lua/lauxlib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lauxlib.o src/lua/lauxlib.c

${OBJECTDIR}/src/lua/lbaselib.o: src/lua/lbaselib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lbaselib.o src/lua/lbaselib.c

${OBJECTDIR}/src/lua/lbitlib.o: src/lua/lbitlib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lbitlib.o src/lua/lbitlib.c

${OBJECTDIR}/src/lua/lcode.o: src/lua/lcode.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lcode.o src/lua/lcode.c

${OBJECTDIR}/src/lua/lcorolib.o: src/lua/lcorolib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lcorolib.o src/lua/lcorolib.c

${OBJECTDIR}/src/lua/lctype.o: src/lua/lctype.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lctype.o src/lua/lctype.c

${OBJECTDIR}/src/lua/ldblib.o: src/lua/ldblib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/ldblib.o src/lua/ldblib.c

${OBJECTDIR}/src/lua/ldebug.o: src/lua/ldebug.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/ldebug.o src/lua/ldebug.c

${OBJECTDIR}/src/lua/ldo.o: src/lua/ldo.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/ldo.o src/lua/ldo.c

${OBJECTDIR}/src/lua/ldump.o: src/lua/ldump.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/ldump.o src/lua/ldump.c

${OBJECTDIR}/src/lua/lfunc.o: src/lua/lfunc.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lfunc.o src/lua/lfunc.c

${OBJECTDIR}/src/lua/lgc.o: src/lua/lgc.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lgc.o src/lua/lgc.c

${OBJECTDIR}/src/lua/linit.o: src/lua/linit.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/linit.o src/lua/linit.c

${OBJECTDIR}/src/lua/liolib.o: src/lua/liolib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/liolib.o src/lua/liolib.c

${OBJECTDIR}/src/lua/llex.o: src/lua/llex.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/llex.o src/lua/llex.c

${OBJECTDIR}/src/lua/lmathlib.o: src/lua/lmathlib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lmathlib.o src/lua/lmathlib.c

${OBJECTDIR}/src/lua/lmem.o: src/lua/lmem.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lmem.o src/lua/lmem.c

${OBJECTDIR}/src/lua/loadlib.o: src/lua/loadlib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/loadlib.o src/lua/loadlib.c

${OBJECTDIR}/src/lua/lobject.o: src/lua/lobject.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lobject.o src/lua/lobject.c

${OBJECTDIR}/src/lua/lopcodes.o: src/lua/lopcodes.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lopcodes.o src/lua/lopcodes.c

${OBJECTDIR}/src/lua/loslib.o: src/lua/loslib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/loslib.o src/lua/loslib.c

${OBJECTDIR}/src/lua/lparser.o: src/lua/lparser.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lparser.o src/lua/lparser.c

${OBJECTDIR}/src/lua/lstate.o: src/lua/lstate.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lstate.o src/lua/lstate.c

${OBJECTDIR}/src/lua/lstring.o: src/lua/lstring.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lstring.o src/lua/lstring.c

${OBJECTDIR}/src/lua/lstrlib.o: src/lua/lstrlib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lstrlib.o src/lua/lstrlib.c

${OBJECTDIR}/src/lua/ltable.o: src/lua/ltable.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/ltable.o src/lua/ltable.c

${OBJECTDIR}/src/lua/ltablib.o: src/lua/ltablib.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/ltablib.o src/lua/ltablib.c

${OBJECTDIR}/src/lua/ltm.o: src/lua/ltm.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/ltm.o src/lua/ltm.c

${OBJECTDIR}/src/lua/lundump.o: src/lua/lundump.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lundump.o src/lua/lundump.c

${OBJECTDIR}/src/lua/lvm.o: src/lua/lvm.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lvm.o src/lua/lvm.c

${OBJECTDIR}/src/lua/lzio.o: src/lua/lzio.c 
	${MKDIR} -p ${OBJECTDIR}/src/lua
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lua/lzio.o src/lua/lzio.c

${OBJECTDIR}/src/main_SDL.o: src/main_SDL.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main_SDL.o src/main_SDL.cpp

${OBJECTDIR}/src/mesh.o: src/mesh.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/mesh.o src/mesh.cpp

${OBJECTDIR}/src/ogg/libogg/bitwise.o: src/ogg/libogg/bitwise.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libogg
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libogg/bitwise.o src/ogg/libogg/bitwise.c

${OBJECTDIR}/src/ogg/libogg/framing.o: src/ogg/libogg/framing.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libogg
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libogg/framing.o src/ogg/libogg/framing.c

${OBJECTDIR}/src/ogg/libvorbis/analysis.o: src/ogg/libvorbis/analysis.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/analysis.o src/ogg/libvorbis/analysis.c

${OBJECTDIR}/src/ogg/libvorbis/bitrate.o: src/ogg/libvorbis/bitrate.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/bitrate.o src/ogg/libvorbis/bitrate.c

${OBJECTDIR}/src/ogg/libvorbis/block.o: src/ogg/libvorbis/block.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/block.o src/ogg/libvorbis/block.c

${OBJECTDIR}/src/ogg/libvorbis/codebook.o: src/ogg/libvorbis/codebook.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/codebook.o src/ogg/libvorbis/codebook.c

${OBJECTDIR}/src/ogg/libvorbis/envelope.o: src/ogg/libvorbis/envelope.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/envelope.o src/ogg/libvorbis/envelope.c

${OBJECTDIR}/src/ogg/libvorbis/floor0.o: src/ogg/libvorbis/floor0.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/floor0.o src/ogg/libvorbis/floor0.c

${OBJECTDIR}/src/ogg/libvorbis/floor1.o: src/ogg/libvorbis/floor1.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/floor1.o src/ogg/libvorbis/floor1.c

${OBJECTDIR}/src/ogg/libvorbis/info.o: src/ogg/libvorbis/info.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/info.o src/ogg/libvorbis/info.c

${OBJECTDIR}/src/ogg/libvorbis/lookup.o: src/ogg/libvorbis/lookup.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/lookup.o src/ogg/libvorbis/lookup.c

${OBJECTDIR}/src/ogg/libvorbis/lpc.o: src/ogg/libvorbis/lpc.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/lpc.o src/ogg/libvorbis/lpc.c

${OBJECTDIR}/src/ogg/libvorbis/lsp.o: src/ogg/libvorbis/lsp.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/lsp.o src/ogg/libvorbis/lsp.c

${OBJECTDIR}/src/ogg/libvorbis/mapping0.o: src/ogg/libvorbis/mapping0.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/mapping0.o src/ogg/libvorbis/mapping0.c

${OBJECTDIR}/src/ogg/libvorbis/mdct.o: src/ogg/libvorbis/mdct.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/mdct.o src/ogg/libvorbis/mdct.c

${OBJECTDIR}/src/ogg/libvorbis/psy.o: src/ogg/libvorbis/psy.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/psy.o src/ogg/libvorbis/psy.c

${OBJECTDIR}/src/ogg/libvorbis/registry.o: src/ogg/libvorbis/registry.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/registry.o src/ogg/libvorbis/registry.c

${OBJECTDIR}/src/ogg/libvorbis/res0.o: src/ogg/libvorbis/res0.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/res0.o src/ogg/libvorbis/res0.c

${OBJECTDIR}/src/ogg/libvorbis/sharedbook.o: src/ogg/libvorbis/sharedbook.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/sharedbook.o src/ogg/libvorbis/sharedbook.c

${OBJECTDIR}/src/ogg/libvorbis/smallft.o: src/ogg/libvorbis/smallft.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/smallft.o src/ogg/libvorbis/smallft.c

${OBJECTDIR}/src/ogg/libvorbis/synthesis.o: src/ogg/libvorbis/synthesis.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/synthesis.o src/ogg/libvorbis/synthesis.c

${OBJECTDIR}/src/ogg/libvorbis/vorbisfile.o: src/ogg/libvorbis/vorbisfile.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/vorbisfile.o src/ogg/libvorbis/vorbisfile.c

${OBJECTDIR}/src/ogg/libvorbis/window.o: src/ogg/libvorbis/window.c 
	${MKDIR} -p ${OBJECTDIR}/src/ogg/libvorbis
	${RM} "$@.d"
	$(COMPILE.c) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ogg/libvorbis/window.o src/ogg/libvorbis/window.c

${OBJECTDIR}/src/polygon.o: src/polygon.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/polygon.o src/polygon.cpp

${OBJECTDIR}/src/portal.o: src/portal.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/portal.o src/portal.cpp

${OBJECTDIR}/src/redblack.o: src/redblack.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/redblack.o src/redblack.cpp

${OBJECTDIR}/src/render.o: src/render.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/render.o src/render.cpp

${OBJECTDIR}/src/resource.o: src/resource.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/resource.o src/resource.cpp

${OBJECTDIR}/src/script.o: src/script.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/script.o src/script.cpp

${OBJECTDIR}/src/system.o: src/system.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/system.o src/system.cpp

${OBJECTDIR}/src/vmath.o: src/vmath.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vmath.o src/vmath.cpp

${OBJECTDIR}/src/vt/l_common.o: src/vt/l_common.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/l_common.o src/vt/l_common.cpp

${OBJECTDIR}/src/vt/l_main.o: src/vt/l_main.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/l_main.o src/vt/l_main.cpp

${OBJECTDIR}/src/vt/l_tr1.o: src/vt/l_tr1.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/l_tr1.o src/vt/l_tr1.cpp

${OBJECTDIR}/src/vt/l_tr2.o: src/vt/l_tr2.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/l_tr2.o src/vt/l_tr2.cpp

${OBJECTDIR}/src/vt/l_tr3.o: src/vt/l_tr3.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/l_tr3.o src/vt/l_tr3.cpp

${OBJECTDIR}/src/vt/l_tr4.o: src/vt/l_tr4.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/l_tr4.o src/vt/l_tr4.cpp

${OBJECTDIR}/src/vt/l_tr5.o: src/vt/l_tr5.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/l_tr5.o src/vt/l_tr5.cpp

${OBJECTDIR}/src/vt/scaler.o: src/vt/scaler.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/scaler.o src/vt/scaler.cpp

${OBJECTDIR}/src/vt/vt_level.o: src/vt/vt_level.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/vt
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vt/vt_level.o src/vt/vt_level.cpp

${OBJECTDIR}/src/world.o: src/world.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/world.o src/world.cpp

${OBJECTDIR}/src/zone.o: src/zone.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/zone.o src/zone.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/opentomb-code.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
