"g++.exe" -c "ftgl/*.cpp" -I"freetype2" -s -O3 -march=prescott
ar rcs libftgle.a *.o
rm -rf *.o