"gcc.exe" -c "lua/*.c" -s -O3 -march=prescott
ar rcs libluae.a *.o
rm -rf *.o