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
    get_depends "${depends[@]}"
}

function sources_lr-arcanetmame() {
	local repo="${ARCANETMAME_REPO:-https://github.com/Zansword/Arcanetmame-libretro.git}"

    if [ -n "${ARCANETMAME_SOURCE_DIR:-}" ]; then
        rm -rf "$md_build"
        mkdir -p "$md_build"
        cp -a "${ARCANETMAME_SOURCE_DIR}/." "$md_build/"
        return
    fi

    gitPullOrClone "$md_build" "$repo" "${ARCANETMAME_BRANCH:-main}"
}

function build_lr-arcanetmame() {
    local build_dir="$md_build"
    local params=("OSD=retro" "TARGETOS=linux")

    if [ -f "$md_build/mame0135s/mame/Makefile.libretro" ]; then
        build_dir="$md_build/mame0135s/mame"
    fi

    if [ ! -f "$build_dir/Makefile.libretro" ]; then
        md_ret_errors+=("ArcanetMame Makefile.libretro was not found in the source tree.")
        return 1
    fi

    if [ "$(getconf LONG_BIT)" = "64" ]; then
        params+=("PTR64=1")
    else
        params+=("PTR64=0")
    fi

    cd "$build_dir" || return 1
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
