"gcc.exe" -c "lua/*.c" -s -O2 -march=prescott
ar rcs lib/libluae.a *.o
rm -rf *.o