// SPDX-License-Identifier: GPL-3.0-or-later

#include "yuzu/fritchii/ns2_inicio.h"

#include <algorithm>

#include <QAbstractItemView>
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

} // namespace

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
