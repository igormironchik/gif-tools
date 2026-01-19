/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "text.hpp"
#include "texteditor.hpp"

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

void TextFrame::startTextEditing()
{
    if (!m_editor) {
        m_editor = new TextEdit(static_cast<QWidget *>(parent()));
        connect(m_editor, &TextEdit::switchToSelectMode, this, &TextFrame::switchToSelectMode);
    }

    QEnterEvent e({}, {}, {});
    enterEvent(&e);
    enableMouse(false);
    m_editor->setGeometry(selectionRect().translated(availableRect().topLeft()));
    m_editor->show();
    m_editor->raise();
    m_editor->setFocus();
}

void TextFrame::switchToSelectMode()
{
    m_editor->hide();
    QApplication::processEvents();
    enableMouse(true);
}

void TextFrame::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    auto textAction = new QAction(QIcon(QStringLiteral(":/img/draw-text.png")), tr("Switch to text mode"), this);
    menu.addAction(textAction);

    m_d->m_menu = true;

    auto action = menu.exec(e->globalPos());

    if (action) {
        QApplication::restoreOverrideCursor();

        if (action == textAction) {
            startTextEditing();

            emit switchToTextEditingMode();
        }
    }

    e->accept();
}
