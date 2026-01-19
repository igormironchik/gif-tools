/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_TEXT_EDITOR_HPP_INCLUDED
#define GIF_EDITOR_TEXT_EDITOR_HPP_INCLUDED

// Qt include.
#include <QScopedPointer>
#include <QTextEdit>

class TextEdit : public QTextEdit
{
    Q_OBJECT

signals:
    //! Switch to region selection mode.
    void switchToSelectMode();

public:
    TextEdit(QWidget *parent = nullptr);
    ~TextEdit() override;

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    QColor m_background;
}; // class TextEdit

#endif // GIF_EDITOR_TEXT_EDITOR_HPP_INCLUDED
