/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "drawrect.hpp"
#include "frame.hpp"
#include "frameontape.hpp"
#include "settings.hpp"
#include "tape.hpp"

// Qt include.
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>

//
// RectFrame
//

RectFrame::RectFrame(Tape *tape,
                     Frame *parent)
    : RectangleSelection(parent)
    , m_tape(tape)
{
    m_frames.insert(parent->image().m_pos);

    connect(parent, &Frame::imagePosChanged, this, &RectFrame::imagePosChanged);
    connect(this, &RectangleSelection::started, this, &RectFrame::onStarted);
}

RectFrame::~RectFrame() noexcept
{
}

void RectFrame::onStarted()
{
    m_tape->frame(m_d->m_frame->image().m_pos + 1)->setModified(true);
}

void RectFrame::imagePosChanged(qsizetype idx)
{
    m_frames.insert(idx);
    m_tape->frame(idx + 1)->setModified(true);
}

void RectFrame::drawRect(QPainter &p,
                         const QRect &r)
{
    p.setPen(QPen(Settings::instance().penColor(), Settings::instance().penWidth()));
    p.setBrush(Settings::instance().brushColor());

    const auto delta = Settings::instance().penWidth() / 2 + Settings::instance().penWidth() % 2;
    const auto rect = r.adjusted(delta, delta, -delta, -delta);

    p.drawRect(rect);
}

const QSet<qsizetype> &RectFrame::frames() const
{
    return m_frames;
}

void RectFrame::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    if (m_d->m_started && !m_d->m_nothing) {
        drawRect(p, selectionRectScaled());
    }

    RectangleSelection::paintEvent(e);
}

void RectFrame::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    auto applyAction = new QAction(QIcon(QStringLiteral(":/img/dialog-ok-apply.png")), tr("Apply"), this);
    connect(applyAction, &QAction::triggered, this, &RectFrame::applyEdit);
    menu.addAction(applyAction);

    m_d->m_menu = true;

    auto action = menu.exec(e->globalPos());

    if (action) {
        QApplication::restoreOverrideCursor();
    }

    e->accept();
}
