"gcc.exe" -c "al/OpenAL32/*.c" "al/Alc/*.c" "al/Alc/effects/*.c" "al/Alc/backends/*.c" -I"al" -s -O2 -march=prescott
ar rcs ../lib/libopenale.a *.o
rm -rf *.o