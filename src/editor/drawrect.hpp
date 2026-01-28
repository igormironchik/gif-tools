/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_DRAWRECT_HPP_INCLUDED
#define GIF_EDITOR_DRAWRECT_HPP_INCLUDED

// GIF editor include.
#include "rectangle.hpp"

class Tape;

//
// RectFrame
//

//! Rect frame.
class RectFrame final : public RectangleSelection
{
    Q_OBJECT

signals:
    void applyEdit();

public:
    explicit RectFrame(Tape *tape,
                       Frame *parent = nullptr);
    ~RectFrame() noexcept override;

    static void drawRect(QPainter &p,
                         const QRect &r);

    const QSet<qsizetype> &frames() const;

private slots:
    void onStarted();

private slots:
    void imagePosChanged(qsizetype idx);

protected:
    void paintEvent(QPaintEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    Q_DISABLE_COPY(RectFrame)

    QSet<qsizetype> m_frames;
    Tape *m_tape = nullptr;
}; // class RectFrame

#endif // GIF_EDITOR_DRAWRECT_HPP_INCLUDED
