/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_DRAWRECT_HPP_INCLUDED
#define GIF_EDITOR_DRAWRECT_HPP_INCLUDED

// GIF editor include.
#include "rectangle.hpp"

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
    RectFrame(Frame *parent = nullptr);
    ~RectFrame() noexcept override;

    static void drawRect(QPainter &p,
                         const QRect &r);

    const QSet<qsizetype> &frames() const;

public slots:
    //! Pen color.
    void penColor();
    //! Brush color.
    void brushColor();

private slots:
    void imagePosChanged(qsizetype idx);

protected:
    void paintEvent(QPaintEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    Q_DISABLE_COPY(RectFrame)

    QSet<qsizetype> m_frames;
}; // class RectFrame

#endif // GIF_EDITOR_DRAWRECT_HPP_INCLUDED
