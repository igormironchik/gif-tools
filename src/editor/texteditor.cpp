/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "texteditor.hpp"

// Qt include.
#include <QContextMenuEvent>
#include <QMenu>

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setStyleSheet(QStringLiteral("background-color: transparent; border: none;"));
}

TextEdit::~TextEdit()
{
}

void TextEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu;
    auto textAction =
        new QAction(QIcon(QStringLiteral(":/img/insert-text.png")), tr("Switch to region selection mode"), this);
    connect(textAction, &QAction::triggered, this, &TextEdit::switchToSelectMode);
    menu.addAction(textAction);

    menu.exec(e->globalPos());

    e->accept();
}
