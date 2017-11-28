@echo off

mkdir build
pushd build

set SOURCE="..\src\platform\platform_win32.cpp"
set OUTPUT="-o win32.exe"
set LIBS="kernel32.lib user32.lib shell32.lib gdi32.lib opengl32.lib glu32.lib"

cl -Zi /O1 /Wall "%OUTPUT%" "%SOURCE%" "%LIBS%"

popd