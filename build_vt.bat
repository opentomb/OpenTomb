"g++.exe" -c "vt/*.cpp" -s -O2 -march=prescott
ar rcs libvte.a *.o
rm -rf *.o