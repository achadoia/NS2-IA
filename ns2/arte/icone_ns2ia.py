"""Icone NS2-IA: neobrutalista AchadoIA, controle (joystick) + sigla N-IA."""
import sys
from PIL import Image, ImageDraw, ImageFont

INK = (20, 20, 19, 255)
CREAM = (250, 249, 245, 255)
YELLOW = (255, 214, 10, 255)
PINK = (255, 107, 157, 255)
CYAN = (76, 201, 240, 255)
LIME = (166, 235, 60, 255)
WHITE = (255, 255, 255, 255)

S = 1024


def rr(d, box, r, fill, grow=0):
    x0, y0, x1, y1 = box
    d.rounded_rectangle((x0 - grow, y0 - grow, x1 + grow, y1 + grow), r + grow, fill=fill)


def ell(d, box, fill, grow=0):
    x0, y0, x1, y1 = box
    d.ellipse((x0 - grow, y0 - grow, x1 + grow, y1 + grow), fill=fill)


def boxed(d, box, fill, border, shadow):
    x0, y0, x1, y1 = box
    d.rectangle((x0 + shadow, y0 + shadow, x1 + shadow, y1 + shadow), fill=INK)
    d.rectangle(box, fill=INK)
    d.rectangle((x0 + border, y0 + border, x1 - border, y1 - border), fill=fill)


def draw(size_hint):
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    small = size_hint <= 32
    border = 56 if small else 40
    shadow = 0 if small else 44
    # fundo amarelo, borda grossa, sombra dura
    boxed(d, (24, 24, S - 24 - shadow, S - 24 - shadow), YELLOW, border, shadow)

    # controle: corpo + duas empunhaduras, contorno preto grosso
    ox, oy = (0, 40) if small else (-10, 0)
    body = (180 + ox, 170 + oy, 790 + ox, 450 + oy)
    gl = (180 + ox, 290 + oy, 420 + ox, 560 + oy)
    gr = (550 + ox, 290 + oy, 790 + ox, 560 + oy)
    g = 34 if small else 28
    rr(d, body, 120, INK, g)
    ell(d, gl, INK, g)
    ell(d, gr, INK, g)
    rr(d, body, 120, WHITE)
    ell(d, gl, WHITE)
    ell(d, gr, WHITE)
    # direcional (cruz)
    cx, cy, a, b = 315 + ox, 320 + oy, 34, 100
    d.rectangle((cx - a, cy - b, cx + a, cy + b), fill=INK)
    d.rectangle((cx - b, cy - a, cx + b, cy + a), fill=INK)
    # botoes
    bx, by, rad, sp = 655 + ox, 320 + oy, 40, 78
    for (dx, dy), col in (((0, -1), CYAN), ((1, 0), PINK), ((0, 1), LIME), ((-1, 0), YELLOW)):
        x, y = bx + dx * sp, by + dy * sp
        ell(d, (x - rad, y - rad, x + rad, y + rad), INK, 14)
        ell(d, (x - rad, y - rad, x + rad, y + rad), col)
    if not small:
        # analogicos
        for x in (420 + ox, 550 + ox):
            ell(d, (x - 42, 440 + oy - 42, x + 42, 440 + oy + 42), INK, 0)
            ell(d, (x - 24, 440 + oy - 24, x + 24, 440 + oy + 24), CREAM)

    # etiqueta rosa com a sigla
    if small:
        lab = (90, 640, S - 90, 930)
        boxed(d, lab, PINK, 40, 0)
        fsize = 250
    else:
        lab = (110, 620, S - 150, 880)
        boxed(d, lab, PINK, 28, 26)
        fsize = 200
    font = ImageFont.truetype("C:/Windows/Fonts/ariblk.ttf", fsize)
    text = "N-IA"
    l, t, r, btm = d.textbbox((0, 0), text, font=font)
    tx = (lab[0] + lab[2]) / 2 - (l + r) / 2
    ty = (lab[1] + lab[3]) / 2 - (t + btm) / 2
    d.text((tx, ty), text, font=font, fill=INK)
    return img


def main(out_ico, out_png):
    sizes = [16, 24, 32, 48, 64, 128, 256]
    big = draw(256)
    small = draw(32)
    frames = [(small if s <= 32 else big).resize((s, s), Image.LANCZOS) for s in sizes]
    frames[-1].save(out_ico, format="ICO", sizes=[(s, s) for s in sizes],
                    append_images=frames[:-1])
    big.resize((512, 512), Image.LANCZOS).save(out_png)
    prev = Image.new("RGBA", (16 + 32 + 48 + 64 + 256 + 60, 266), (240, 240, 240, 255))
    x = 10
    for s in (16, 32, 48, 64, 256):
        f = frames[sizes.index(s)]
        prev.alpha_composite(f, (x, 5))
        x += s + 10
    prev.save(out_png.replace(".png", "_tamanhos.png"))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
