"""Reaplica as mudancas do NS2-IA sobre o Fritchii v5.5 + microfone (D:\\NS2-IA-codigo)."""
import io
import os
import shutil

ROOT = r"D:\NS2-IA-codigo"
SCR = os.path.dirname(os.path.abspath(__file__))


def rd(p):
    return io.open(os.path.join(ROOT, p), encoding="utf-8", newline="").read()


def wr(p, s):
    io.open(os.path.join(ROOT, p), "w", encoding="utf-8", newline="").write(s)


def sub(path, old, new, count=1):
    s = rd(path)
    if "\r\n" in s:
        old = old.replace("\n", "\r\n")
        new = new.replace("\n", "\r\n")
    assert s.count(old) == count, (path, old[:80], s.count(old))
    wr(path, s.replace(old, new))


def add_include(path, inc):
    s = rd(path)
    if inc in s:
        return
    nl = "\r\n" if "\r\n" in s else "\n"
    i = s.index("#include")
    wr(path, s[:i] + inc + nl + s[i:])


def versioninfo(desc, internal, original, nl):
    return f'''
// NS2-IA: the name Windows shows for the process
VS_VERSION_INFO VERSIONINFO
 FILEVERSION 5,5,0,0
 PRODUCTVERSION 5,5,0,0
 FILEFLAGSMASK 0x3fL
 FILEFLAGS 0x0L
 FILEOS VOS_NT_WINDOWS32
 FILETYPE VFT_APP
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "041604b0"
        BEGIN
            VALUE "CompanyName", "Achado IA"
            VALUE "FileDescription", "{desc}"
            VALUE "FileVersion", "5.5"
            VALUE "InternalName", "{internal}"
            VALUE "LegalCopyright", "GPL-3.0 - baseado no Fritchii, Lichii e Eden"
            VALUE "OriginalFilename", "{original}"
            VALUE "ProductName", "NS2-IA - Nintendo Switch 2 Achado IA"
            VALUE "ProductVersion", "5.5"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x416, 1200
    END
END
'''.replace("\n", nl)


# --- icone e versao dos exes ---
shutil.copy(os.path.join(SCR, "ns2ia.ico"), os.path.join(ROOT, "dist", "ns2aia.ico"))
for rc, desc, internal, original, old_icon in (
    ("src/yuzu/yuzu.rc", "NS2-IA", "NS2-IA", "NS2-IA.exe", '"../../dist/fritchii.ico"'),
    ("src/yuzu_cmd/yuzu.rc", "NS2-IA (linha de comando)", "NS2-IA-cli", "NS2-IA-cli.exe",
     '"../../dist/fritchii.ico"'),
):
    s = rd(rc)
    nl = "\r\n" if "\r\n" in s else "\n"
    assert old_icon in s, rc
    s = s.replace(old_icon, '"../../dist/ns2aia.ico"')
    wr(rc, s.rstrip() + nl + versioninfo(desc, internal, original, nl))
sub("src/yuzu/yuzu.qrc", '<file alias="lichii.ico">../../dist/fritchii.ico</file>',
    '<file alias="lichii.ico">../../dist/ns2aia.ico</file>')

# --- titulos e idioma ---
sub("src/yuzu/bootmanager.cpp",
    'setWindowTitle(QStringLiteral("Fritchii %1").arg(QString::fromUtf8(Common::g_build_version)));',
    'setWindowTitle(QStringLiteral("NS2-IA %1").arg(QString::fromUtf8(Common::g_build_version)));')
sub("src/yuzu/main_window.cpp",
    '''        fmt::format("{} | {} | {}", std::string{Common::g_build_name},
                    std::string{Common::g_build_version}, std::string{Common::g_compiler_id});''',
    '''        // NS2-IA: the internal build name stays "Fritchii" so its tuned paths stay on.
        fmt::format("NS2-IA | {} | {}", std::string{Common::g_build_version},
                    std::string{Common::g_compiler_id});''')
sub("src/yuzu/main_window.cpp",
    '''    bool loaded;

    if (UISettings::values.language.GetValue().empty()) {''',
    '''    bool loaded;

    // NS2-IA: Portuguese by default (older builds saved "en" when they had no translations).
    if (UISettings::values.language.GetValue().empty() ||
        UISettings::values.language.GetValue() == "en") {
        UISettings::values.language = std::string("pt_BR");
    }

    if (UISettings::values.language.GetValue().empty()) {''')

# --- tema ---
F = "src/yuzu/fritchii/fritchii_ui.cpp"
sub(F, 'tr("&About Fritchii")', 'tr("&About NS2-IA")')
sub(F, '''// Graphite
const QColor Background{0x11, 0x11, 0x13};
const QColor Raised{0x1b, 0x1b, 0x1f};
const QColor Field{0x18, 0x18, 0x1b};
const QColor Button{0x1f, 0x1f, 0x24};
const QColor Text{0xec, 0xec, 0xf1};
const QColor TextSecondary{0x9a, 0x9a, 0xa8};
const QColor TextDisabled{0x55, 0x55, 0x5f};
const QColor IconIdle{0x6b, 0x6b, 0x78};
const QColor Accent{0xa7, 0x8b, 0xfa};
const QColor OnAccent{0x1c, 0x14, 0x33};''', '''// NS2-IA: AchadoIA neobrutalism (cream, black ink, flat yellow / pink / cyan / lime)
const QColor Background{0xfa, 0xf9, 0xf5};
const QColor Raised{0xff, 0xff, 0xff};
const QColor Field{0xff, 0xff, 0xff};
const QColor Button{0xff, 0xff, 0xff};
const QColor Text{0x14, 0x14, 0x13};
const QColor TextSecondary{0x4a, 0x48, 0x42};
const QColor TextDisabled{0x82, 0x7e, 0x76};
const QColor IconIdle{0xd6, 0xd3, 0xca};
const QColor Accent{0xff, 0xd6, 0x0a};
const QColor OnAccent{0x14, 0x14, 0x13};''')
sub(F, '''    p.setColor(QPalette::Light, QColor(0x2e, 0x2e, 0x35));
    p.setColor(QPalette::Midlight, QColor(0x26, 0x26, 0x2c));
    p.setColor(QPalette::Mid, QColor(0x22, 0x22, 0x27));
    p.setColor(QPalette::Dark, QColor(0x0c, 0x0c, 0x0e));
    p.setColor(QPalette::Shadow, QColor(0x05, 0x05, 0x06));''', '''    p.setColor(QPalette::Highlight, QColor(0xff, 0x6b, 0x9d));
    p.setColor(QPalette::Light, QColor(0xff, 0xff, 0xff));
    p.setColor(QPalette::Midlight, QColor(0xee, 0xec, 0xe4));
    p.setColor(QPalette::Mid, QColor(0xd6, 0xd3, 0xca));
    p.setColor(QPalette::Dark, QColor(0x14, 0x14, 0x13));
    p.setColor(QPalette::Shadow, QColor(0x14, 0x14, 0x13));''')
sub(F, 'const QPixmap hover = RenderSvg(svg, Text, px);',
    'const QPixmap hover = RenderSvg(svg, QColor(0xff, 0xff, 0xff), px);')
sub(F, '''    QFont font(QStringLiteral("Inter"));
    font.setPixelSize(14);''', '''    QFont font(QStringLiteral("Segoe UI"));
    font.setPixelSize(14);
    font.setWeight(QFont::DemiBold);''')
sub(F, '''    // Folder and refresh icons come from the dark icon set.
    QIcon::setThemeName(QStringLiteral("colorful_dark"));

    QFile qss(QStringLiteral(":/fritchii/style.qss"));''', '''    // Light background: the light icon set.
    QIcon::setThemeName(QStringLiteral("colorful"));

    QFile qss(QStringLiteral(":/fritchii/ns2aia.qss"));''')
sub(F, '''QProgressBar {
  background-color: rgba(255, 255, 255, 30);
  border: none;
  border-radius: 3px;
  max-height: 6px;
}
QProgressBar::chunk {
  background-color: #a78bfa;
  border-radius: 3px;
})";''', '''QProgressBar {
  background-color: #faf9f5;
  border: 2px solid #141413;
  border-radius: 0;
  max-height: 10px;
}
QProgressBar::chunk {
  background-color: #ffd60a;
})";''')
sub("src/yuzu/fritchii/fritchii.qrc", "        <file>style.qss</file>\n",
    "        <file>style.qss</file>\n        <file>ns2aia.qss</file>\n")
H = "src/yuzu/fritchii/home.cpp"
sub(H, "0x1c1433", "0x141413", 2)
sub(H, "painter.fillRect(QRect(0, 0, size, size), QColor(0x24, 0x23, 0x2c));",
    "painter.fillRect(QRect(0, 0, size, size), QColor(0xff, 0x6b, 0x9d));")
sub(H, '''            painter.drawPixmap(QRect(0, 0, size, size), source);
        }''', '''            painter.drawPixmap(QRect(0, 0, size, size), source);
        }
        // NS2-IA: thick black frame on every cover.
        const int frame = std::max(3, size / 40);
        painter.setPen(QPen(QColor(0x14, 0x14, 0x13), frame));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(frame / 2.0, frame / 2.0, size - frame, size - frame));''')
sub(H, "Rounded(hero->icon, CoverSize, 18)", "Rounded(hero->icon, CoverSize, 0)")
sub(H, "Rounded(game.icon, TileIcon, 12)", "Rounded(game.icon, TileIcon, 0)")
G = "src/yuzu/fritchii/game_controls.cpp"
sub(G, "constexpr unsigned IconColor = 0xececf1;", "constexpr unsigned IconColor = 0x141413;")
sub(G, '''    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(255, 255, 255, 22));
    painter.setBrush(QColor(0x16, 0x16, 0x1a, 235));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 14, 14);''', '''    // NS2-IA: flat cream box, thick black border, hard shadow.
    const QRect box = rect().adjusted(0, 0, -5, -5);
    painter.fillRect(box.translated(5, 5), QColor(0x14, 0x14, 0x13));
    painter.fillRect(box, QColor(0xfa, 0xf9, 0xf5));
    painter.setPen(QPen(QColor(0x14, 0x14, 0x13), 3));
    painter.drawRect(box.adjusted(1, 1, -2, -2));''')

# --- MinGW: dwmapi ---
sub("src/video_core/CMakeLists.txt", "    target_link_libraries(video_core PRIVATE advapi32)",
    "    # dwmapi: vk_host_pacer (MinGW ignores #pragma comment(lib))\n"
    "    target_link_libraries(video_core PRIVATE advapi32 dwmapi)")

# --- datas de arquivo no formato do Fritchii (MSVC) ---
io.open(os.path.join(ROOT, "src/common/fs/ns2_file_time.h"), "w", encoding="utf-8", newline="\n").write(
    io.open(os.path.join(SCR, "ns2_file_time.h"), encoding="utf-8").read())
INC = '#include "common/fs/ns2_file_time.h"'
V = "src/video_core/renderer_vulkan/vk_pipeline_cache.cpp"
sub(V, '''    const auto time = std::filesystem::last_write_time(file, ec);
    if (ec) {
        return std::nullopt;
    }
    return WarmStamp{size,
                     static_cast<s64>(time.time_since_epoch().count()),''', '''    const auto time = Common::FS::WriteTimeTicks(file, ec);
    if (ec) {
        return std::nullopt;
    }
    return WarmStamp{size,
                     static_cast<s64>(time),''')
sub(V, "const auto modified = std::filesystem::last_write_time(path, error).time_since_epoch().count();",
    "const auto modified = Common::FS::WriteTimeTicks(path, error);")
add_include(V, INC)
P = "src/video_core/nx2_precompile.cpp"
sub(P, '''    const auto time = std::filesystem::last_write_time(path, error);
    if (error) {
        return {};
    }
    return fmt::format("{} {}", size, time.time_since_epoch().count());''', '''    const auto time = Common::FS::WriteTimeTicks(path, error);
    if (error) {
        return {};
    }
    return fmt::format("{} {}", size, time);''')
add_include(P, INC)
C = "src/video_core/pipeline_cache_files.cpp"
sub(C, '''    const auto modified = std::filesystem::last_write_time(file, error);
    if (error) {
        return std::nullopt;
    }
    return PipelineCacheFileStamp{
        .size = size,
        .modified = static_cast<s64>(modified.time_since_epoch().count()),''', '''    const auto modified = Common::FS::WriteTimeTicks(file, error);
    if (error) {
        return std::nullopt;
    }
    return PipelineCacheFileStamp{
        .size = size,
        .modified = static_cast<s64>(modified),''')
add_include(C, INC)

# --- shaders guardados: ignora a ordem de OpCapability/OpExtension ---
sub(V, '''/// What differs between a translation's stages (SPIR-V and serialized infos, per pipeline''', '''/// NS2-IA: SPIR-V with its leading OpCapability / OpExtension instructions sorted. Sirit keeps
/// them in unordered sets, so MSVC (official Fritchii) and libstdc++ (MinGW) builds list the same
/// capabilities in another order; the module is otherwise the same, and either order is valid.
std::vector<u32> CanonicalPreamble(const std::vector<u32>& words) {
    constexpr u32 OpExtension = 10;
    constexpr u32 OpCapability = 17;
    if (words.size() <= 5) {
        return words;
    }
    std::vector<std::vector<u32>> capabilities;
    std::vector<std::vector<u32>> extensions;
    size_t at = 5;
    while (at < words.size()) {
        const u32 count = words[at] >> 16;
        const u32 opcode = words[at] & 0xffff;
        if (count == 0 || at + count > words.size() ||
            (opcode != OpCapability && opcode != OpExtension)) {
            break;
        }
        std::vector<u32> instruction(words.begin() + at, words.begin() + at + count);
        (opcode == OpCapability ? capabilities : extensions).push_back(std::move(instruction));
        at += count;
    }
    std::ranges::sort(capabilities);
    std::ranges::sort(extensions);
    std::vector<u32> out(words.begin(), words.begin() + 5);
    out.reserve(words.size());
    for (const auto& group : {&capabilities, &extensions}) {
        for (const auto& instruction : *group) {
            out.insert(out.end(), instruction.begin(), instruction.end());
        }
    }
    out.insert(out.end(), words.begin() + at, words.end());
    return out;
}

/// What differs between a translation's stages (SPIR-V and serialized infos, per pipeline''')
sub(V, '''        if (kept_spirv != spirv[stage]) {
            const auto [mismatch, unused] = std::ranges::mismatch(kept_spirv, spirv[stage]);''', '''        if (kept_spirv != spirv[stage] &&
            CanonicalPreamble(kept_spirv) != CanonicalPreamble(spirv[stage])) {
            const auto [mismatch, unused] = std::ranges::mismatch(kept_spirv, spirv[stage]);''')
print("ok: NS2-IA reaplicado")
