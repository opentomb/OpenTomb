"gcc.exe" -c "image/*.c" "image/jpeg-9/*.c" "image/libpng-1.6.2/*.c" -DLOAD_JPG -DLOAD_PNG -DLOAD_XPM -DLOAD_TGA -DLOAD_PCX -I"image/jpeg-9" -I"image/libpng-1.6.2" -s -O2 -march=prescott
ar rcs ../lib/libimagee.a *.o
rm -rf *.o