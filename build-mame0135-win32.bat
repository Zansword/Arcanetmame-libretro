@echo off
setlocal

set "MAME_GCC=D:\compiler\mame2007\bin"
set "PATH=%MAME_GCC%;%PATH%"
if not defined RETROARCH_CORES set "RETROARCH_CORES=C:\Users\whban\Downloads\RetroArch_32\RetroArch-Win32\cores"

cd /d "%~dp0"
mingw32-make.exe clean OSD=retro TARGETOS=win32 PTR64=0 CC=gcc LD=gcc AR=ar
mingw32-make.exe -f Makefile.libretro OSD=retro TARGETOS=win32 PTR64=0 CC=gcc LD=gcc AR=ar %* -j8
if errorlevel 1 exit /b %errorlevel%

if exist "%RETROARCH_CORES%\" (
	copy /y "arcanetmame_libretro.dll" "%RETROARCH_CORES%\arcanetmame_libretro.dll" >nul
	if errorlevel 1 exit /b %errorlevel%
	echo Deployed arcanetmame_libretro.dll to "%RETROARCH_CORES%"
) else (
	echo RetroArch cores directory not found: "%RETROARCH_CORES%"
)

echo.
echo Built arcanetmame_libretro.dll
endlocal
