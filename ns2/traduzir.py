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
    "Directory": "Pasta",
}
# Frases com plural (numerus): formas singular e plural.
PLURAIS = {
    "%1 of %n result(s)": ("%1 de %n resultado", "%1 de %n resultados"),
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
            if tr is None:
                continue
            if m.get("numerus") == "yes":
                forms = PLURAIS.get(m.findtext("source") or "")
                if forms is not None:
                    for child in list(tr):
                        tr.remove(child)
                    tr.text = None
                    for form in forms:
                        ET.SubElement(tr, "numerusform").text = form
                    tr.attrib.pop("type", None)
                    filled += 1
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
    # Os menus do Fritchii usam QObject::tr dentro de FritchiiUi: o lupdate guarda os textos no
    # contexto "FritchiiUi", mas em tempo de execucao eles sao procurados em "QObject".
    root = tree.getroot()
    fritchii = next((c for c in root.iter("context") if c.findtext("name") == "FritchiiUi"), None)
    if fritchii is not None:
        qobject = next((c for c in root.iter("context") if c.findtext("name") == "QObject"), None)
        if qobject is None:
            qobject = ET.SubElement(root, "context")
            ET.SubElement(qobject, "name").text = "QObject"
        have = {m.findtext("source") for m in qobject.iter("message")}
        for m in fritchii.iter("message"):
            if m.findtext("source") not in have and m.find("translation") is not None:
                qobject.append(ET.fromstring(ET.tostring(m)))
    # Classes do Fritchii dentro do namespace FritchiiUi: em tempo de execucao o contexto e
    # "FritchiiUi::<Classe>", mas o lupdate grava so "<Classe>". Copia para o nome completo.
    for cls in ("GameControls", "Home", "PerfOverlay", "PrecompileGate"):
        short = next((c for c in root.iter("context") if c.findtext("name") == cls), None)
        full_name = "FritchiiUi::" + cls
        if short is None or any(c.findtext("name") == full_name for c in root.iter("context")):
            continue
        full = ET.SubElement(root, "context")
        ET.SubElement(full, "name").text = full_name
        for m in short.iter("message"):
            full.append(ET.fromstring(ET.tostring(m)))
    header = '<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n'
    body = ET.tostring(tree.getroot(), encoding="unicode")
    TS.write_text(header + body + "\n", encoding="utf-8")
    print(f"pt_BR.ts: {filled} traduzidas pelo NS2-IA, {missing} ainda em ingles")


if __name__ == "__main__":
    {"juntar": juntar, "aplicar": aplicar}[sys.argv[1] if len(sys.argv) > 1 else "aplicar"]()
