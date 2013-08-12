"g++.exe" -c "vt/*.cpp" -s -O3 -march=prescott
ar rcs libvte.a *.o
rm -rf *.o