// SPDX-License-Identifier: GPL-3.0-or-later
// NS2-IA: "Notas do Zelda" - mapa com os checks do jogo, marcados automaticamente pelo save.

#pragma once

#include <QDateTime>
#include <QHash>
#include <QSet>
#include <QString>
#include <QVector>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QGraphicsPixmapItem;
class QGraphicsScene;
class QLabel;
class QLineEdit;
class QListWidget;
class QProgressBar;
class QPushButton;
class QTabWidget;
class QTimer;
class QVBoxLayout;
class NotasMapView;

namespace NS2Notas {

struct Area {
    QString id, nome;
    bool masmorra = false;
    QHash<QString, QString> mapas; // "crianca"/"adulto" -> caminho relativo
};

struct Categoria {
    QString id, nome, grupo, cor, letra;
    bool visivel = true;
};

struct Check {
    QString id, cat, nome, area, area_nome, era;
    double x = -1, y = -1;
    bool tem_pos = false;
    quint64 chave = 0;
};

struct ItemLista {
    QString lista, nome;
    quint32 flag = 0;
};

struct Save {
    QString caminho;
    QDateTime quando;
    QHash<quint32, quint32> bools;
    QSet<quint64> chaves;
    bool ok = false;
};

/// Le o progress.sav (tabela hash/valor com marcadores de secao; secao 32 = lista de chaves u64).
Save LerSave(const QString& caminho);

} // namespace NS2Notas

class NotasZelda : public QWidget {
    Q_OBJECT
public:
    explicit NotasZelda(QWidget* parent = nullptr);

    /// Mostra a janela para o jogo dado (title id). Sem dados para o jogo, explica como gerar.
    void Abrir(quint64 title_id);

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    bool CarregarDados(const QString& dir);
    void MontarUi();
    void Atualizar();       // le o save mais recente se mudou
    void Redesenhar();      // mapa + listas
    void TrocarArea(int idx);
    QString EraAtual() const;
    bool Pego(const NS2Notas::Check& c) const;
    QString AcharSave() const;

    QString dir_dados;
    QString titulo_hex;
    QVector<NS2Notas::Area> areas;
    QVector<NS2Notas::Categoria> categorias;
    QVector<NS2Notas::Check> checks;
    QVector<NS2Notas::ItemLista> listas;
    QHash<QString, bool> cat_visivel;
    NS2Notas::Save save;
    QString save_forcado;

    QLabel* lbl_save = nullptr;
    QLabel* lbl_total = nullptr;
    QComboBox* cmb_area = nullptr;
    QComboBox* cmb_slot = nullptr;
    QPushButton* btn_crianca = nullptr;
    QPushButton* btn_adulto = nullptr;
    QLineEdit* busca = nullptr;
    QCheckBox* chk_esconder_pegos = nullptr;
    QVBoxLayout* lay_cats = nullptr;
    QHash<QString, QCheckBox*> chk_cat;
    QHash<QString, QLabel*> lbl_cat;
    QHash<QString, QProgressBar*> barra_cat;
    QListWidget* lista_area = nullptr;
    QListWidget* lista_cancoes = nullptr;
    QListWidget* lista_itens = nullptr;
    QGraphicsScene* cena = nullptr;
    QGraphicsPixmapItem* fundo = nullptr;
    NotasMapView* vista = nullptr;
    QTimer* timer = nullptr;
    int area_atual = -1;
};
