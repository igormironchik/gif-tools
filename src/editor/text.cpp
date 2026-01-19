/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "text.hpp"

// Qt include.
#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>

//
// TextFrame
//

TextFrame::TextFrame(Frame *parent)
    : RectangleSelection(parent)
{
}

TextFrame::~TextFrame() noexcept
{
}

void TextFrame::contextMenuEvent(QContextMenuEvent *e)
{
    // QMenu menu(this);
    // auto cropAction = new QAction(QIcon(QStringLiteral(":/img/transform-crop.png")), tr("Crop"), this);
    // connect(cropAction, &QAction::triggered, this, &CropFrame::applyEdit);
    // menu.addAction(cropAction);

    // m_d->m_menu = true;

    // auto action = menu.exec(e->globalPos());

    // if (action) {
    //     QApplication::restoreOverrideCursor();
    // }

    e->accept();
}
