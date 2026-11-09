#!/bin/bash
# NS2-IA: configure (MSYS2 UCRT64, clang + PGO, tuned for Ryzen Zen/Zen+)
set -euo pipefail
cd /d/NS2-IA-codigo
export CPM_SOURCE_CACHE=/d/NS2-IA-codigo/.cache/cpm
profile="-fprofile-use=$(cygpath -m "$PWD/eglantine.profdata") -Wno-backend-plugin -Wno-profile-instr-unprofiled -Wno-profile-instr-out-of-date"
amd="-march=znver1 -mtune=znver1"
cmake -S . -B build/ns2 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DYUZU_TESTS=OFF -DYUZU_CMD=ON \
  -DYUZU_ROOM=OFF -DYUZU_ROOM_STANDALONE=OFF -DYUZU_USE_BUNDLED_QT=OFF \
  -DYUZU_USE_BUNDLED_SDL3=ON -DYUZU_USE_BUNDLED_FFMPEG=ON \
  -DVulkanHeaders_FORCE_BUNDLED=ON -DVulkanUtilityLibraries_FORCE_BUNDLED=ON \
  -DENABLE_UPDATE_CHECKER=OFF -DLICHII_BUILD_TAG=ns2-ia -DLICHII_BUILD_PLATFORM=windows-x86_64-amd \
  -DUSE_DISCORD_PRESENCE=OFF -DUSE_CCACHE=ON -DENABLE_QT_TRANSLATION=ON -DENABLE_WERROR=OFF \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld \
  "-DCMAKE_C_FLAGS=$profile $amd" "-DCMAKE_CXX_FLAGS=$profile $amd"
