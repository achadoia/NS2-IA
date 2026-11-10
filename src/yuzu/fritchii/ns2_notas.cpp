// SPDX-License-Identifier: GPL-3.0-or-later
// NS2-IA: "Notas do Zelda" - mapa estilo Zelda Notes / mapa interativo.
// Dados: <pasta do exe>/notas/<TITLE ID>/notas.json + mapas/*.jpg, gerados do RomFS do proprio
// usuario por D:\OoT-Notas\tools\gerar_notas.py (nada do jogo vem embutido no emulador).
// Progresso: o progress.sav mais recente do jogo no NAND, relido sozinho quando o jogo salva.

#include "yuzu/fritchii/ns2_notas.h"

#include <algorithm>
#include <cstring>

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QStyleOptionGraphicsItem>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "common/fs/path_util.h"

using namespace NS2Notas;

namespace {

constexpr char kAmarelo[] = "#ffd60a";
constexpr char kFundo[] = "#161615";
constexpr char kPainel[] = "#1e1e1c";

QString U(const char* s) {
    return QString::fromUtf8(s);
}

quint32 Rd32(const QByteArray& d, qsizetype o) {
    quint32 v;
    std::memcpy(&v, d.constData() + o, 4);
    return v;
}

quint64 Rd64(const QByteArray& d, qsizetype o) {
    quint64 v;
    std::memcpy(&v, d.constData() + o, 8);
    return v;
}

} // namespace

namespace NS2Notas {

Save LerSave(const QString& caminho) {
    Save s;
    s.caminho = caminho;
    QFile f(caminho);
    if (!f.open(QIODevice::ReadOnly)) {
        return s;
    }
    const QByteArray d = f.readAll();
    s.quando = QFileInfo(caminho).lastModified();
    if (d.size() < 0x30 || Rd32(d, 0) != 0x01020304u) {
        return s;
    }
    const qsizetype fim = std::min<qsizetype>(Rd32(d, 8), d.size());
    int secao = 0;
    for (qsizetype o = 0x28; o + 8 <= fim; o += 8) {
        const quint32 h = Rd32(d, o);
        const quint32 v = Rd32(d, o + 4);
        if (h == 0) {
            secao = static_cast<int>(v);
            continue;
        }
        if (secao == 0) {
            s.bools.insert(h, v);
        } else if (secao == 16 && v + 64 <= static_cast<quint32>(d.size())) {
            // String64: valor = offset de 64 bytes terminados em 0
            const char* txt = d.constData() + v;
            s.textos.insert(h, QString::fromUtf8(txt, static_cast<qsizetype>(qstrnlen(txt, 64))));
        } else if (secao == 10 && v + 12 <= static_cast<quint32>(d.size())) {
            float f[3];
            std::memcpy(f, d.constData() + v, 12);
            s.vetores.insert(h, {f[0], f[1], f[2]});
        } else if (secao == 32) {
            // lista de chaves de 64 bits ligadas, ate um 0 ou o fim do arquivo
            for (qsizetype p = v; p + 8 <= d.size(); p += 8) {
                const quint64 k = Rd64(d, p);
                if (k == 0) {
                    break;
                }
                s.chaves.insert(k);
            }
        }
    }
    s.ok = true;
    return s;
}

} // namespace NS2Notas

// ---------------------------------------------------------------------------------------------
// Marcador no mapa

namespace {

class Marcador : public QGraphicsItem {
public:
    Marcador(const Check& c, const Categoria& cat, bool pego)
        : check(c), cor(cat.cor), letra(cat.letra), pego(pego) {
        setPos(c.x, c.y);
        setToolTip(QStringLiteral("<b>%1</b><br>%2<br>%3")
                       .arg(c.nome.toHtmlEscaped(), cat.nome.toHtmlEscaped(),
                            !cat.rastreavel ? U("(o jogo não guarda no save)")
                            : pego          ? U("✔ Já pego")
                                            : U("○ Falta pegar")));
        setZValue(pego ? 1 : 2);
        setFlag(ItemIgnoresTransformations);
        setAcceptHoverEvents(true);
    }
    QRectF boundingRect() const override {
        return {-13, -13, 26, 26};
    }
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override {
        p->setRenderHint(QPainter::Antialiasing);
        const QColor base(cor);
        const qreal r = hover ? 12 : 10;
        p->setOpacity(pego ? 0.45 : 1.0);
        p->setPen(QPen(QColor(kFundo), 2.5));
        p->setBrush(pego ? QColor(70, 70, 66) : base);
        p->drawEllipse(QPointF(0, 0), r, r);
        QFont f = p->font();
        f.setBold(true);
        f.setPixelSize(11);
        p->setFont(f);
        p->setPen(pego ? base : QColor(kFundo));
        p->drawText(QRectF(-r, -r, 2 * r, 2 * r), Qt::AlignCenter, pego ? U("✔") : letra);
    }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override {
        hover = true;
        update();
    }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override {
        hover = false;
        update();
    }

private:
    Check check;
    QString cor, letra;
    bool pego = false;
    bool hover = false;
};

class MarcadorLink : public QGraphicsItem {
public:
    MarcadorLink(double x, double y) {
        setPos(x, y);
        setZValue(10);
        setFlag(ItemIgnoresTransformations);
        setToolTip(U("<b>Você está aqui</b><br>posição do último save/autosave"));
    }
    QRectF boundingRect() const override {
        return {-20, -20, 40, 40};
    }
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override {
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(QPen(QColor(kFundo), 3));
        p->setBrush(QColor(255, 255, 255, 70));
        p->drawEllipse(QPointF(0, 0), 18, 18);
        p->setBrush(QColor(0x3d, 0xd6, 0x5c));
        p->drawEllipse(QPointF(0, 0), 12, 12);
        QFont f = p->font();
        f.setBold(true);
        f.setPixelSize(13);
        p->setFont(f);
        p->setPen(QColor(kFundo));
        p->drawText(QRectF(-12, -12, 24, 24), Qt::AlignCenter, QStringLiteral("L"));
    }
};

} // namespace

class NotasMapView : public QGraphicsView {
public:
    explicit NotasMapView(QGraphicsScene* s, QWidget* parent) : QGraphicsView(s, parent) {
        setDragMode(ScrollHandDrag);
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        setTransformationAnchor(AnchorUnderMouse);
        setBackgroundBrush(QColor(23, 20, 28));
        setFrameShape(QFrame::NoFrame);
    }
    void Encaixar() {
        if (scene() && !scene()->sceneRect().isEmpty()) {
            fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
        }
    }

protected:
    void wheelEvent(QWheelEvent* e) override {
        const qreal f = e->angleDelta().y() > 0 ? 1.2 : 1 / 1.2;
        const qreal z = transform().m11() * f;
        if (z > 0.2 && z < 8) {
            scale(f, f);
        }
    }
    void resizeEvent(QResizeEvent* e) override {
        QGraphicsView::resizeEvent(e);
        if (!ajustado) {
            Encaixar();
        }
    }
    void mousePressEvent(QMouseEvent* e) override {
        ajustado = true;
        QGraphicsView::mousePressEvent(e);
    }

public:
    bool ajustado = false;
};

// ---------------------------------------------------------------------------------------------

NotasZelda::NotasZelda(QWidget* parent) : QWidget(parent, Qt::Window) {
    setWindowTitle(U("Notas do Zelda — NS2-IA"));
    resize(1500, 860);
    setStyleSheet(QStringLiteral(
                      "NotasZelda{background:%1;}"
                      "QFrame#Lado{background:%2;border-right:1px solid #2e2e2b;}"
                      "QFrame#Dir{background:%2;border-left:1px solid #2e2e2b;}"
                      "QLabel#Titulo{color:%3;font-size:20px;font-weight:800;letter-spacing:1px;}"
                      "QLabel#Grupo{color:#8a8880;font-size:11px;font-weight:700;margin-top:8px;}"
                      "QLabel#Total{color:%3;font-size:28px;font-weight:800;}"
                      "QPushButton#Era{padding:6px 10px;border:2px solid #3a3a36;background:#232321;"
                      "color:#ecebe6;font-weight:700;}"
                      "QPushButton#Era:checked{background:%3;color:%1;border-color:%3;}"
                      "QProgressBar{border:1px solid #3a3a36;background:#232321;height:10px;"
                      "text-align:right;color:#ecebe6;font-size:10px;}"
                      "QProgressBar::chunk{background:%3;}"
                      "QListWidget{background:%1;border:1px solid #2e2e2b;}"
                      "QListWidget::item{padding:3px 4px;}")
                      .arg(QString::fromLatin1(kFundo), QString::fromLatin1(kPainel),
                           QString::fromLatin1(kAmarelo)));
    MontarUi();
    timer = new QTimer(this);
    timer->setInterval(1500);
    connect(timer, &QTimer::timeout, this, &NotasZelda::Atualizar);
}

void NotasZelda::MontarUi() {
    auto* raiz = new QHBoxLayout(this);
    raiz->setContentsMargins(0, 0, 0, 0);
    raiz->setSpacing(0);

    // ---- lado esquerdo: filtros
    auto* lado = new QFrame(this);
    lado->setObjectName(QStringLiteral("Lado"));
    lado->setFixedWidth(300);
    auto* l = new QVBoxLayout(lado);
    l->setContentsMargins(16, 16, 16, 16);
    auto* titulo = new QLabel(U("NOTAS DO ZELDA"), lado);
    titulo->setObjectName(QStringLiteral("Titulo"));
    l->addWidget(titulo);
    lbl_save = new QLabel(U("Procurando o save..."), lado);
    lbl_save->setWordWrap(true);
    lbl_save->setStyleSheet(QStringLiteral("color:#8a8880;font-size:11px;"));
    l->addWidget(lbl_save);

    cmb_slot = new QComboBox(lado);
    cmb_slot->setToolTip(U("Qual save acompanhar. \"Automático\" segue o último que o jogo gravou."));
    l->addWidget(cmb_slot);
    connect(cmb_slot, &QComboBox::activated, this, [this](int i) {
        save_forcado = cmb_slot->itemData(i).toString();
        save = {};
        Atualizar();
    });

    chk_seguir = new QCheckBox(U("📍 Seguir o Link (área e idade atuais)"), lado);
    chk_seguir->setChecked(true);
    chk_seguir->setToolTip(U("Abre na área e na idade em que o Link está, pelo último save/autosave.\n"
                             "Trocar a área ou a idade à mão desliga; marque de novo para voltar."));
    l->addWidget(chk_seguir);
    connect(chk_seguir, &QCheckBox::toggled, this, [this](bool on) {
        if (on) {
            SeguirLink();
        }
    });

    auto* g1 = new QLabel(U("ÁREA"), lado);
    g1->setObjectName(QStringLiteral("Grupo"));
    l->addWidget(g1);
    cmb_area = new QComboBox(lado);
    cmb_area->setMaxVisibleItems(32);
    l->addWidget(cmb_area);
    connect(cmb_area, &QComboBox::currentIndexChanged, this, &NotasZelda::TrocarArea);
    connect(cmb_area, &QComboBox::activated, this, [this](int) { chk_seguir->setChecked(false); });

    auto* eras = new QHBoxLayout;
    btn_crianca = new QPushButton(U("Criança (passado)"), lado);
    btn_adulto = new QPushButton(U("Adulto (futuro)"), lado);
    for (auto* b : {btn_crianca, btn_adulto}) {
        b->setObjectName(QStringLiteral("Era"));
        b->setCheckable(true);
        b->setAutoExclusive(true);
        eras->addWidget(b);
        connect(b, &QPushButton::toggled, this, [this](bool on) {
            if (on) {
                TrocarArea(cmb_area->currentIndex());
            }
        });
        connect(b, &QPushButton::clicked, this, [this] { chk_seguir->setChecked(false); });
    }
    btn_crianca->setChecked(true);
    l->addLayout(eras);

    busca = new QLineEdit(lado);
    busca->setPlaceholderText(U("Buscar (ex.: chave, coração, gancho)"));
    busca->setClearButtonEnabled(true);
    l->addWidget(busca);
    connect(busca, &QLineEdit::textChanged, this, &NotasZelda::Redesenhar);

    auto* mostrar = new QHBoxLayout;
    auto* b_todos = new QPushButton(U("Mostrar tudo"), lado);
    auto* b_nenhum = new QPushButton(U("Esconder tudo"), lado);
    mostrar->addWidget(b_todos);
    mostrar->addWidget(b_nenhum);
    l->addLayout(mostrar);
    auto marcar_todos = [this](bool on) {
        for (auto* c : chk_cat) {
            c->blockSignals(true);
            c->setChecked(on);
            c->blockSignals(false);
            cat_visivel[chk_cat.key(c)] = on;
        }
        Redesenhar();
    };
    connect(b_todos, &QPushButton::clicked, this, [marcar_todos] { marcar_todos(true); });
    connect(b_nenhum, &QPushButton::clicked, this, [marcar_todos] { marcar_todos(false); });

    chk_esconder_pegos = new QCheckBox(U("Esconder o que já peguei"), lado);
    l->addWidget(chk_esconder_pegos);
    connect(chk_esconder_pegos, &QCheckBox::toggled, this, &NotasZelda::Redesenhar);

    auto* scroll = new QScrollArea(lado);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* cats_w = new QWidget(scroll);
    lay_cats = new QVBoxLayout(cats_w);
    lay_cats->setContentsMargins(0, 0, 0, 0);
    lay_cats->addStretch();
    scroll->setWidget(cats_w);
    l->addWidget(scroll, 1);

    // ---- centro: mapa
    cena = new QGraphicsScene(this);
    vista = new NotasMapView(cena, this);

    // ---- direita: progresso + listas
    auto* dir = new QFrame(this);
    dir->setObjectName(QStringLiteral("Dir"));
    dir->setFixedWidth(330);
    auto* r = new QVBoxLayout(dir);
    r->setContentsMargins(16, 16, 16, 16);
    auto* gp = new QLabel(U("PROGRESSO GERAL"), dir);
    gp->setObjectName(QStringLiteral("Grupo"));
    r->addWidget(gp);
    lbl_total = new QLabel(QStringLiteral("—"), dir);
    lbl_total->setObjectName(QStringLiteral("Total"));
    r->addWidget(lbl_total);
    auto* barras = new QWidget(dir);
    auto* lb = new QVBoxLayout(barras);
    lb->setContentsMargins(0, 0, 0, 0);
    lb->setSpacing(2);
    barras->setObjectName(QStringLiteral("Barras"));
    r->addWidget(barras);

    auto* abas = new QTabWidget(dir);
    lista_area = new QListWidget(abas);
    lista_cancoes = new QListWidget(abas);
    lista_itens = new QListWidget(abas);
    abas->addTab(lista_area, U("Nesta área"));
    abas->addTab(lista_cancoes, U("Canções"));
    abas->addTab(lista_itens, U("Itens"));
    r->addWidget(abas, 1);
    connect(lista_area, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* it) {
        const QPointF p = it->data(Qt::UserRole).toPointF();
        if (p.x() >= 0) {
            vista->ajustado = true;
            vista->centerOn(p);
        }
    });

    raiz->addWidget(lado);
    raiz->addWidget(vista, 1);
    raiz->addWidget(dir);
}

QString NotasZelda::AcharSave() const {
    if (!save_forcado.isEmpty()) {
        return save_forcado;
    }
    const auto nand = QString::fromStdString(
        Common::FS::PathToUTF8String(Common::FS::GetEdenPath(Common::FS::EdenPath::NANDDir)));
    const QString base = nand + QStringLiteral("/user/save/0000000000000000");
    QString melhor;
    QDateTime quando;
    QDir users(base);
    for (const auto& u : users.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDirIterator it(base + QLatin1Char('/') + u + QLatin1Char('/') + titulo_hex,
                        {QStringLiteral("progress.sav")}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString p = it.next();
            const QDateTime t = QFileInfo(p).lastModified();
            if (melhor.isEmpty() || t > quando) {
                melhor = p;
                quando = t;
            }
        }
    }
    return melhor;
}

void NotasZelda::Abrir(quint64 title_id) {
    const QString novo_titulo = QStringLiteral("%1").arg(title_id, 16, 16, QLatin1Char('0')).toUpper();
    if (isVisible() && novo_titulo == titulo_hex) {
        // ja aberta: F7 de novo fecha (alterna), sem recarregar nada
        close();
        return;
    }
    titulo_hex = novo_titulo;
    const QString dir = QApplication::applicationDirPath() + QStringLiteral("/notas/") + titulo_hex;
    if (dir != dir_dados) {
        if (!CarregarDados(dir)) {
            lbl_save->setText(
                U("Não há notas para este jogo (%1).\nGere os dados com gerar_notas.py e coloque em:\n%2")
                    .arg(titulo_hex, QDir::toNativeSeparators(dir)));
        }
    }
    // lista de slots
    cmb_slot->clear();
    cmb_slot->addItem(U("Automático (último save)"), QString());
    const auto nand = QString::fromStdString(
        Common::FS::PathToUTF8String(Common::FS::GetEdenPath(Common::FS::EdenPath::NANDDir)));
    QDirIterator it(nand + QStringLiteral("/user/save/0000000000000000"), {QStringLiteral("progress.sav")},
                    QDir::Files, QDirIterator::Subdirectories);
    QStringList saves_slot;
    while (it.hasNext()) {
        const QString p = it.next();
        if (p.contains(titulo_hex, Qt::CaseInsensitive)) {
            saves_slot << p;
        }
    }
    saves_slot.sort();
    for (const auto& p : saves_slot) {
        cmb_slot->addItem(QFileInfo(p).dir().dirName(), p);
    }
    setAttribute(Qt::WA_ShowWithoutActivating, false);
    show();
    raise();
    save = {};
    chk_seguir->blockSignals(true);
    chk_seguir->setChecked(true);
    chk_seguir->blockSignals(false);
    Atualizar();
    SeguirLink();
    timer->start();
}

void NotasZelda::closeEvent(QCloseEvent* e) {
    timer->stop();
    e->accept();
}

bool NotasZelda::CarregarDados(const QString& dir) {
    QFile f(dir + QStringLiteral("/notas.json"));
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
    if (o.isEmpty()) {
        return false;
    }
    dir_dados = dir;
    areas.clear();
    categorias.clear();
    checks.clear();
    listas.clear();
    for (const auto& v : o.value(QStringLiteral("areas")).toArray()) {
        const auto a = v.toObject();
        Area ar;
        ar.id = a.value(QStringLiteral("id")).toString();
        ar.nome = a.value(QStringLiteral("nome")).toString();
        ar.masmorra = a.value(QStringLiteral("masmorra")).toBool();
        ar.cx = a.value(QStringLiteral("cx")).toDouble();
        ar.cz = a.value(QStringLiteral("cz")).toDouble();
        ar.altura = a.value(QStringLiteral("altura")).toDouble(1);
        const auto m = a.value(QStringLiteral("mapas")).toObject();
        for (auto k = m.begin(); k != m.end(); ++k) {
            ar.mapas.insert(k.key(), k.value().toString());
        }
        areas.push_back(ar);
    }
    mapa_w = o.value(QStringLiteral("mapa_w")).toDouble(1000);
    mapa_h = o.value(QStringLiteral("mapa_h")).toDouble(720);
    bancs.clear();
    const auto bo = o.value(QStringLiteral("bancs")).toObject();
    for (auto k = bo.begin(); k != bo.end(); ++k) {
        const auto par = k.value().toArray();
        bancs.insert(k.key(), {par.at(0).toString(), par.at(1).toString()});
    }
    const auto est = o.value(QStringLiteral("estado")).toObject();
    h_banc = static_cast<quint32>(est.value(QStringLiteral("Sequence_CurrentBanc")).toDouble());
    h_infancia = static_cast<quint32>(est.value(QStringLiteral("Sequence_IsChildhood")).toDouble());
    h_pos = static_cast<quint32>(est.value(QStringLiteral("Player_Pos_For_Map")).toDouble());
    for (const auto& v : o.value(QStringLiteral("categorias")).toArray()) {
        const auto c = v.toObject();
        Categoria cat{c.value(QStringLiteral("id")).toString(), c.value(QStringLiteral("nome")).toString(),
                      c.value(QStringLiteral("grupo")).toString(), c.value(QStringLiteral("cor")).toString(),
                      c.value(QStringLiteral("letra")).toString(), c.value(QStringLiteral("visivel")).toBool(),
                      c.value(QStringLiteral("rastreavel")).toBool(true)};
        categorias.push_back(cat);
        cat_visivel[cat.id] = cat.visivel;
    }
    for (const auto& v : o.value(QStringLiteral("checks")).toArray()) {
        const auto c = v.toObject();
        Check ch;
        ch.id = c.value(QStringLiteral("id")).toString();
        ch.cat = c.value(QStringLiteral("cat")).toString();
        ch.nome = c.value(QStringLiteral("nome")).toString();
        ch.area = c.value(QStringLiteral("area")).toString();
        ch.area_nome = c.value(QStringLiteral("area_nome")).toString();
        ch.era = c.value(QStringLiteral("era")).toString();
        ch.tem_pos = c.value(QStringLiteral("x")).isDouble();
        ch.x = c.value(QStringLiteral("x")).toDouble(-1);
        ch.y = c.value(QStringLiteral("y")).toDouble(-1);
        ch.chave = c.value(QStringLiteral("chave")).toString().toULongLong(nullptr, 16);
        ch.flag = static_cast<quint32>(c.value(QStringLiteral("flag")).toDouble(0));
        checks.push_back(ch);
    }
    for (const auto& v : o.value(QStringLiteral("listas")).toArray()) {
        const auto c = v.toObject();
        listas.push_back({c.value(QStringLiteral("lista")).toString(), c.value(QStringLiteral("nome")).toString(),
                          static_cast<quint32>(c.value(QStringLiteral("flag")).toDouble())});
    }

    // categorias no painel, por grupo
    while (lay_cats->count() > 1) {
        auto* it = lay_cats->takeAt(0);
        delete it->widget();
        delete it;
    }
    chk_cat.clear();
    lbl_cat.clear();
    QString grupo;
    int pos = 0;
    for (const auto& c : categorias) {
        if (c.grupo != grupo) {
            grupo = c.grupo;
            auto* g = new QLabel(grupo.toUpper());
            g->setObjectName(QStringLiteral("Grupo"));
            lay_cats->insertWidget(pos++, g);
        }
        auto* linha = new QWidget;
        auto* h = new QHBoxLayout(linha);
        h->setContentsMargins(0, 0, 0, 0);
        auto* chk = new QCheckBox(c.nome);
        chk->setChecked(c.visivel);
        chk->setStyleSheet(QStringLiteral("QCheckBox::indicator:checked{background:%1;border:2px solid %1;}")
                               .arg(c.cor));
        auto* n = new QLabel(QStringLiteral("0/0"));
        n->setStyleSheet(QStringLiteral("color:#8a8880;"));
        h->addWidget(chk, 1);
        h->addWidget(n);
        lay_cats->insertWidget(pos++, linha);
        chk_cat.insert(c.id, chk);
        lbl_cat.insert(c.id, n);
        connect(chk, &QCheckBox::toggled, this, [this, id = c.id](bool on) {
            cat_visivel[id] = on;
            Redesenhar();
        });
    }

    // barras de progresso (categorias que valem como "colecao")
    auto* barras = findChild<QWidget*>(QStringLiteral("Barras"));
    for (auto* b : barra_cat) {
        delete b->parentWidget();
    }
    barra_cat.clear();
    for (const auto& c : categorias) {
        if (!c.rastreavel) {
            continue;
        }
        auto* w = new QWidget(barras);
        auto* v = new QVBoxLayout(w);
        v->setContentsMargins(0, 4, 0, 0);
        v->setSpacing(1);
        v->addWidget(new QLabel(c.nome, w));
        auto* pb = new QProgressBar(w);
        pb->setFormat(QStringLiteral("%v/%m"));
        v->addWidget(pb);
        barras->layout()->addWidget(w);
        barra_cat.insert(c.id, pb);
    }

    cmb_area->blockSignals(true);
    cmb_area->clear();
    for (const auto& a : areas) {
        cmb_area->addItem((a.masmorra ? U("⚔ ") : U("🌿 ")) + a.nome);
    }
    cmb_area->blockSignals(false);
    TrocarArea(0);
    return true;
}

QString NotasZelda::EraAtual() const {
    return btn_adulto->isChecked() ? QStringLiteral("adulto") : QStringLiteral("crianca");
}

bool NotasZelda::Pego(const Check& c) const {
    if (!save.ok) {
        return false;
    }
    return c.flag ? save.bools.value(c.flag) != 0 : save.chaves.contains(c.chave);
}

void NotasZelda::TrocarArea(int idx) {
    if (idx < 0 || idx >= areas.size()) {
        return;
    }
    const bool mudou = idx != area_atual;
    area_atual = idx;
    const auto& a = areas[idx];
    QString rel = a.mapas.value(EraAtual());
    if (rel.isEmpty()) {
        rel = a.mapas.value(QStringLiteral("crianca"), a.mapas.value(QStringLiteral("adulto")));
    }
    cena->clear();
    fundo = cena->addPixmap(QPixmap(dir_dados + QLatin1Char('/') + rel));
    fundo->setZValue(0);
    cena->setSceneRect(fundo->boundingRect());
    if (mudou) {
        vista->ajustado = false;
        vista->resetTransform();
    }
    vista->Encaixar();
    Redesenhar();
}

void NotasZelda::Atualizar() {
    const QString p = AcharSave();
    if (p.isEmpty()) {
        lbl_save->setText(U("Nenhum save deste jogo encontrado ainda. Jogue e salve uma vez."));
        return;
    }
    const QDateTime t = QFileInfo(p).lastModified();
    if (save.ok && p == save.caminho && t == save.quando) {
        return;
    }
    Save novo = LerSave(p);
    if (!novo.ok) {
        return; // o jogo pode estar no meio da gravacao; tenta de novo no proximo tique
    }
    save = std::move(novo);
    lbl_save->setText(U("Acompanhando %1 · atualizado às %2")
                          .arg(QFileInfo(p).dir().dirName(), t.toString(QStringLiteral("HH:mm:ss"))));
    if (chk_seguir->isChecked()) {
        SeguirLink();
    } else {
        Redesenhar();
    }
}

bool NotasZelda::OndeEstaLink(int& area, QString& era, double& px, double& py) const {
    if (!save.ok || h_banc == 0) {
        return false;
    }
    const QString banc = save.textos.value(h_banc);
    if (banc.isEmpty()) {
        return false;
    }
    QPair<QString, QString> alvo = bancs.value(banc);
    if (alvo.first.isEmpty()) {
        // cena sem mapa proprio (interior, variante): tenta pelo nome da area antes do primeiro "_"
        const QString base = banc.section(QLatin1Char('_'), 0, 0);
        for (auto it = bancs.begin(); it != bancs.end(); ++it) {
            if (it.value().first == base) {
                alvo = {base, QString()};
                break;
            }
        }
    }
    if (alvo.first.isEmpty()) {
        return false;
    }
    area = -1;
    for (int i = 0; i < areas.size(); ++i) {
        if (areas[i].id == alvo.first) {
            area = i;
            break;
        }
    }
    if (area < 0) {
        return false;
    }
    era = alvo.second;
    if (era.isEmpty()) {
        if (banc.contains(QStringLiteral("_Adult"))) {
            era = QStringLiteral("adulto");
        } else if (banc.contains(QStringLiteral("_Child"))) {
            era = QStringLiteral("crianca");
        } else {
            era = save.bools.value(h_infancia, 1) ? QStringLiteral("crianca") : QStringLiteral("adulto");
        }
    }
    px = py = -1;
    const auto pos = save.vetores.value(h_pos);
    const auto& a = areas[area];
    if (pos.size() == 3 && a.altura > 0 && (pos[0] != 0.0f || pos[2] != 0.0f)) {
        const double k = mapa_h / a.altura;
        px = mapa_w / 2 + (pos[0] - a.cx) * k;
        py = mapa_h / 2 + (pos[2] - a.cz) * k;
    }
    return true;
}

void NotasZelda::SeguirLink() {
    int area;
    QString era;
    double px, py;
    if (!OndeEstaLink(area, era, px, py)) {
        Redesenhar();
        return;
    }
    for (auto* b : {btn_crianca, btn_adulto}) {
        b->blockSignals(true);
    }
    (era == QStringLiteral("adulto") ? btn_adulto : btn_crianca)->setChecked(true);
    for (auto* b : {btn_crianca, btn_adulto}) {
        b->blockSignals(false);
    }
    cmb_area->blockSignals(true);
    cmb_area->setCurrentIndex(area);
    cmb_area->blockSignals(false);
    area_atual = -1; // forca recarregar o mapa (a idade pode ter mudado)
    TrocarArea(area);
    if (px >= 0) {
        vista->centerOn(px, py);
    }
}

void NotasZelda::Redesenhar() {
    if (area_atual < 0 || area_atual >= areas.size()) {
        return;
    }
    // limpa marcadores (mantem o fundo)
    for (auto* it : cena->items()) {
        if (it != fundo) {
            cena->removeItem(it);
            delete it;
        }
    }
    QHash<QString, const Categoria*> cat_por_id;
    for (const auto& c : categorias) {
        cat_por_id.insert(c.id, &c);
    }
    const auto& area = areas[area_atual];
    const QString era = EraAtual();
    const QString filtro = busca->text().trimmed();
    const bool esconder = chk_esconder_pegos->isChecked();

    QHash<QString, int> tot, got, tot_area, got_area;
    QHash<QString, QSet<quint64>> vistos;
    int total = 0, pegos = 0;
    lista_area->clear();
    for (const auto& c : checks) {
        const auto* cat0 = cat_por_id.value(c.cat);
        const bool rastreavel = cat0 && cat0->rastreavel;
        const bool p = rastreavel && Pego(c);
        // progresso geral: conta cada chave uma vez (so o que o jogo guarda no save)
        const quint64 id_unico = c.flag ? (0xF000000000000000ULL | c.flag) : c.chave;
        if (rastreavel && !vistos[c.cat].contains(id_unico)) {
            vistos[c.cat].insert(id_unico);
            tot[c.cat]++;
            total++;
            if (p) {
                got[c.cat]++;
                pegos++;
            }
        }
        if (c.area != area.id || (c.era != era && c.era != QStringLiteral("ambos"))) {
            continue;
        }
        if (rastreavel) {
            tot_area[c.cat]++;
            if (p) {
                got_area[c.cat]++;
            }
        }
        const auto* cat = cat_por_id.value(c.cat);
        if (!cat || !cat_visivel.value(c.cat) || (esconder && p)) {
            continue;
        }
        if (!filtro.isEmpty() && !c.nome.contains(filtro, Qt::CaseInsensitive) &&
            !cat->nome.contains(filtro, Qt::CaseInsensitive)) {
            continue;
        }
        if (c.tem_pos) {
            cena->addItem(new Marcador(c, *cat, p));
        }
        auto* it = new QListWidgetItem((!rastreavel ? U("·  ") : p ? U("✔  ") : U("○  ")) + c.nome, lista_area);
        it->setForeground(p ? QColor(0x8a, 0x88, 0x80) : QColor(cat->cor));
        it->setData(Qt::UserRole, c.tem_pos ? QPointF(c.x, c.y) : QPointF(-1, -1));
        it->setToolTip(c.tem_pos ? U("Clique duas vezes para achar no mapa") : U("Sem posição no mapa"));
    }
    {
        int la;
        QString le;
        double lx, ly;
        if (OndeEstaLink(la, le, lx, ly) && la == area_atual && le == era && lx >= 0) {
            cena->addItem(new MarcadorLink(lx, ly));
        }
    }
    for (auto it = lbl_cat.begin(); it != lbl_cat.end(); ++it) {
        const auto* cat = cat_por_id.value(it.key());
        it.value()->setText(cat && !cat->rastreavel
                                ? U("só mapa")
                                : QStringLiteral("%1/%2").arg(got_area.value(it.key())).arg(tot_area.value(it.key())));
    }
    for (auto it = barra_cat.begin(); it != barra_cat.end(); ++it) {
        it.value()->setMaximum(std::max(1, tot.value(it.key())));
        it.value()->setValue(got.value(it.key()));
    }
    lbl_total->setText(total ? QStringLiteral("%1%").arg(100.0 * pegos / total, 0, 'f', 1) : QStringLiteral("—"));

    lista_cancoes->clear();
    lista_itens->clear();
    for (const auto& li : listas) {
        const bool p = save.ok && save.bools.value(li.flag) != 0;
        auto* dest = li.lista == U("Canções") ? lista_cancoes : lista_itens;
        auto* it = new QListWidgetItem((p ? U("✔  ") : U("○  ")) + li.nome, dest);
        it->setForeground(p ? QColor(QString::fromLatin1(kAmarelo)) : QColor(0x8a, 0x88, 0x80));
    }
}
