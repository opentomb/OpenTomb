# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 2.8)

project(OpenTomb)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

message(WARNING "This CMakeLists.txt does not generate usable executables, it is only used to process the documentation and test compilability.")

add_subdirectory(doc)

set(OPENTOMB_SRCS
    src/anim_state_control.cpp
    src/audio.cpp
    src/character_controller.cpp
    src/controls.cpp
    src/engine.cpp
    src/entity.cpp
    src/game.cpp
    src/gameflow.cpp
    src/gui.cpp
    src/inventory.cpp
    src/main_SDL.cpp
    src/mesh.c
    src/physics_bullet.cpp
    src/render/bordered_texture_atlas.cpp
    src/render/bsp_tree.cpp
    src/render/bsp_tree_2d.c
    src/render/camera.cpp
    src/render/frustum.cpp
    src/render/render.cpp
    src/render/shader_description.cpp
    src/render/shader_manager.cpp
    src/resource.cpp
    src/room.cpp
    src/script.cpp
    src/skeletal_model.c
    src/trigger.cpp
    src/vt/l_common.cpp
    src/vt/l_main.cpp
    src/vt/l_tr1.cpp
    src/vt/l_tr2.cpp
    src/vt/l_tr3.cpp
    src/vt/l_tr4.cpp
    src/vt/l_tr5.cpp
    src/vt/scaler.cpp
    src/vt/vt_level.cpp
    src/world.cpp
)

# TODO Where is this needed?
add_definitions(-DOV_EXCLUDE_STATIC_CALLBACKS)

add_executable(OpenTomb ${OPENTOMB_SRCS} $<TARGET_OBJECTS:core>)

if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_COMPILER_IS_GNUCC)
    message(STATUS "GNU-like compilers detected")
    # Trimming the string may result in a compilation error
endif()

#target_compile_options(OpenTomb PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++11> $<$<COMPILE_LANGUAGE:C>:-std=c99>)
set_target_properties(OpenTomb PROPERTIES C_STANDARD 99 CXX_STANDARD 11)

include(FindPkgConfig)
pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
find_package(SDL2 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Lua REQUIRED)

target_include_directories(
    OpenTomb PRIVATE
    ${SDL2_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    # this is a workaround
    # system headers are incompatible because OpenTomb expects it in a different place
    # it need to be fixed in OpenTomb first
    extern/al
)

target_link_libraries(OpenTomb ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${ZLIB_LIBRARY} ${LUA_LIBRARY} ${OPENAL_LIBRARY})

set(BULLET_SRCS
    extern/bullet/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btDbvt.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btDispatcher.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp
    extern/bullet/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp
    extern/bullet/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btCollisionObject.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btCollisionWorld.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btCollisionWorldImporter.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btGhostObject.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btManifoldResult.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/CollisionDispatch/btUnionFind.cpp
    extern/bullet/BulletCollision/CollisionShapes/btBox2dShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btBoxShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btCapsuleShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btCollisionShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btCompoundShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConcaveShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConeShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConvex2dShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConvexHullShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConvexInternalShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConvexPolyhedron.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConvexShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btCylinderShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btEmptyShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btMultiSphereShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btOptimizedBvh.cpp
    extern/bullet/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btShapeHull.cpp
    extern/bullet/BulletCollision/CollisionShapes/btSphereShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp
    extern/bullet/BulletCollision/CollisionShapes/btTetrahedronShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btTriangleBuffer.cpp
    extern/bullet/BulletCollision/CollisionShapes/btTriangleCallback.cpp
    extern/bullet/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp
    extern/bullet/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp
    extern/bullet/BulletCollision/CollisionShapes/btTriangleMesh.cpp
    extern/bullet/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp
    extern/bullet/BulletCollision/CollisionShapes/btUniformScalingShape.cpp
    extern/bullet/BulletCollision/Gimpact/btContactProcessing.cpp
    extern/bullet/BulletCollision/Gimpact/btGImpactBvh.cpp
    extern/bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp
    extern/bullet/BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp
    extern/bullet/BulletCollision/Gimpact/btGImpactShape.cpp
    extern/bullet/BulletCollision/Gimpact/btGenericPoolAllocator.cpp
    extern/bullet/BulletCollision/Gimpact/btTriangleShapeEx.cpp
    extern/bullet/BulletCollision/Gimpact/gim_box_set.cpp
    extern/bullet/BulletCollision/Gimpact/gim_contact.cpp
    extern/bullet/BulletCollision/Gimpact/gim_memory.cpp
    extern/bullet/BulletCollision/Gimpact/gim_tri_collision.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp
    extern/bullet/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp
    extern/bullet/BulletDynamics/Character/btKinematicCharacterController.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btContactConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btFixedConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btGearConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btGeneric6DofSpring2Constraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp
    extern/bullet/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp
    extern/bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp
    extern/bullet/BulletDynamics/Dynamics/btRigidBody.cpp
    extern/bullet/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp
    extern/bullet/BulletDynamics/Featherstone/btMultiBody.cpp
    extern/bullet/BulletDynamics/Featherstone/btMultiBodyConstraint.cpp
    extern/bullet/BulletDynamics/Featherstone/btMultiBodyConstraintSolver.cpp
    extern/bullet/BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.cpp
    extern/bullet/BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.cpp
    extern/bullet/BulletDynamics/Featherstone/btMultiBodyJointMotor.cpp
    extern/bullet/BulletDynamics/Featherstone/btMultiBodyPoint2Point.cpp
    extern/bullet/BulletDynamics/MLCPSolvers/btDantzigLCP.cpp
    extern/bullet/BulletDynamics/MLCPSolvers/btLemkeAlgorithm.cpp
    extern/bullet/BulletDynamics/MLCPSolvers/btMLCPSolver.cpp
    extern/bullet/BulletDynamics/Vehicle/btRaycastVehicle.cpp
    extern/bullet/BulletDynamics/Vehicle/btWheelInfo.cpp
    extern/bullet/BulletSoftBody/btDefaultSoftBodySolver.cpp
    extern/bullet/BulletSoftBody/btSoftBody.cpp
    extern/bullet/BulletSoftBody/btSoftBodyConcaveCollisionAlgorithm.cpp
    extern/bullet/BulletSoftBody/btSoftBodyHelpers.cpp
    extern/bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.cpp
    extern/bullet/BulletSoftBody/btSoftRigidCollisionAlgorithm.cpp
    extern/bullet/BulletSoftBody/btSoftRigidDynamicsWorld.cpp
    extern/bullet/BulletSoftBody/btSoftSoftCollisionAlgorithm.cpp
    extern/bullet/LinearMath/btAlignedAllocator.cpp
    extern/bullet/LinearMath/btConvexHull.cpp
    extern/bullet/LinearMath/btConvexHullComputer.cpp
    extern/bullet/LinearMath/btGeometryUtil.cpp
    extern/bullet/LinearMath/btPolarDecomposition.cpp
    extern/bullet/LinearMath/btQuickprof.cpp
    extern/bullet/LinearMath/btSerializer.cpp
    extern/bullet/LinearMath/btVector3.cpp
)

set(FREETYPE_SRCS
    extern/freetype2/src/autofit/autofit.c
    extern/freetype2/src/base/ftapi.c
    extern/freetype2/src/base/ftbase.c
    extern/freetype2/src/base/ftbbox.c
    extern/freetype2/src/base/ftbdf.c
    extern/freetype2/src/base/ftbitmap.c
    extern/freetype2/src/base/ftcid.c
    extern/freetype2/src/base/ftdebug.c
    extern/freetype2/src/base/ftfstype.c
    extern/freetype2/src/base/ftgasp.c
    extern/freetype2/src/base/ftglyph.c
    extern/freetype2/src/base/ftgxval.c
    extern/freetype2/src/base/ftinit.c
    extern/freetype2/src/base/ftlcdfil.c
    extern/freetype2/src/base/ftmm.c
    extern/freetype2/src/base/ftotval.c
    extern/freetype2/src/base/ftpatent.c
    extern/freetype2/src/base/ftpfr.c
    extern/freetype2/src/base/ftstroke.c
    extern/freetype2/src/base/ftsynth.c
    extern/freetype2/src/base/ftsystem.c
    extern/freetype2/src/base/fttype1.c
    extern/freetype2/src/base/ftwinfnt.c
    extern/freetype2/src/base/ftxf86.c
    extern/freetype2/src/base/md5.c
    extern/freetype2/src/bdf/bdf.c
    extern/freetype2/src/bzip2/ftbzip2.c
    extern/freetype2/src/cache/ftcache.c
    extern/freetype2/src/cff/cff.c
    extern/freetype2/src/cid/type1cid.c
    extern/freetype2/src/gxvalid/gxvalid.c
    extern/freetype2/src/gzip/ftgzip.c
    extern/freetype2/src/lzw/ftlzw.c
    extern/freetype2/src/otvalid/otvalid.c
    extern/freetype2/src/pcf/pcf.c
    extern/freetype2/src/pfr/pfr.c
    extern/freetype2/src/psaux/psaux.c
    extern/freetype2/src/pshinter/pshinter.c
    extern/freetype2/src/psnames/psnames.c
    extern/freetype2/src/raster/raster.c
    extern/freetype2/src/sfnt/sfnt.c
    extern/freetype2/src/smooth/smooth.c
    extern/freetype2/src/truetype/truetype.c
    extern/freetype2/src/type1/type1.c
    extern/freetype2/src/type42/type42.c
    extern/freetype2/src/winfonts/winfnt.c
)

set(OGG_SRCS
    extern/ogg/libogg/bitwise.c
    extern/ogg/libogg/framing.c
    extern/ogg/libvorbis/analysis.c
    extern/ogg/libvorbis/bitrate.c
    extern/ogg/libvorbis/block.c
    extern/ogg/libvorbis/codebook.c
    extern/ogg/libvorbis/envelope.c
    extern/ogg/libvorbis/floor0.c
    extern/ogg/libvorbis/floor1.c
    extern/ogg/libvorbis/info.c
    extern/ogg/libvorbis/lookup.c
    extern/ogg/libvorbis/lpc.c
    extern/ogg/libvorbis/lsp.c
    extern/ogg/libvorbis/mapping0.c
    extern/ogg/libvorbis/mdct.c
    extern/ogg/libvorbis/psy.c
    extern/ogg/libvorbis/registry.c
    extern/ogg/libvorbis/res0.c
    extern/ogg/libvorbis/sharedbook.c
    extern/ogg/libvorbis/smallft.c
    extern/ogg/libvorbis/synthesis.c
    extern/ogg/libvorbis/vorbisfile.c
    extern/ogg/libvorbis/window.c
)

set(OPENAL_SRCS
    extern/al/Alc/ALc.c
    extern/al/Alc/ALu.c
    extern/al/Alc/alcConfig.c
    extern/al/Alc/alcRing.c
    extern/al/Alc/backends/backend.c
    extern/al/Alc/backends/base.c
    extern/al/Alc/backends/loopback.c
    extern/al/Alc/backends/null.c
    extern/al/Alc/backends/wave.c
    extern/al/Alc/bs2b.c
    extern/al/Alc/effects/autowah.c
    extern/al/Alc/effects/chorus.c
    extern/al/Alc/effects/compressor.c
    extern/al/Alc/effects/dedicated.c
    extern/al/Alc/effects/distortion.c
    extern/al/Alc/effects/echo.c
    extern/al/Alc/effects/equalizer.c
    extern/al/Alc/effects/flanger.c
    extern/al/Alc/effects/modulator.c
    extern/al/Alc/effects/null.c
    extern/al/Alc/effects/reverb.c
    extern/al/Alc/helpers.c
    extern/al/Alc/hrtf.c
    extern/al/Alc/midi/base.c
    extern/al/Alc/midi/dummy.c
    extern/al/Alc/midi/fluidsynth.c
    extern/al/Alc/midi/sf2load.c
    extern/al/Alc/midi/soft.c
    extern/al/Alc/mixer.c
    extern/al/Alc/mixer_c.c
    extern/al/Alc/mixer_neon.c
    extern/al/Alc/mixer_sse.c
    extern/al/Alc/mixer_sse2.c
    extern/al/Alc/mixer_sse41.c
    extern/al/Alc/panning.c
    extern/al/OpenAL32/alAuxEffectSlot.c
    extern/al/OpenAL32/alBuffer.c
    extern/al/OpenAL32/alEffect.c
    extern/al/OpenAL32/alError.c
    extern/al/OpenAL32/alExtension.c
    extern/al/OpenAL32/alFilter.c
    extern/al/OpenAL32/alFontsound.c
    extern/al/OpenAL32/alListener.c
    extern/al/OpenAL32/alMidi.c
    extern/al/OpenAL32/alPreset.c
    extern/al/OpenAL32/alSoundfont.c
    extern/al/OpenAL32/alSource.c
    extern/al/OpenAL32/alState.c
    extern/al/OpenAL32/alThunk.c
    extern/al/OpenAL32/sample_cvt.c
    extern/al/common/atomic.c
    extern/al/common/rwlock.c
    extern/al/common/threads.c
    extern/al/common/uintmap.c
)

set(CORE_SRCS
    src/core/console.c
    src/core/gl_font.c
    src/core/gl_text.c
    src/core/gl_util.c
    src/core/obb.c
    src/core/polygon.c
    src/core/redblack.c
    src/core/system.c
    src/core/utf8_32.c
    src/core/vmath.c
)

add_library(core OBJECT ${CORE_SRCS})
target_include_directories(core PUBLIC extern/freetype2/include ${LUA_INCLUDE_DIR})
#target_compile_options(core PUBLIC $<$<COMPILE_LANGUAGE:C>:-std=c99>)
set_target_properties(core PROPERTIES C_STANDARD 99)

add_library(bullet STATIC ${BULLET_SRCS})
target_include_directories(bullet PUBLIC extern/bullet)
target_link_libraries(OpenTomb bullet)

add_library(ogg STATIC ${OGG_SRCS})
target_include_directories(ogg PUBLIC extern/ogg)
target_link_libraries(OpenTomb ogg)

add_library(freetype2 STATIC ${FREETYPE_SRCS})
target_include_directories(freetype2 PUBLIC extern/freetype2/include)
target_link_libraries(OpenTomb freetype2)
