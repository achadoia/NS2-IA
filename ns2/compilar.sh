#!/bin/bash
# NS2-IA: traducoes + configurar + compilar (rodar no MSYS2 UCRT64)
set -euo pipefail
cd /d/NS2-IA-codigo
export PATH="/ucrt64/bin:$PATH"
# textos novos do Fritchii -> pt_BR.ts, depois as nossas traducoes por cima
lupdate -locations none -no-obsolete src/yuzu src/qt_common -ts dist/languages/pt_BR.ts >/dev/null
python ns2/traduzir.py aplicar
bash ns2/configurar.sh
export CCACHE_DIR=/d/NS2-IA-codigo/.ccache
# 10 de 12 threads: o PC continua usavel
cmake --build build/ns2 --target yuzu yuzu-cmd --parallel 10
