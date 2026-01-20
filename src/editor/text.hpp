/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_TEXT_HPP_INCLUDED
#define GIF_EDITOR_TEXT_HPP_INCLUDED

// GIF editor include.
#include "rectangle.hpp"

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class TextEdit;

//
// TextFrame
//

//! Text frame.
class TextFrame final : public RectangleSelection
{
    Q_OBJECT

signals:
    void switchToTextEditingMode();
    void switchToTextSelectionRectMode();

public:
    TextFrame(Frame *parent = nullptr);
    ~TextFrame() noexcept override;

    using Documents = QMap<qsizetype, QTextDocument*>;

    const Documents &text() const;

public slots:
    //! Switch to text typing mode.
    void startTextEditing();
    //! Bold text.
    void boldText();
    //! Italic text.
    void italicText();
    //! Font less.
    void fontLess();
    //! Font more.
    void fontMore();
    //! Text color.
    void textColor();
    //! Clear format.
    void clearFormat();
    //! Set font size.
    void setFontSize(int p);
    //! Clear.
    void clear();

private slots:
    void switchToSelectMode();
    void frameResized();
    void imagePosChanged(qsizetype idx);

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    Q_DISABLE_COPY(TextFrame)

    TextEdit *m_editor = nullptr;
    Documents m_map;
}; // class TextFrame

#endif // GIF_EDITOR_TEXT_HPP_INCLUDED
