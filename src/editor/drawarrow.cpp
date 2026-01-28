/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "drawarrow.hpp"
#include "frame.hpp"
#include "frameontape.hpp"
#include "settings.hpp"
#include "tape.hpp"

// Qt include.
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>

// C++ include.
#define _USE_MATH_DEFINES
#include <cmath>

//
// ArrowFrame
//

ArrowFrame::ArrowFrame(Tape *tape,
                       Frame *parent)
    : RectangleSelection(parent)
    , m_tape(tape)
{
    m_frames.insert(parent->image().m_pos);

    connect(parent, &Frame::imagePosChanged, this, &ArrowFrame::imagePosChanged);
    connect(this, &RectangleSelection::started, this, &ArrowFrame::onStarted);
}

ArrowFrame::~ArrowFrame() noexcept
{
}

ArrowFrame::Orientation ArrowFrame::orientation() const
{
    return m_orientation;
}

bool equalPoints(const QPoint &p1,
                 const QPoint &p2)
{
    return qAbs(p1.x() - p2.x()) <= 1 && qAbs(p1.y() - p2.y()) <= 1;
}

void ArrowFrame::onStarted()
{
    m_tape->frame(m_d->m_frame->image().m_pos + 1)->setModified(true);

    if (!m_orientationDefined) {
        m_orientationDefined = true;

        m_prevRect = selectionRectScaled();

        if (equalPoints(m_prevRect.topLeft(), releasePoint())) {
            m_orientation = BottomRightToTopLeft;
        } else if (equalPoints(m_prevRect.topRight(), releasePoint())) {
            m_orientation = BottomLeftToTopRight;
        } else if (equalPoints(m_prevRect.bottomLeft(), releasePoint())) {
            m_orientation = TopRightToBottomLeft;
        } else {
            m_orientation = TopLeftToBottomRight;
        }
    } else {
        const auto r = selectionRectScaled();

        auto top = r.top() == m_prevRect.top();
        auto bottom = r.bottom() == m_prevRect.bottom();
        auto left = r.left() == m_prevRect.left();
        auto right = r.right() == m_prevRect.right();

        auto count = top + bottom + left + right;

        if (count <= 1 || (count == 2 && ((top && bottom) || (right && left)))) {
            top = qAbs(m_prevRect.top() - r.bottom()) <= 1;
            bottom = qAbs(m_prevRect.bottom() - r.top()) <= 1;
            left = qAbs(m_prevRect.left() - r.right()) <= 1;
            right = qAbs(m_prevRect.right() - r.left()) <= 1;

            count = top + bottom + left + right;

            if (count == 2) {
                switch (m_orientation) {
                case BottomLeftToTopRight: {
                    m_orientation = TopRightToBottomLeft;
                } break;

                case BottomRightToTopLeft: {
                    m_orientation = TopLeftToBottomRight;
                } break;

                case TopLeftToBottomRight: {
                    m_orientation = BottomRightToTopLeft;
                } break;

                case TopRightToBottomLeft: {
                    m_orientation = BottomLeftToTopRight;
                } break;

                default: {
                    break;
                }
                }
            } else if (count == 1) {
                if (top || bottom) {
                    switch (m_orientation) {
                    case BottomLeftToTopRight: {
                        m_orientation = TopLeftToBottomRight;
                    } break;

                    case BottomRightToTopLeft: {
                        m_orientation = TopRightToBottomLeft;
                    } break;

                    case TopLeftToBottomRight: {
                        m_orientation = BottomLeftToTopRight;
                    } break;

                    case TopRightToBottomLeft: {
                        m_orientation = BottomRightToTopLeft;
                    } break;

                    default: {
                        break;
                    }
                    }
                } else if (left || right) {
                    switch (m_orientation) {
                    case BottomLeftToTopRight: {
                        m_orientation = BottomRightToTopLeft;
                    } break;

                    case BottomRightToTopLeft: {
                        m_orientation = BottomLeftToTopRight;
                    } break;

                    case TopLeftToBottomRight: {
                        m_orientation = TopRightToBottomLeft;
                    } break;

                    case TopRightToBottomLeft: {
                        m_orientation = TopLeftToBottomRight;
                    } break;

                    default: {
                        break;
                    }
                    }
                }
            }
        }

        m_prevRect = r;
    }

    update();
}

void ArrowFrame::imagePosChanged(qsizetype idx)
{
    m_frames.insert(idx);
    m_tape->frame(idx + 1)->setModified(true);
}

void ArrowFrame::drawArrowHead(QPainter &p,
                               const QLine &line)
{
    const auto angle = std::atan2(-line.dy(), line.dx());
    const auto arrowSize = std::max(15, std::max(line.dy(), line.dx()) / 15);

    const auto arrowP1 = line.p2()
        - QPoint(qRound(std::sin(angle + M_PI / 3) * arrowSize), qRound(std::cos(angle + M_PI / 3) * arrowSize));
    const auto arrowP2 = line.p2()
        - QPoint(qRound(std::sin(angle + M_PI - M_PI / 3) * arrowSize),
                 qRound(std::cos(angle + M_PI - M_PI / 3) * arrowSize));

    QPolygon arrowHead;
    arrowHead << line.p2() << arrowP1 << arrowP2;

    p.drawPolygon(arrowHead);
}

void ArrowFrame::drawArrow(QPainter &p,
                           const QRect &r,
                           Orientation o)
{
    QPen pen(Settings::instance().penColor(), Settings::instance().penWidth());
    pen.setJoinStyle(Qt::MiterJoin);
    p.setPen(pen);
    p.setBrush(Settings::instance().penColor());
    p.setRenderHint(QPainter::Antialiasing, true);

    const auto delta = Settings::instance().penWidth() / 2 + Settings::instance().penWidth() % 2;
    const auto rect = r.adjusted(delta, delta, -delta, -delta);

    QLine line;

    switch (o) {
    case TopLeftToBottomRight: {
        line = QLine(rect.topLeft(), rect.bottomRight());
    } break;

    case TopRightToBottomLeft: {
        line = QLine(rect.topRight(), rect.bottomLeft());
    } break;

    case BottomLeftToTopRight: {
        line = QLine(rect.bottomLeft(), rect.topRight());
    } break;

    case BottomRightToTopLeft: {
        line = QLine(rect.bottomRight(), rect.topLeft());
    } break;

    default: {
        break;
    }
    }

    p.drawLine(line);
    drawArrowHead(p, line);
}

const QSet<qsizetype> &ArrowFrame::frames() const
{
    return m_frames;
}

void ArrowFrame::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    if (m_d->m_started && !m_d->m_nothing) {
        drawArrow(p, selectionRectScaled(), orientation());
    }

    RectangleSelection::paintEvent(e);
}

void ArrowFrame::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    auto applyAction = new QAction(QIcon(QStringLiteral(":/img/dialog-ok-apply.png")), tr("Apply"), this);
    connect(applyAction, &QAction::triggered, this, &ArrowFrame::applyEdit);
    menu.addAction(applyAction);

    m_d->m_menu = true;

    auto action = menu.exec(e->globalPos());

    if (action) {
        QApplication::restoreOverrideCursor();
    }

    e->accept();
}
