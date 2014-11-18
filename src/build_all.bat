rm -rf *.exe
rm -rf lib/*.a
rm -rf *.o
CALL build_vt.bat 
CALL build_lua.bat 
CALL build_ftgl.bat 
CALL build_freetype2.bat 
CALL build_image.bat
CALL build_bullet.bat 
CALL build_engine.bat 
CALL build_OpenAL.bat
CALL assemble.bat
rm -rf *.o