@echo off
setlocal

set "MAME_MINGW=D:\compiler\MSYS2\ucrt64\bin"
set "MAME_MAKE=D:\compiler\Buildtools_mame\buildtools\vendor\mingw64\bin\make.exe"
set "PATH=%MAME_MINGW%;D:\compiler\MSYS2\usr\bin;%PATH%"

cd /d "%~dp0"
"%MAME_MAKE%" clean OSD=retro TARGETOS=win32 PTR64=1 CC=gcc LD=gcc AR=ar
"%MAME_MAKE%" -f Makefile.libretro OSD=retro TARGETOS=win32 PTR64=1 CC=gcc LD=gcc AR=ar %*
if errorlevel 1 exit /b %errorlevel%

echo.
echo Built arcanetmame_libretro.dll for Win64
endlocal
