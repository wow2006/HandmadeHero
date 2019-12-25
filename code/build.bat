@echo off

mkdir ..\..\build
pushd ..\..\build
cl -Zi ..\src\code\win32_handmade.cpp user32.lib Gdi32.lib
popd
