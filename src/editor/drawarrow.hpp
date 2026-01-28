/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_DRAWARROW_HPP_INCLUDED
#define GIF_EDITOR_DRAWARROW_HPP_INCLUDED

// GIF editor include.
#include "rectangle.hpp"

class Tape;

//
// ArrowFrame
//

//! Arrow frame.
class ArrowFrame final : public RectangleSelection
{
    Q_OBJECT

signals:
    void applyEdit();

public:
    explicit ArrowFrame(Tape *tape,
                        Frame *parent = nullptr);
    ~ArrowFrame() noexcept override;

    //! Arrow's orientation.
    enum Orientation {
        TopLeftToBottomRight,
        TopRightToBottomLeft,
        BottomRightToTopLeft,
        BottomLeftToTopRight
    }; // enum Orientation

    static void drawArrow(QPainter &p,
                          const QRect &r,
                          Orientation o);

    const QSet<qsizetype> &frames() const;

    //! \return Orientation.
    Orientation orientation() const;

private slots:
    void onStarted();
    void imagePosChanged(qsizetype idx);

private:
    static void drawArrowHead(QPainter &p,
                              const QLine &line);

protected:
    void paintEvent(QPaintEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    Q_DISABLE_COPY(ArrowFrame)

    QSet<qsizetype> m_frames;
    Tape *m_tape = nullptr;
    Orientation m_orientation = TopLeftToBottomRight;
    bool m_orientationDefined = false;
    QRect m_prevRect;
}; // class ArrowFrame

#endif // GIF_EDITOR_DRAWARROW_HPP_INCLUDED
