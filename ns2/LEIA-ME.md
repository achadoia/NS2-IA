# NS2-IA — Nintendo Switch 2 Achado IA

ATENCAO: esta pasta (D:\NS2-IA-codigo) e o codigo-fonte do NS2-IA. Nao apague: sem ela nao da para
corrigir nem atualizar o emulador. O emulador pronto fica em D:\NS2-IA.

O NS2-IA e o Fritchii (github.com/Frizti/Fritchii, GPL-3.0, fork do Lichii/Eden) com:
- microfone do Switch 2 (commits #497, #585 e #881 do Lichii): ocarina por voz no Ocarina of Time Remake
- correcao do risco colorido nas montanhas em placas AMD (Fritchii PR #29 / Lichii #353)
- visual neobrutalista AchadoIA (src/yuzu/fritchii/ns2aia.qss + paleta em fritchii_ui.cpp), icone N-IA
- interface em portugues (ns2/traducoes.json), nome NS2-IA no processo e no titulo
- compilado com Clang + PGO e -march=znver1 (Ryzen Zen/Zen+), MSYS2 UCRT64 em C:\msys64
- reaproveita os caches do Fritchii oficial (datas FILETIME, ordem das capacidades SPIR-V)

Por dentro o nome da build continua "Fritchii" (ajustes do Ocarina, mesmo cache em %APPDATA%\eden).

Scripts (MSYS2 UCRT64, a partir de D:\NS2-IA-codigo):
- bash ns2/compilar.sh   -> traducoes + configurar + compilar (build/ns2)
- bash ns2/empacotar.sh  -> copia para D:\NS2-IA\NS2-IA.exe com as DLLs
- bash ns2/atualizar.sh  -> pega a release mais nova do Fritchii e reaplica tudo

Creditos: Frizti (Fritchii), zortos293 e colaboradores (Lichii), equipe Eden/yuzu. Licenca GPL-3.0.
