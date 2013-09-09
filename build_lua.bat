"gcc.exe" -c "lua/*.c" -s -O2 -march=prescott
ar rcs libluae.a *.o
rm -rf *.o