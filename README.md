# Arcanetmame-libretro

-KR-
Arcanetmame-libretro는 라즈베리파이 4와 같은 제한된 성능의 싱글 보드 환경에서 MAME를 실용적으로 구동하기 위해, 프레임 기반(Frame-based) 아키텍처의 마지막 레거시 버전인 MAME 0.135를 기반으로 Libretro용으로 포팅한 프로젝트입니다.

현대의 디바이스 기반(Device-based) MAME는 라즈베리파이 같은 저전력 기기에서 3D 아케이드 게임을 돌리기엔 지나치게 무겁습니다. 본 프로젝트는 가장 대중적인 남코 시스템 12(Namco System 12)의 에뮬레이션을 소프트웨어적 최적화로 가볍게 다듬어, 프레임 기반 MAME의 실용성과 가능성을 증명하고자 하는 실험입니다.

### 네트워크 플레이 지원
* **Kaillera-Reborn (K-arch)** 기반의 변종 레트로아치(RetroArch)를 통한 네트워크 플레이를 지원합니다.
* 특정 드라이버 구동 중 싱크(Desync)가 발생할 경우, 해당 드라이버 레벨의 비결정성(Non-determinism) 요소 보강이 필요할 수 있습니다.

### 설치 및 빌드 가이드
* 상세 빌드 옵션은 포함된 `.bat` 파일을 참고하시기 바랍니다.
* 레트로파이(RetroPie) 사용자는 해당 스크립트를 `scriptmodule` 폴더에 배치하세요.
* 아래의 `arcanetmame_libretro.info` 내용을 파일로 저장하여 레트로아치 `info` 폴더에 넣으시면 됩니다.


-EN-
Arcanetmame-libretro is a Libretro port based on MAME 0.135—the last legacy release using a frame-based architecture—tailored to make MAME practical on low-performance single-board computers like the Raspberry Pi 4.

Modern device-based MAME architecture is far too heavy for SBC hardware when running 3D arcade titles. This project focuses on optimizing Namco System 12 emulation to demonstrate the viability and lightweight efficiency of a frame-based MAME approach through software-level tuning.

### Network Play Support
* Supports network play via **Kaillera-Reborn (K-arch)** variants of RetroArch.
* If desyncs occur, they are typically caused by remaining non-deterministic code paths in specific arcade drivers that require further refinement.

### Installation & Build
* Check the included `.bat` file for build options.
* For RetroPie users, place the script inside your `scriptmodule` folder.
* Save the `.info` block below as `arcanetmame_libretro.info` and place it in your RetroArch `info` directory.

Target ROM Set: MAME 0.135 (ClrMAMEPro DAT compatible)
  
[arcanetmame_libretro.info]  
Software Information  
display_name = "Arcanetmame (MAME 0135)"  
authors = "MAMEdev"  
supported_extensions = "zip|7z"  
corename = "Arcanetmame (0.135)"  
license = "MAME"  
permissions = ""  
display_version = "0.135"  
categories = "Emulator"  


Hardware Information  
manufacturer = "Various"  
systemname = "Arcade (various)"  
systemid = "mame"  


Libretro Features  
supports_no_game = "false"  
database = "MAME 0135"  
savestate = "true"  
savestate_features = "deterministic"  
cheats = "false"  
input_descriptors = "true"  
input_headset = "false"  
memory_descriptors = "false"  
libretro_saves = "true"  
core_options = "true"  
core_options_version = "1.0"  
hw_render = "false"  
disk_control = "false"  
notes = "(!) The BIOS files must be inside the ROM directory.|"  
supports_netplay = "true"  


description = "Frame-based emulation MAME for Rpi4 and Kaillera Netplay"  
