"g++.exe" -c "vt/*.cpp" -s -O2 -march=prescott
ar rcs ../lib/libvte.a *.o
rm -rf *.o