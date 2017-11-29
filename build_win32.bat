@echo off

mkdir build
pushd build

set SOURCE="..\src\*.cpp ..\src\scenes\*.cpp ..\src\platform\platform_win32.cpp"
set OUTPUT="-o win32.exe"
set LIBS="..\libs\freetype281.lib msvcrt.lib msvcmrt.lib ..\libs\glew32s.lib kernel32.lib user32.lib shell32.lib gdi32.lib opengl32.lib glu32.lib"
set INCLUDES="/I..\libs\include"
set DEF="/DPLATFORM_WIN32 /DDEBUG=1"

cl -Zi /O1 /W3 "%DEF%" "%OUTPUT%" "%SOURCE%" "%INCLUDES%" "%LIBS%" 

popd