echo "If You can't build it, fix libraries names or patches in tring 30."
echo "Starting engine building..." 
rm -rf *.exe
rm -rf lib/*.a
rm -rf *.o
echo "(1/8)    Clearning complete"
"g++" -c "vt/*.cpp" -s -O2 -march=prescott
ar rcs lib/libvte.a *.o
rm -rf *.o
echo "(2/8)    'vt' lib building complete"
"gcc" -c "lua/*.c" -s -O2 -march=prescott
ar rcs lib/libluae.a *.o
rm -rf *.o
echo "(3/8)    'lua' lib building complete"
"g++" -c "ftgl/*.cpp" -I"freetype2" -s -O2 -march=prescott
ar rcs lib/libftgle.a *.o
rm -rf *.o
echo "(4/8)    'ftgl' lib building complete"
"gcc" -c "freetype2/src/base/*.c" "freetype2/src/autofit/*.c" "freetype2/src/bdf/*.c" "freetype2/src/cache/*.c" "freetype2/src/cff/*.c" "freetype2/src/cid/*.c" "freetype2/src/gzip/*.c" "freetype2/src/lzw/*.c" "freetype2/src/otvalid/*.c" "freetype2/src/pcf/*.c" "freetype2/src/pfr/*.c" "freetype2/src/psaux/*.c" "freetype2/src/pshinter/*.c" "freetype2/src/psnames/*.c" "freetype2/src/raster/*.c" "freetype2/src/sfnt/*.c" "freetype2/src/smooth/*.c" "freetype2/src/truetype/*.c" "freetype2/src/type1/*.c" "freetype2/src/type42/*.c" "freetype2/src/winfonts/*.c" -I"freetype2" -s -O2 -march=prescott
ar rcs lib/libfreetype2e.a *.o
rm -rf *.o
echo "(5/8)    'freetype2' lib building complete"
"gcc" -c "al/OpenAL32/*.c" "al/Alc/*.c" "al/Alc/effects/*.c" "al/Alc/backends/*.c" -I"al" -s -O2 -march=prescott
ar rcs lib/libopenale.a *.o
rm -rf *.o
echo "(6/8)    'OpenAL' lib building complete"
"g++" -c "bullet/LinearMath/*.cpp" "bullet/BulletSoftBody/*.cpp" "bullet/BulletDynamics/ConstraintSolver/*.cpp" "bullet/BulletDynamics/Dynamics/*.cpp" "bullet/BulletDynamics/Vehicle/*.cpp" "bullet/BulletCollision/BroadphaseCollision/*.cpp" "bullet/BulletCollision/CollisionDispatch/*.cpp" "bullet/BulletCollision/CollisionShapes/*.cpp" "bullet/BulletCollision/Gimpact/*.cpp" "bullet/BulletCollision/NarrowPhaseCollision/*.cpp" -I"bullet" -s -O2 -march=prescott
ar rcs lib/libbullete.a *.o
rm -rf *.o
echo "(7/8)    'bullet' lib building complete"
"gcc" -c "*.c"
"g++" -c "*.cpp" -I"freetype2" -I"bullet" -s -O2 -march=prescott
echo "(8/8)    'engine' object files building complete, next linking..."
"g++" -o "engine.exe" -static -lmingw32 -lSDL2main -lSDL2 -Llib/. -lvte -lluae "*.o" -lopenale -lSDL2main -lSDL2 -lvte -lftgle -lfreetype2e -lluae -lbullete -lglu32 -lopengl32 -limm32 -lole32 -loleaut32 -luuid -lversion -lwinmm -lgdi32 -lz -lpthread
rm -rf *.o
echo "'engine' building complete. end."