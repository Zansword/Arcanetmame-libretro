#!/usr/bin/env bash

# This file is part of The RetroPie Project
# Custom scriptmodule for lr-mame2009

rp_module_id="lr-mame2009"
rp_module_desc="MAME 2009 libretro port (Custom Repository & Expat Support)"
rp_module_help="ROM Extension: .zip .chd"
rp_module_licence="GPL2"
rp_module_section="opt"
rp_module_flags=""

function depends_lr-mame2009() {
    local depends=(git make libexpat1-dev)
    get_depends "${depends[@]}"
}

function sources_lr-mame2009() {
    gitPullOrClone "$md_build" https://github.com/Zansword/mame2009-libretro.git
}

function build_lr-mame2009() {
    make clean
    if ! dpkg -s libexpat1-dev >/dev/null 2>&1; then
        sudo apt-get install -y libexpat1-dev
    fi

    local PARAMS=("platform=unix" "TARGETOS=linux")

    if [ "$(uname -m)" = "aarch64" ]; then
        PARAMS+=("PTR64=1")
    else
        PARAMS+=("PTR64=0")
    fi

    make -f Makefile "${PARAMS[@]}" -j$(nproc)

    if [ ! -f "mame2009_libretro.so" ]; then
        exit 1
    fi

    md_ret_require="mame2009_libretro.so"
}

function install_lr-mame2009() {
    md_ret_files=(
        "mame2009_libretro.so"
    )
}

function configure_lr-mame2009() {
    local system
    for system in arcade mame-libretro; do
        mkRomDir "$system"
        defaultRAConfig "$system"
        addEmulator 0 "$md_id" "$system" "$md_inst/mame2009_libretro.so"
        addSystem "$system"
    done
}