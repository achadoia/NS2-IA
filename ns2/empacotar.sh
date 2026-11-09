#!/bin/bash
# NS2-IA: copia o build para D:\NS2-IA com as DLLs necessarias (rodar no MSYS2 UCRT64)
set -euo pipefail
export PATH="/ucrt64/bin:$PATH"
SRC=/d/NS2-IA-codigo/build/ns2/bin
DST=/d/NS2-IA
mkdir -p "$DST"
cp -f "$SRC/lichii.exe" "$DST/NS2-IA.exe"
cp -f "$SRC/eden-cli.exe" "$DST/NS2-IA-cli.exe"
rm -f "$DST/eden-cli.exe"
cd "$DST"
windeployqt6 --no-translations --no-compiler-runtime --no-opengl-sw \
  --no-system-dxc-compiler --no-system-d3d-compiler NS2-IA.exe >/dev/null
deps() {
  objdump -p "$1" | awk '/DLL Name:/ {print $3}' | while read -r dll; do
    [ -z "$dll" ] && continue
    source=$(command -v "$dll" 2>/dev/null || true)
    [ -z "$source" ] && continue
    case "$source" in *System32* | *system32* | *SysWOW64*) continue ;; esac
    if [ ! -f "$dll" ]; then cp "$source" "$dll"; deps "$source"; fi
  done
}
for exe in NS2-IA.exe NS2-IA-cli.exe; do deps "$exe"; done
find . -name '*.dll' | while read -r dll; do deps "$dll"; done
# receitas de shaders do Fritchii (OoT cache v54): nao vem no codigo-fonte
for r in /d/Fritchii-v5-Windows-x64/recipes; do
  [ -d "$r" ] && mkdir -p "$DST/recipes" && cp -n "$r"/* "$DST/recipes/" 2>/dev/null || true
done
echo "pronto: $DST/NS2-IA.exe"
