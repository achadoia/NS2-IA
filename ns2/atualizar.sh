#!/bin/bash
# NS2-IA: atualiza a partir do Fritchii oficial (github.com/Frizti/Fritchii) e recompila.
#
#   ns2-base = a tag do Fritchii em que o NS2-IA esta apoiado
#   ns2-mic  = ns2-base + os commits do microfone do Lichii (se o Fritchii ainda nao tiver)
#   ns2-ia   = ns2-mic + as mudancas do NS2-IA (visual, titulo, icone, portugues, AMD, correcoes)
#
# Uso (MSYS2 UCRT64):  bash ns2/atualizar.sh            -> release mais nova
#                      bash ns2/atualizar.sh fritchii-v6 -> uma tag especifica
set -euo pipefail
cd /d/NS2-IA-codigo
UPSTREAM=https://github.com/Frizti/Fritchii.git
MIC_COMMITS="1d86dd61e0be340b077edc2428dd76cb44b8c9f4 168601328630cdcea9f0a1dd608bdde627e68d4b bbf3e62974efa40581333f181d97e7f11bd80690"
GIT="git -c user.name=NS2-IA -c user.email=ns2-ia@local"

TAG="${1:-}"
if [ -z "$TAG" ]; then
    TAG=$(git ls-remote --tags --sort=-v:refname "$UPSTREAM" 'fritchii-v*' | grep -v '\^{}' | head -1 | sed 's#.*refs/tags/##')
fi
echo ">> versao do Fritchii: $TAG"
$GIT fetch --depth 300 "$UPSTREAM" "refs/tags/$TAG:refs/tags/$TAG"
if [ "$(git rev-parse "$TAG^{commit}")" = "$(git rev-parse ns2-base^{commit})" ]; then
    echo ">> ja estamos nessa versao ($TAG). Nada a atualizar."
    exit 0
fi

# o pt_BR.ts e gerado na compilacao: volta ao original antes de mexer no historico
git checkout -- dist/languages/pt_BR.ts 2>/dev/null || true
if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "!! ha mudancas nao salvas (git status). Faca commit delas antes de atualizar."; exit 1
fi
git branch -f ns2-ia-anterior ns2-ia

# 1) microfone: so se o Fritchii novo ainda nao tiver
git checkout -q -B ns2-novo "$TAG"
if git grep -q "AudioBuiltInMicrophone" -- src/core/hle/service/audio; then
    echo ">> o Fritchii novo ja tem o microfone: pulando os commits do Lichii"
else
    echo ">> aplicando o microfone do Lichii"
    for c in $MIC_COMMITS; do
        git cat-file -e "$c^{commit}" 2>/dev/null || $GIT fetch -q --depth 2 https://github.com/zortos293/Lichii.git "$c"
        if ! $GIT cherry-pick -x "$c"; then
            for f in $(git diff --name-only --diff-filter=U); do
                case "$f" in src/tests/*) git checkout --theirs "$f"; git add "$f" ;; esac
            done
            if [ -n "$(git diff --name-only --diff-filter=U)" ]; then
                echo "!! conflito no microfone em: $(git diff --name-only --diff-filter=U)"
                echo "   resolva, rode 'git cherry-pick --continue' e depois rode este script de novo com a mesma tag."
                exit 1
            fi
            GIT_EDITOR=true $GIT cherry-pick --continue || $GIT cherry-pick --skip
        fi
    done
fi
NEW_MIC=$(git rev-parse HEAD)

# 2) mudancas do NS2-IA
echo ">> reaplicando o NS2-IA"
if ! $GIT rebase --onto "$NEW_MIC" ns2-mic ns2-ia; then
    if git diff --name-only --diff-filter=U | grep -q "dist/languages/pt_BR.ts"; then
        git checkout --ours dist/languages/pt_BR.ts 2>/dev/null || git checkout "$NEW_MIC" -- dist/languages/pt_BR.ts
        git add dist/languages/pt_BR.ts
    fi
    if [ -n "$(git diff --name-only --diff-filter=U)" ]; then
        echo "!! conflito em: $(git diff --name-only --diff-filter=U)"
        echo "   resolva, rode 'git rebase --continue' e depois: bash ns2/compilar.sh && bash ns2/empacotar.sh"
        echo "   (para desistir: git rebase --abort; git checkout ns2-ia-anterior)"
        exit 1
    fi
    GIT_EDITOR=true $GIT rebase --continue
fi
git tag -f ns2-base "$TAG" >/dev/null
git tag -f ns2-mic "$NEW_MIC" >/dev/null
git branch -D ns2-novo >/dev/null 2>&1 || true

# 3) traducoes + compilacao + pacote
bash ns2/compilar.sh
bash ns2/empacotar.sh
echo ">> NS2-IA atualizado para $TAG"
