#!/usr/bin/env bash

# This file is part of The RetroPie Project
# Custom scriptmodule for ArcanetMame.

rp_module_id="lr-arcanetmame"
rp_module_desc="ArcanetMame libretro port (MAME 0.135 based)"
rp_module_help="ROM Extension: .zip .chd .7z"
rp_module_licence="GPL2"
rp_module_section="opt"
rp_module_flags=""

function depends_lr-arcanetmame() {
    local depends=(git make gcc g++ zlib1g-dev libexpat1-dev)
    getDepends "${depends[@]}"
}

function sources_lr-arcanetmame() {
    gitPullOrClone "$md_build" "https://github.com/Zansword/Arcanetmame-libretro.git" main
}

function build_lr-arcanetmame() {
    local params=("OSD=retro" "TARGETOS=linux" "CC=cc" "LD=cc" "AR=ar")

    if [ "$(getconf LONG_BIT)" = "64" ]; then
        params+=("PTR64=1")
    else
        params+=("PTR64=0")
    fi

    cd "$md_build" || return 1
    make clean "${params[@]}"
    make -f Makefile.libretro "${params[@]}" -j"$(nproc)"

    if [ ! -f "arcanetmame_libretro.so" ]; then
        md_ret_errors+=("arcanetmame_libretro.so was not produced by the build.")
        return 1
    fi

    md_ret_require="arcanetmame_libretro.so"
}

function install_lr-arcanetmame() {
    md_ret_files=(
        "arcanetmame_libretro.so"
    )
}

function configure_lr-arcanetmame() {
    local system
    for system in arcade mame-libretro; do
        mkRomDir "$system"
        defaultRAConfig "$system"
        addEmulator 0 "$md_id" "$system" "$md_inst/arcanetmame_libretro.so"
        addSystem "$system"
    done
}
