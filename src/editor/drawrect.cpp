/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "drawrect.hpp"
#include "frame.hpp"
#include "settings.hpp"

// Qt include.
#include <QColorDialog>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>

//
// RectFrame
//

RectFrame::RectFrame(Frame *parent)
    : RectangleSelection(parent)
{
    m_frames.insert(parent->image().m_pos);

    connect(parent, &Frame::imagePosChanged, this, &RectFrame::imagePosChanged);
}

RectFrame::~RectFrame() noexcept
{
}

void RectFrame::penColor()
{
    QColorDialog dlg(Settings::instance().penColor(), this);

    if (dlg.exec() == QDialog::Accepted) {
        Settings::instance().setPenColor(dlg.currentColor());

        update();
    }
}

void RectFrame::brushColor()
{
    QColorDialog dlg(Settings::instance().brushColor(), this);
    dlg.setOption(QColorDialog::ShowAlphaChannel, true);

    if (dlg.exec() == QDialog::Accepted) {
        Settings::instance().setBrushColor(dlg.currentColor());

        update();
    }
}

void RectFrame::imagePosChanged(qsizetype idx)
{
    m_frames.insert(idx);
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
