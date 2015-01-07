echo "If You can't build it, fix libraries names or patches in tring 30."
echo "Starting engine building..." 
rm -rf *.exe
rm -rf ../lib/*.a
rm -rf *.o
echo "(1/10)    Clearning complete"
"g++" -c "vt/*.cpp" -s -O2 -march=prescott
ar rcs ../lib/libvte.a *.o
rm -rf *.o
echo "(2/10)    'vt' lib building complete"
"gcc" -c "lua/*.c" -s -O2 -march=prescott
ar rcs ../lib/libluae.a *.o
rm -rf *.o
echo "(3/10)    'lua' lib building complete"
"g++" -c "ftgl/*.cpp" -I"freetype2" -s -O2 -march=prescott
ar rcs ../lib/libftgle.a *.o
rm -rf *.o
echo "(4/10)    'ftgl' lib building complete"
"gcc" -c "freetype2/src/base/*.c" "freetype2/src/autofit/*.c" "freetype2/src/bdf/*.c" "freetype2/src/cache/*.c" "freetype2/src/cff/*.c" "freetype2/src/cid/*.c" "freetype2/src/gzip/*.c" "freetype2/src/lzw/*.c" "freetype2/src/otvalid/*.c" "freetype2/src/pcf/*.c" "freetype2/src/pfr/*.c" "freetype2/src/psaux/*.c" "freetype2/src/pshinter/*.c" "freetype2/src/psnames/*.c" "freetype2/src/raster/*.c" "freetype2/src/sfnt/*.c" "freetype2/src/smooth/*.c" "freetype2/src/truetype/*.c" "freetype2/src/type1/*.c" "freetype2/src/type42/*.c" "freetype2/src/winfonts/*.c" -I"freetype2" -s -O2 -march=prescott
ar rcs ../lib/libfreetype2e.a *.o
rm -rf *.o
echo "(5/10)    'freetype2' lib building complete"
"gcc" -c "al/OpenAL32/*.c" "al/Alc/*.c" "al/Alc/effects/*.c" "al/Alc/backends/*.c" -I"al" -s -O2 -march=prescott
ar rcs ../lib/libopenale.a *.o
rm -rf *.o
echo "(6/10)    'OpenAL' lib building complete"
"g++" -c "ogg/libogg/*.c" "ogg/libvorbis/*.c" -DOV_EXCLUDE_STATIC_CALLBACKS -s -O2 -march=prescott
ar rcs ../lib/libogge.a *.o
rm -rf *.o
echo "(7/10)    'ogg' lib building complete"
"g++" -c "bullet/LinearMath/*.cpp" "bullet/BulletSoftBody/*.cpp" "bullet/BulletDynamics/ConstraintSolver/*.cpp" "bullet/BulletDynamics/Dynamics/*.cpp" "bullet/BulletDynamics/Vehicle/*.cpp" "bullet/BulletCollision/BroadphaseCollision/*.cpp" "bullet/BulletCollision/CollisionDispatch/*.cpp" "bullet/BulletCollision/CollisionShapes/*.cpp" "bullet/BulletCollision/Gimpact/*.cpp" "bullet/BulletCollision/NarrowPhaseCollision/*.cpp" -I"bullet" -s -O2 -march=prescott
ar rcs ../lib/libbullete.a *.o
rm -rf *.o
echo "(8/10)    'bullet' lib building complete"
"gcc" -c "*.c"
"g++" -c "*.cpp" -I"freetype2" -I"bullet" -s -O2 -march=prescott
echo "(9/10)    'image' lib building complete"
"gcc.exe" -c "image/*.c" "image/jpeg-9/*.c" "image/libpng-1.6.2/*.c" -DLOAD_JPG -DLOAD_PNG -DLOAD_XPM -DLOAD_TGA -DLOAD_PCX -I"image/jpeg-9" -I"image/libpng-1.6.2" -s -O2 -march=prescott
ar rcs ../lib/libimagee.a *.o
rm -rf *.o
echo "(10/10)    'engine' object files building complete, next linking..."
"g++" -o "engine.exe" -static -lmingw32 -limagee -lSDL2main -lSDL2.dll -L"../lib/." -lvte -lluae "*.o" -lopenale -logge -lSDL2main -lSDL2.dll -lvte -lftgle -lfreetype2e -lluae -lbullete -lglu32 -lopengl32 -lz -lpthread
rm -rf *.o
echo "'engine' building complete. end."