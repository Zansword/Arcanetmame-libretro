#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

if [ "$(getconf LONG_BIT)" = "64" ]; then
    PTR64=1
else
    PTR64=0
fi

make clean OSD=retro TARGETOS=linux PTR64="$PTR64"
make -f Makefile.libretro OSD=retro TARGETOS=linux PTR64="$PTR64" -j"$(nproc)"

echo "Built arcanetmame_libretro.so"