"""NS2-IA: traducoes pt-BR proprias, aplicadas por cima do pt_BR.ts do Fritchii.

  python ns2/traduzir.py juntar  -> junta ns2-tr-in-N.json + ns2-tr-out-N.json em ns2/traducoes.json
  python ns2/traduzir.py aplicar -> preenche no dist/languages/pt_BR.ts as entradas sem traducao
                                    que estao em ns2/traducoes.json (chave: contexto + texto original)
Rodar 'aplicar' depois do lupdate (ns2/compilar.sh faz isso).
"""
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TS = ROOT / "dist" / "languages" / "pt_BR.ts"
DB = ROOT / "ns2" / "traducoes.json"
# Traducoes antigas do pt_BR.ts do projeto que estao erradas ou em portugues de Portugal:
# trocadas sempre, em qualquer contexto.
FORCAR = {
    "Controls": "Controles",
    "Audio": "Áudio",
    "Configure Controls": "Configurar controles",
}


def juntar():
    db = json.loads(DB.read_text(encoding="utf-8")) if DB.exists() else {}
    n = 0
    for src_file in sorted(ROOT.glob("ns2-tr-in-*.json")):
        out_file = ROOT / src_file.name.replace("-in-", "-out-")
        if not out_file.exists():
            print("faltando", out_file.name)
            continue
        items = json.loads(src_file.read_text(encoding="utf-8"))
        tr = json.loads(out_file.read_text(encoding="utf-8"))
        for it in items:
            pt = tr.get(str(it["id"]))
            if pt is not None and it["src"] is not None:
                db[it["ctx"] + "\x1f" + it["src"]] = pt
                n += 1
    DB.write_text(json.dumps(db, ensure_ascii=False, indent=0, sort_keys=True), encoding="utf-8")
    print(f"{n} traducoes juntadas, {len(db)} no total em {DB.name}")


def aplicar():
    db = json.loads(DB.read_text(encoding="utf-8"))
    tree = ET.ElementTree(ET.fromstring(TS.read_text(encoding="utf-8")))
    filled = missing = 0
    for ctx in tree.getroot().iter("context"):
        name = ctx.findtext("name")
        for m in ctx.iter("message"):
            tr = m.find("translation")
            if tr is None or m.get("numerus") == "yes":
                continue
            forced = FORCAR.get(m.findtext("source") or "")
            if forced is not None:
                tr.text = forced
                tr.attrib.pop("type", None)
                filled += 1
                continue
            if tr.get("type") != "unfinished" and (tr.text or "").strip():
                continue
            pt = db.get(name + "\x1f" + (m.findtext("source") or ""))
            if pt is None:
                missing += 1
                continue
            tr.text = pt
            tr.attrib.pop("type", None)
            filled += 1
    header = '<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n'
    body = ET.tostring(tree.getroot(), encoding="unicode")
    TS.write_text(header + body + "\n", encoding="utf-8")
    print(f"pt_BR.ts: {filled} traduzidas pelo NS2-IA, {missing} ainda em ingles")


if __name__ == "__main__":
    {"juntar": juntar, "aplicar": aplicar}[sys.argv[1] if len(sys.argv) > 1 else "aplicar"]()
