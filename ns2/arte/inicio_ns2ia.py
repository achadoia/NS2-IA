"""Arte do inicio do NS2-IA: fundo escuro com faixas diagonais e controle desenhado (arte propria,
nada da Nintendo) + logo "NS2-IA". Gera src/yuzu/fritchii/ns2/fundo.jpg e logo.png.

  python ns2/arte/inicio_ns2ia.py
"""
import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "src" / "yuzu" / "fritchii" / "ns2"
INK = (20, 20, 19)
YELLOW = (255, 214, 10)
PINK = (255, 107, 157)
CYAN = (76, 201, 240)
LIME = (166, 235, 60)
CREAM = (250, 249, 245)
BLACK_FONT = "C:/Windows/Fonts/ariblk.ttf"

W, H = 1920, 1080


def band(d, x0, width, color, slope=0.9, y0=-200, y1=H + 200):
    """Faixa diagonal (paralelogramo) que sobe da esquerda para a direita."""
    dx = (y1 - y0) * slope
    d.polygon([(x0, y1), (x0 + width, y1), (x0 + width + dx, y0), (x0 + dx, y0)], fill=color)


def halftone(img, cx, cy, radius, color, step=18, max_r=5.0, seed=1):
    d = ImageDraw.Draw(img)
    rnd = random.Random(seed)
    for y in range(int(cy - radius), int(cy + radius), step):
        for x in range(int(cx - radius), int(cx + radius), step):
            dist = math.hypot(x - cx, y - cy) / radius
            if dist >= 1:
                continue
            r = max_r * (1 - dist) * (0.7 + 0.3 * rnd.random())
            if r > 0.6:
                d.ellipse((x - r, y - r, x + r, y + r), fill=color)


def gamepad(img, cx, cy, s):
    """Controle grande, em linhas, no canto inferior direito (estilo do icone N-IA)."""
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    body = (cx - 300 * s, cy - 120 * s, cx + 300 * s, cy + 110 * s)
    gl = (cx - 330 * s, cy - 40 * s, cx - 90 * s, cy + 240 * s)
    gr = (cx + 90 * s, cy - 40 * s, cx + 330 * s, cy + 240 * s)
    edge = int(14 * s)

    def shapes(fill, grow):
        d.rounded_rectangle((body[0] - grow, body[1] - grow, body[2] + grow, body[3] + grow),
                            int(120 * s) + grow, fill=fill)
        for g in (gl, gr):
            d.ellipse((g[0] - grow, g[1] - grow, g[2] + grow, g[3] + grow), fill=fill)

    shapes(YELLOW + (230,), edge)          # contorno amarelo
    shapes((30, 30, 28, 255), 0)            # corpo escuro
    # direcional
    x, y, a, b = cx - 185 * s, cy - 10 * s, 26 * s, 78 * s
    d.rectangle((x - a, y - b, x + a, y + b), fill=(55, 55, 52, 255))
    d.rectangle((x - b, y - a, x + b, y + a), fill=(55, 55, 52, 255))
    # botoes
    bx, by, r, sp = cx + 185 * s, cy - 10 * s, 30 * s, 62 * s
    for (dx, dy), col in (((0, -1), CYAN), ((1, 0), PINK), ((0, 1), LIME), ((-1, 0), YELLOW)):
        px, py = bx + dx * sp, by + dy * sp
        d.ellipse((px - r - 6 * s, py - r - 6 * s, px + r + 6 * s, py + r + 6 * s), fill=INK + (255,))
        d.ellipse((px - r, py - r, px + r, py + r), fill=col + (255,))
    # analogicos
    for ax in (cx - 80 * s, cx + 80 * s):
        ay = cy + 95 * s
        d.ellipse((ax - 52 * s, ay - 52 * s, ax + 52 * s, ay + 52 * s), fill=INK + (255,))
        d.ellipse((ax - 36 * s, ay - 36 * s, ax + 36 * s, ay + 36 * s), fill=(60, 60, 56, 255))
    layer = layer.rotate(-14, resample=Image.BICUBIC, center=(cx, cy))
    shadow = Image.new("RGBA", img.size, (0, 0, 0, 0))
    shadow.paste((0, 0, 0, 200), (0, 0), layer.split()[3])
    shadow = shadow.transform(img.size, Image.AFFINE, (1, 0, -22, 0, 1, -22))
    img.alpha_composite(shadow)
    img.alpha_composite(layer)


def fundo():
    img = Image.new("RGBA", (W, H), INK + (255,))
    # granulado leve
    rnd = random.Random(7)
    noise = Image.new("L", (W // 4, H // 4))
    noise.putdata([rnd.randint(0, 28) for _ in range(noise.width * noise.height)])
    noise = noise.resize((W, H), Image.NEAREST).filter(ImageFilter.GaussianBlur(1))
    grain = Image.new("RGBA", (W, H), (255, 255, 255, 0))
    grain.putalpha(noise)
    img.alpha_composite(grain)

    d = ImageDraw.Draw(img)
    # faixas do canto inferior esquerdo (como no print, nas cores AchadoIA)
    band(d, -620, 120, YELLOW, y0=520)
    band(d, -470, 40, CREAM, y0=600)
    band(d, -400, 70, PINK, y0=650)
    band(d, -300, 26, CYAN, y0=720)
    band(d, -230, 16, CREAM, y0=780)
    # faixas do topo (atras do logo) e do canto superior direito
    band(d, 380, 120, YELLOW, y0=-40, y1=110)
    band(d, 530, 34, CREAM, y0=-40, y1=110)
    band(d, 590, 60, PINK, y0=-40, y1=110)
    band(d, 1500, 90, YELLOW + (255,), y0=-40, y1=60)
    band(d, 1620, 30, CYAN, y0=-40, y1=60)
    # faixa fina na borda do topo
    d.rectangle((0, 104, W, 108), fill=YELLOW)
    # retículas
    halftone(img, 1650, 330, 260, PINK + (255,), seed=3)
    halftone(img, 1820, 760, 220, CYAN + (255,), seed=4)
    halftone(img, 260, 980, 260, YELLOW + (255,), seed=5)
    gamepad(img, 1500, 800, 1.15)
    # escurece um pouco para a lista ficar legivel
    img.alpha_composite(Image.new("RGBA", (W, H), (0, 0, 0, 70)))
    OUT.mkdir(parents=True, exist_ok=True)
    img.convert("RGB").save(OUT / "fundo.jpg", quality=90)


def logo():
    h = 84
    font = ImageFont.truetype(BLACK_FONT, 60)
    text = "NS2-IA"
    tmp = ImageDraw.Draw(Image.new("RGBA", (10, 10)))
    l, t, r, b = tmp.textbbox((0, 0), text, font=font)
    tw = r - l
    w = 110 + tw + 40
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    # duas barras inclinadas antes do nome
    for i, col in enumerate((YELLOW, PINK)):
        x = 14 + i * 34
        d.polygon([(x, h - 10), (x + 18, h - 10), (x + 58, 8), (x + 40, 8)], fill=col + (255,))
    # nome com contorno preto e sombra dura
    tx, ty = 110 - l, (h - (b - t)) // 2 - t
    d.text((tx + 5, ty + 5), text, font=font, fill=INK + (255,))
    d.text((tx, ty), text, font=font, fill=CREAM + (255,), stroke_width=3, stroke_fill=INK + (255,))
    img = img.transform(img.size, Image.AFFINE, (1, 0.18, -12, 0, 1, 0), resample=Image.BICUBIC)
    img.save(OUT / "logo.png")


def logo_lateral():
    """Icone N-IA + nome embaixo, para o pe da barra lateral das configuracoes."""
    icon = Image.open(ROOT / "ns2" / "arte" / "ns2ia.png").convert("RGBA").resize((120, 120), Image.LANCZOS)
    font = ImageFont.truetype(BLACK_FONT, 34)
    w, h = 190, 190
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    img.alpha_composite(icon, ((w - 120) // 2, 0))
    d = ImageDraw.Draw(img)
    l, t, r, b = d.textbbox((0, 0), "NS2-IA", font=font)
    tx, ty = (w - (r - l)) // 2 - l, 132 - t
    d.text((tx + 3, ty + 3), "NS2-IA", font=font, fill=INK + (255,))
    d.text((tx, ty), "NS2-IA", font=font, fill=CREAM + (255,), stroke_width=2, stroke_fill=INK + (255,))
    img.save(OUT / "logo_lateral.png")


if __name__ == "__main__":
    fundo()
    logo()
    logo_lateral()
    print("ok:", OUT)
