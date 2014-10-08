"g++.exe" -c "ftgl/*.cpp" -I"freetype2" -s -O2 -march=prescott
ar rcs ../lib/libftgle.a *.o
rm -rf *.o