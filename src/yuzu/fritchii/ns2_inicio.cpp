// SPDX-License-Identifier: GPL-3.0-or-later

#include "yuzu/fritchii/ns2_inicio.h"

#include <algorithm>

#include <QAbstractItemView>
#include <QApplication>
#include <QEvent>
#include <QLayout>
#include <QPainter>
#include <QPixmap>
#include <QWidget>

namespace Ns2 {
namespace {

/// Draws the art (scaled to cover, anchored bottom-right where the controller is) and the logo
/// (top-left, unscaled) before the game list paints its rows over them.
class Backdrop final : public QObject {
public:
    explicit Backdrop(QWidget* target)
        : QObject(target), art(QStringLiteral(":/fritchii/ns2/fundo.jpg")),
          logo(QStringLiteral(":/fritchii/ns2/logo.png")) {}

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() != QEvent::Paint) {
            return false;
        }
        auto* widget = static_cast<QWidget*>(watched);
        QPainter painter(widget);
        const QSize area = widget->size();
        if (!art.isNull()) {
            const QPixmap scaled = art.scaled(area, Qt::KeepAspectRatioByExpanding,
                                              Qt::SmoothTransformation);
            painter.drawPixmap(area.width() - scaled.width(), area.height() - scaled.height(),
                               scaled);
        } else {
            painter.fillRect(widget->rect(), QColor(0x14, 0x14, 0x13));
        }
        if (!logo.isNull()) {
            painter.drawPixmap(12, 6, logo);
        }
        return false; // the widget still paints itself (nothing) and its children paint after
    }

private:
    QPixmap art;
    QPixmap logo;
};

/// Settings dialogs: the art (darkened) behind the pages and the vertical logo under the sidebar.
class DialogArt final : public QObject {
public:
    explicit DialogArt(QObject* parent)
        : QObject(parent), art(QStringLiteral(":/fritchii/ns2/fundo.jpg")),
          logo(QStringLiteral(":/fritchii/ns2/logo_lateral.png")) {}

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() != QEvent::Paint || !watched->isWidgetType()) {
            return false;
        }
        const QString name = watched->objectName();
        if (name != QStringLiteral("ConfigureDialog") &&
            name != QStringLiteral("ConfigurePerGame")) {
            return false;
        }
        auto* dialog = static_cast<QWidget*>(watched);
        QPainter painter(dialog);
        const QSize area = dialog->size();
        if (!art.isNull()) {
            const QPixmap scaled = art.scaled(area, Qt::KeepAspectRatioByExpanding,
                                              Qt::SmoothTransformation);
            painter.drawPixmap(area.width() - scaled.width(), area.height() - scaled.height(),
                               scaled);
            painter.fillRect(dialog->rect(), QColor(0, 0, 0, 110));
        }
        if (!logo.isNull()) {
            QRect slot(12, area.height() - logo.height() - 70, logo.width(), logo.height());
            if (auto* list = dialog->findChild<QWidget*>(QStringLiteral("selectorList"))) {
                const QRect g(list->mapTo(dialog, QPoint(0, 0)), list->size());
                if (g.width() >= 120) {
                    const int w = std::min(logo.width(), g.width() - 12);
                    const QPixmap fit = logo.scaledToWidth(w, Qt::SmoothTransformation);
                    slot = QRect(g.x() + (g.width() - fit.width()) / 2,
                                 g.bottom() - fit.height() - 8, fit.width(), fit.height());
                    painter.drawPixmap(slot.topLeft(), fit);
                    return false;
                }
            }
            painter.drawPixmap(slot.topLeft(), logo);
        }
        return false;
    }

private:
    QPixmap art;
    QPixmap logo;
};

} // namespace

void SetUpDialogArt() {
    static bool once = false;
    if (once || !qApp) {
        return;
    }
    once = true;
    qApp->installEventFilter(new DialogArt(qApp));
}

void SetUpInicio(QWidget* game_list) {
    if (!game_list) {
        return;
    }
    game_list->setObjectName(QStringLiteral("Ns2Inicio"));
    game_list->installEventFilter(new Backdrop(game_list));
    if (QLayout* layout = game_list->layout()) {
        // Room for the logo strip at the top (the art draws a yellow line under it).
        layout->setContentsMargins(18, 96, 18, 10);
        layout->setSpacing(10);
    }
    // The rows draw over the art: no opaque viewport behind them.
    for (auto* view : game_list->findChildren<QAbstractItemView*>()) {
        view->viewport()->setAutoFillBackground(false);
        view->setAutoFillBackground(false);
    }
}

} // namespace Ns2
