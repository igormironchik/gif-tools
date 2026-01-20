/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "frame.hpp"
#include "text.hpp"
#include "texteditor.hpp"

// Qt include.
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QColorDialog>
#include <QContextMenuEvent>
#include <QMenu>

//
// TextFrame
//

TextFrame::TextFrame(Frame *parent)
    : RectangleSelection(parent)
{
    connect(parent, &Frame::resized, this, &TextFrame::frameResized);
}

TextFrame::~TextFrame() noexcept
{
}

void TextFrame::frameResized()
{
    m_editor->setGeometry(selectionRect());
}

void TextFrame::startTextEditing()
{
    if (!m_editor) {
        m_editor = new TextEdit(static_cast<QWidget *>(parent()));
        setFontSize(12);
        connect(m_editor, &TextEdit::switchToSelectMode, this, &TextFrame::switchToSelectMode);
    }

    if (m_d->m_menu) {
        QEnterEvent e({}, {}, {});
        enterEvent(&e);
    }

    enableMouse(false);
    frameResized();
    m_editor->show();
    m_editor->raise();
    m_editor->setFocus();

    emit switchToTextEditingMode();
}

void TextFrame::clear()
{
    m_editor->hide();
    m_editor->deleteLater();
    m_editor = nullptr;
}

void TextFrame::switchToSelectMode()
{
    m_editor->hide();
    QApplication::processEvents();
    enableMouse(true);

    emit switchToTextSelectionRectMode();
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
        }
    }

    e->accept();
}

void TextFrame::boldText()
{
    QTextCursor c = m_editor->textCursor();

    if (c.hasSelection()) {
        if (c.position() != c.selectionEnd()) {
            c.setPosition(c.selectionEnd());
        }

        QTextCharFormat fmt = c.charFormat();

        fmt.setFontWeight(QFont::Bold);

        m_editor->textCursor().setCharFormat(fmt);
    } else {
        m_editor->setFontWeight(QFont::Bold);
    }
}

void TextFrame::italicText()
{
    QTextCursor c = m_editor->textCursor();

    if (c.hasSelection()) {
        if (c.position() != c.selectionEnd()) {
            c.setPosition(c.selectionEnd());
        }

        QTextCharFormat fmt = c.charFormat();

        fmt.setFontItalic(true);

        m_editor->textCursor().setCharFormat(fmt);
    } else {
        m_editor->setFontItalic(true);
    }
}

void TextFrame::fontLess()
{
    QTextCursor c = m_editor->textCursor();

    if (c.hasSelection()) {
        QTextCharFormat fmt = c.charFormat();
        setFontSize(fmt.font().pointSize() - 1);
    } else {
        setFontSize(m_editor->currentFont().pointSize() - 1);
    }
}

void TextFrame::fontMore()
{
    QTextCursor c = m_editor->textCursor();

    if (c.hasSelection()) {
        QTextCharFormat fmt = c.charFormat();
        setFontSize(fmt.font().pointSize() + 1);
    } else {
        setFontSize(m_editor->currentFont().pointSize() + 1);
    }
}

void TextFrame::textColor()
{
    QColorDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        QTextCursor c = m_editor->textCursor();

        if (c.hasSelection()) {
            if (c.position() != c.selectionEnd()) {
                c.setPosition(c.selectionEnd());
            }

            QTextCharFormat fmt = c.charFormat();

            fmt.setForeground(dlg.currentColor());

            m_editor->textCursor().setCharFormat(fmt);
        } else {
            m_editor->setTextColor(dlg.currentColor());
            m_editor->setFontItalic(true);
        }
    }
}

void TextFrame::clearFormat()
{
    QTextCursor c = m_editor->textCursor();

    if (c.hasSelection()) {
        QTextCharFormat fmt = c.charFormat();

        fmt.setFontUnderline(false);
        fmt.setFontItalic(false);
        fmt.setFontWeight(QFont::Normal);
        fmt.clearBackground();
        fmt.clearForeground();
        QFont f = fmt.font();
        f.setPointSize(font().pointSize());
        fmt.setFont(f);
        fmt.setForeground(QBrush(QColor(Qt::black)));

        m_editor->textCursor().setCharFormat(fmt);
    } else {
        m_editor->setFontUnderline(false);
        m_editor->setFontItalic(false);
        m_editor->setFontWeight(QFont::Normal);
        m_editor->setTextColor(Qt::black);
        QFont f = m_editor->currentFont();
        f.setPointSize(font().pointSize());
        m_editor->setCurrentFont(f);
        QTextCursor cursor = m_editor->textCursor();
        QTextCharFormat fmt = cursor.charFormat();
        fmt.setForeground(QBrush(QColor(Qt::black)));
        cursor.setCharFormat(fmt);
        m_editor->setTextCursor(cursor);
    }
}

void TextFrame::setFontSize(int p)
{
    QTextCursor c = m_editor->textCursor();

    if (c.hasSelection()) {
        if (c.position() != c.selectionEnd()) {
            c.setPosition(c.selectionEnd());
        }

        QTextCharFormat fmt = c.charFormat();

        QFont f = fmt.font();
        f.setPointSize(p);
        fmt.setFont(f);

        m_editor->textCursor().setCharFormat(fmt);
    } else {
        QFont f = m_editor->currentFont();
        f.setPointSize(p);
        m_editor->setCurrentFont(f);
    }
}
