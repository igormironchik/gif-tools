/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_TEXT_HPP_INCLUDED
#define GIF_EDITOR_TEXT_HPP_INCLUDED

// GIF editor include.
#include "rectangle.hpp"

class TextEdit;

//
// TextFrame
//

//! Text frame.
class TextFrame final : public RectangleSelection
{
    Q_OBJECT

public:
    TextFrame(Frame *parent = nullptr);
    ~TextFrame() noexcept override;

public slots:
    //! Switch to text typing mode.
    void startTextEditing();

private slots:
    void switchToSelectMode();

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    Q_DISABLE_COPY(TextFrame)

    TextEdit *m_editor = nullptr;
}; // class TextFrame

#endif // GIF_EDITOR_TEXT_HPP_INCLUDED
