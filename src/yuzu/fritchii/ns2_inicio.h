// SPDX-License-Identifier: GPL-3.0-or-later
// NS2-IA: the start screen is the classic game list (columns, folder rows, "Add New Game
// Directory", filter bar) over the NS2-IA art, instead of Fritchii's home page.

#pragma once

class QWidget;

namespace Ns2 {

/// Paints the NS2-IA background and logo behind `game_list` and leaves room for the logo.
void SetUpInicio(QWidget* game_list);

} // namespace Ns2
