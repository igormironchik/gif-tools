/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "rectangle.hpp"
#include "frame.hpp"
#include "settings.hpp"

// Qt include.
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

//! Size of the handle to change geometry of selected region.
static const int c_handleSize = 15;

//
// RectangleSelectionPrivate
//

RectangleSelectionPrivate::RectangleSelectionPrivate(RectangleSelection *parent,
                                                     Frame *toObserve)
    : m_started(false)
    , m_nothing(true)
    , m_clicked(false)
    , m_hovered(false)
    , m_cursorOverriden(false)
    , m_handle(Handle::Unknown)
    , m_frame(toObserve)
    , m_q(parent)
{
}

bool RectangleSelectionPrivate::isHandleOutside() const
{
    return (qAbs(m_selected.width()) / 3 < c_handleSize + 1 || qAbs(m_selected.height()) / 3 < c_handleSize + 1);
}

QRect RectangleSelectionPrivate::topLeftHandleRect() const
{
    return (isHandleOutside() ? QRect(qRound(m_selected.x() - (m_selected.width() > 0 ? c_handleSize : 0)),
                                      qRound(m_selected.y() - (m_selected.height() > 0 ? c_handleSize : 0)),
                                      c_handleSize,
                                      c_handleSize)
                              : QRect(qRound(m_selected.x() - (m_selected.width() > 0 ? 0 : c_handleSize)),
                                      qRound(m_selected.y() - (m_selected.height() > 0 ? 0 : c_handleSize)),
                                      c_handleSize,
                                      c_handleSize));
}

QRect RectangleSelectionPrivate::topRightHandleRect() const
{
    return (isHandleOutside()
                ? QRect(qRound(m_selected.x() + m_selected.width() - 1 - (m_selected.width() > 0 ? 0 : c_handleSize)),
                        qRound(m_selected.y() - (m_selected.height() > 0 ? c_handleSize : 0)),
                        c_handleSize,
                        c_handleSize)
                : QRect(qRound(m_selected.x() + m_selected.width() - (m_selected.width() > 0 ? c_handleSize : 0) - 1),
                        qRound(m_selected.y() - (m_selected.height() > 0 ? 0 : c_handleSize)),
                        c_handleSize,
                        c_handleSize));
}

QRect RectangleSelectionPrivate::bottomRightHandleRect() const
{
    return (isHandleOutside()
                ? QRect(qRound(m_selected.x() + m_selected.width() - 1 - (m_selected.width() > 0 ? 0 : c_handleSize)),
                        qRound(m_selected.y() + m_selected.height() - (m_selected.height() > 0 ? 0 : c_handleSize)),
                        c_handleSize,
                        c_handleSize)
                : QRect(qRound(m_selected.x() + m_selected.width() - (m_selected.width() > 0 ? c_handleSize : 0) - 1),
                        qRound(m_selected.y() + m_selected.height() - (m_selected.height() > 0 ? c_handleSize : 0) - 1),
                        c_handleSize,
                        c_handleSize));
}

QRect RectangleSelectionPrivate::bottomLeftHandleRect() const
{
    return (isHandleOutside()
                ? QRect(qRound(m_selected.x() - (m_selected.width() > 0 ? c_handleSize : 0)),
                        qRound(m_selected.y() + m_selected.height() - 1 - (m_selected.height() > 0 ? 0 : c_handleSize)),
                        c_handleSize,
                        c_handleSize)
                : QRect(qRound(m_selected.x() - (m_selected.width() > 0 ? 0 : c_handleSize)),
                        qRound(m_selected.y() + m_selected.height() - (m_selected.height() > 0 ? c_handleSize : 0) - 1),
                        c_handleSize,
                        c_handleSize));
}

int RectangleSelectionPrivate::yHandleWidth() const
{
    const int w = qRound(m_selected.width());

    return (isHandleOutside() ? w : w - 2 * c_handleSize - (w - 2 * c_handleSize) / 3);
}

int RectangleSelectionPrivate::xHandleHeight() const
{
    const int h = qRound(m_selected.height());

    return (isHandleOutside() ? h : h - 2 * c_handleSize - (h - 2 * c_handleSize) / 3);
}

int RectangleSelectionPrivate::yHandleXPos() const
{
    return qRound(m_selected.x() + (qRound(m_selected.width()) - yHandleWidth()) / 2.0);
}

int RectangleSelectionPrivate::xHandleYPos() const
{
    return qRound(m_selected.y() + (qRound(m_selected.height()) - xHandleHeight()) / 2.0);
}

QRect RectangleSelectionPrivate::topHandleRect() const
{
    return (isHandleOutside() ? QRect(yHandleXPos(),
                                      qRound(m_selected.y() - (m_selected.height() > 0 ? c_handleSize : 0)),
                                      yHandleWidth(),
                                      c_handleSize)
                              : QRect(yHandleXPos(),
                                      qRound(m_selected.y() - (m_selected.height() > 0 ? 0 : c_handleSize)),
                                      yHandleWidth(),
                                      c_handleSize));
}

QRect RectangleSelectionPrivate::bottomHandleRect() const
{
    return (isHandleOutside()
                ? QRect(yHandleXPos(),
                        qRound(m_selected.y() + m_selected.height() - 1 - (m_selected.height() > 0 ? 0 : c_handleSize)),
                        yHandleWidth(),
                        c_handleSize)
                : QRect(yHandleXPos(),
                        qRound(m_selected.y() + m_selected.height() - 1 - (m_selected.height() > 0 ? c_handleSize : 0)),
                        yHandleWidth(),
                        c_handleSize));
}

QRect RectangleSelectionPrivate::leftHandleRect() const
{
    return (isHandleOutside() ? QRect(qRound(m_selected.x() - (m_selected.width() > 0 ? c_handleSize : 0)),
                                      xHandleYPos(),
                                      c_handleSize,
                                      xHandleHeight())
                              : QRect(qRound(m_selected.x() - (m_selected.width() > 0 ? 0 : c_handleSize)),
                                      xHandleYPos(),
                                      c_handleSize,
                                      xHandleHeight()));
}

QRect RectangleSelectionPrivate::rightHandleRect() const
{
    return (isHandleOutside()
                ? QRect(qRound(m_selected.x() + m_selected.width() - 1 - (m_selected.width() > 0 ? 0 : c_handleSize)),
                        xHandleYPos(),
                        c_handleSize,
                        xHandleHeight())
                : QRect(qRound(m_selected.x() + m_selected.width() - 1 - (m_selected.width() > 0 ? c_handleSize : 0)),
                        xHandleYPos(),
                        c_handleSize,
                        xHandleHeight()));
}

QPoint RectangleSelectionPrivate::boundToAvailable(const QPoint &p) const
{
    QPoint ret = p;

    if (p.x() < m_available.x()) {
        ret.setX(m_available.x());
    } else if (p.x() > m_available.x() + m_available.width()) {
        ret.setX(m_available.x() + m_available.width());
    }

    if (p.y() < m_available.y()) {
        ret.setY(m_available.y());
    } else if (p.y() > m_available.y() + m_available.height()) {
        ret.setY(m_available.y() + m_available.height());
    }

    return ret;
}

QPoint RectangleSelectionPrivate::boundLeftTopToAvailable(const QPoint &p) const
{
    QPoint ret = p;

    if (p.x() < m_available.x()) {
        ret.setX(m_available.x());
    } else if (p.x() > m_available.x() + m_available.width() - qRound(m_selected.width())) {
        ret.setX(m_available.x() + m_available.width() - qRound(m_selected.width()));
    }

    if (p.y() < m_available.y()) {
        ret.setY(m_available.y());
    } else if (p.y() > m_available.y() + m_available.height() - qRound(m_selected.height())) {
        ret.setY(m_available.y() + m_available.height() - qRound(m_selected.height()));
    }

    return ret;
}

void RectangleSelectionPrivate::checkAndOverrideCursor(Qt::CursorShape shape)
{
    if (QApplication::overrideCursor()) {
        if (*QApplication::overrideCursor() != QCursor(shape)) {
            if (m_cursorOverriden) {
                QApplication::restoreOverrideCursor();
            } else {
                m_cursorOverriden = true;
            }

            QApplication::setOverrideCursor(QCursor(shape));
        }
    } else {
        m_cursorOverriden = true;

        QApplication::setOverrideCursor(QCursor(shape));
    }
}

void RectangleSelectionPrivate::overrideCursor(const QPoint &pos)
{
    if (topLeftHandleRect().contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::TopLeft;
        checkAndOverrideCursor(Qt::SizeFDiagCursor);
    } else if (bottomRightHandleRect().contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::BottomRight;
        checkAndOverrideCursor(Qt::SizeFDiagCursor);
    } else if (topRightHandleRect().contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::TopRight;
        checkAndOverrideCursor(Qt::SizeBDiagCursor);
    } else if (bottomLeftHandleRect().contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::BottomLeft;
        checkAndOverrideCursor(Qt::SizeBDiagCursor);
    } else if (topHandleRect().contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::Top;
        checkAndOverrideCursor(Qt::SizeVerCursor);
    } else if (bottomHandleRect().contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::Bottom;
        checkAndOverrideCursor(Qt::SizeVerCursor);
    } else if (leftHandleRect().contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::Left;
        checkAndOverrideCursor(Qt::SizeHorCursor);
    } else if (rightHandleRect().contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::Right;
        checkAndOverrideCursor(Qt::SizeHorCursor);
    } else if (m_selected.contains(pos)) {
        m_handle = RectangleSelectionPrivate::Handle::Unknown;
        checkAndOverrideCursor(Qt::SizeAllCursor);
    } else if (m_cursorOverriden) {
        m_cursorOverriden = false;
        m_handle = RectangleSelectionPrivate::Handle::Unknown;
        QApplication::restoreOverrideCursor();
    }
}

void RectangleSelectionPrivate::resize(const QPoint &pos)
{
    switch (m_handle) {
    case RectangleSelectionPrivate::Handle::Unknown:
        m_selected.moveTo(boundLeftTopToAvailable(m_selected.topLeft().toPoint() - m_mousePos + pos));
        break;

    case RectangleSelectionPrivate::Handle::TopLeft:
        m_selected.setTopLeft(boundToAvailable(m_selected.topLeft().toPoint() - m_mousePos + pos));
        break;

    case RectangleSelectionPrivate::Handle::TopRight:
        m_selected.setTopRight(boundToAvailable(m_selected.topRight().toPoint() - m_mousePos + pos));
        break;

    case RectangleSelectionPrivate::Handle::BottomRight:
        m_selected.setBottomRight(boundToAvailable(m_selected.bottomRight().toPoint() - m_mousePos + pos));
        break;

    case RectangleSelectionPrivate::Handle::BottomLeft:
        m_selected.setBottomLeft(boundToAvailable(m_selected.bottomLeft().toPoint() - m_mousePos + pos));
        break;

    case RectangleSelectionPrivate::Handle::Top:
        m_selected.setTop(
            boundToAvailable(QPoint(qRound(m_selected.left()), qRound(m_selected.top())) - m_mousePos + pos).y());
        break;

    case RectangleSelectionPrivate::Handle::Bottom:
        m_selected.setBottom(
            boundToAvailable(QPoint(qRound(m_selected.left()), qRound(m_selected.bottom())) - m_mousePos + pos).y());
        break;

    case RectangleSelectionPrivate::Handle::Left:
        m_selected.setLeft(
            boundToAvailable(QPoint(qRound(m_selected.left()), qRound(m_selected.top())) - m_mousePos + pos).x());
        break;

    case RectangleSelectionPrivate::Handle::Right:
        m_selected.setRight(
            boundToAvailable(QPoint(qRound(m_selected.right()), qRound(m_selected.top())) - m_mousePos + pos).x());
        break;
    }

    m_mousePos = pos;
}

QRectF RectangleSelectionPrivate::selected(const QRect &full) const
{
    const auto oldR = m_available;

    const qreal xRatio = static_cast<qreal>(full.width()) / static_cast<qreal>(oldR.width());
    const qreal yRatio = static_cast<qreal>(full.height()) / static_cast<qreal>(oldR.height());

    QRectF r;

    if (!m_nothing) {
        const auto x = (m_selected.x() - oldR.x()) * xRatio + full.x();
        const auto y = (m_selected.y() - oldR.y()) * yRatio + full.y();
        const auto w = m_selected.width() * xRatio;
        const auto h = m_selected.height() * yRatio;

        r.setTopLeft(QPointF(x, y));
        r.setSize(QSizeF(w, h));
    }

    return r;
}

void RectangleSelectionPrivate::restoreOverridenCursor()
{
    if (m_cursorOverriden) {
        QApplication::restoreOverrideCursor();
    }

    if (m_hovered) {
        QApplication::restoreOverrideCursor();
    }

    m_cursorOverriden = false;
    m_hovered = false;
}

//
// RectangleSelection
//

RectangleSelection::RectangleSelection(Frame *parent)
    : QWidget(parent)
    , m_d(new RectangleSelectionPrivate(this,
                                        parent))
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

    m_d->m_available = parent->thumbnailRect();

    connect(m_d->m_frame, &Frame::resized, this, &RectangleSelection::frameResized);
}

RectangleSelection::~RectangleSelection() noexcept
{
    m_d->restoreOverridenCursor();
}

QRect RectangleSelection::selectionRectScaled() const
{
    return m_d->m_selected.toRect();
}

QRect RectangleSelection::availableRectScaled() const
{
    return m_d->m_available;
}

QRect RectangleSelection::selectionRect() const
{
    return m_d->selected(availableRect()).toRect();
}

QRect RectangleSelection::availableRect() const
{
    return m_d->m_frame->imageRect();
}

void RectangleSelection::start()
{
    m_d->m_started = true;
    m_d->m_nothing = true;

    update();
}

void RectangleSelection::stop()
{
    m_d->m_started = false;

    m_d->restoreOverridenCursor();

    update();
}

void RectangleSelection::enableMouse(bool on)
{
    m_d->m_mouseEnabled = on;

    if (!on) {
        m_d->restoreOverridenCursor();
    } else if (underMouse()) {
        QEnterEvent e({}, {}, {});
        enterEvent(&e);
    }

    update();
}

void RectangleSelection::setStartMessage(const QString &msg)
{
    m_d->m_msg = msg;
}

void RectangleSelection::frameResized()
{
    m_d->m_selected = m_d->selected(m_d->m_frame->thumbnailRect());

    setGeometry(QRect(0, 0, m_d->m_frame->width(), m_d->m_frame->height()));

    m_d->m_available = m_d->m_frame->thumbnailRect();

    update();
}

void RectangleSelection::paintEvent(QPaintEvent *)
{
    static const QColor dark(0, 0, 0, 100);

    QPainter p(this);
    p.setPen(Qt::black);
    p.setBrush(dark);

    if (m_d->m_started && !m_d->m_nothing) {
        QPainterPath path;
        path.addRect(QRectF(m_d->m_available).adjusted(0, 0, -1, -1));

        if (m_d->m_available != m_d->m_selected.toRect()) {
            QPainterPath spath;
            spath.addRect(m_d->m_selected.adjusted(0, 0, -1, -1));
            path = path.subtracted(spath);
        } else {
            p.setBrush(Qt::transparent);
        }

        p.drawPath(path);
    } else if (Settings::instance().showHelpMsg() && !m_d->m_msg.isEmpty()) {
        p.drawRect(rect());
        QTextOption opt;
        opt.setAlignment(Qt::AlignCenter);
        opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        auto f = p.font();
        f.setPointSize(16);
        f.setBold(true);
        p.setFont(f);
        p.drawText(rect().toRectF(), m_d->m_msg, opt);
    }

    if (m_d->m_mouseEnabled) {
        p.setBrush(Qt::lightGray);

        if (m_d->m_started
            && !m_d->m_clicked
            && !m_d->m_nothing
            && m_d->m_handle == RectangleSelectionPrivate::Handle::Unknown) {
            p.drawRect(m_d->topLeftHandleRect());
            p.drawRect(m_d->topRightHandleRect());
            p.drawRect(m_d->bottomRightHandleRect());
            p.drawRect(m_d->bottomLeftHandleRect());
        } else if (m_d->m_started && !m_d->m_nothing && m_d->m_handle != RectangleSelectionPrivate::Handle::Unknown) {
            switch (m_d->m_handle) {
            case RectangleSelectionPrivate::Handle::TopLeft:
                p.drawRect(m_d->topLeftHandleRect());
                break;

            case RectangleSelectionPrivate::Handle::TopRight:
                p.drawRect(m_d->topRightHandleRect());
                break;

            case RectangleSelectionPrivate::Handle::BottomRight:
                p.drawRect(m_d->bottomRightHandleRect());
                break;

            case RectangleSelectionPrivate::Handle::BottomLeft:
                p.drawRect(m_d->bottomLeftHandleRect());
                break;

            case RectangleSelectionPrivate::Handle::Top:
                p.drawRect(m_d->topHandleRect().adjusted(0, 0, -1, 0));
                break;

            case RectangleSelectionPrivate::Handle::Bottom:
                p.drawRect(m_d->bottomHandleRect().adjusted(0, 0, -1, 0));
                break;

            case RectangleSelectionPrivate::Handle::Left:
                p.drawRect(m_d->leftHandleRect().adjusted(0, 0, 0, -1));
                break;

            case RectangleSelectionPrivate::Handle::Right:
                p.drawRect(m_d->rightHandleRect().adjusted(0, 0, 0, -1));
                break;

            default:
                break;
            }
        }
    }
}

void RectangleSelection::mousePressEvent(QMouseEvent *e)
{
    if (m_d->m_mouseEnabled) {
        if (e->button() == Qt::LeftButton) {
            m_d->m_clicked = true;

            if (!m_d->m_cursorOverriden) {
                m_d->m_selected.setTopLeft(m_d->boundToAvailable(e->pos()));
            } else {
                m_d->m_mousePos = e->pos();
            }

            update();

            e->accept();
        } else {
            e->ignore();
        }
    } else {
        e->ignore();
    }
}

void RectangleSelection::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->m_mouseEnabled) {
        if (m_d->m_clicked) {
            if (!m_d->m_cursorOverriden) {
                m_d->m_selected.setBottomRight(m_d->boundToAvailable(e->pos()));

                m_d->m_nothing = false;
            } else {
                m_d->resize(e->pos());
            }

            update();

            e->accept();
        } else if (!m_d->m_hovered) {
            m_d->m_hovered = true;

            QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
        } else if (m_d->m_hovered && !m_d->m_nothing) {
            m_d->overrideCursor(e->pos());

            update();
        } else {
            e->ignore();
        }
    } else {
        e->ignore();
    }
}

void RectangleSelection::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_d->m_mouseEnabled) {
        m_d->m_clicked = false;

        if (e->button() == Qt::LeftButton) {
            m_d->m_selected = m_d->m_selected.normalized();

            emit started();

            update();

            e->accept();
        } else {
            e->ignore();
        }
    } else {
        e->ignore();
    }
}

void RectangleSelection::enterEvent(QEnterEvent *e)
{
    if (m_d->m_mouseEnabled) {
        if (m_d->m_started) {
            m_d->m_hovered = true;

            if (!m_d->m_menu) {
                QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
            } else {
                m_d->m_menu = false;
                QApplication::changeOverrideCursor(m_d->m_cursor);
            }

            e->accept();
        } else {
            e->ignore();
        }
    } else {
        e->ignore();
    }
}

void RectangleSelection::leaveEvent(QEvent *e)
{
    if (m_d->m_mouseEnabled) {
        if (m_d->m_started) {
            m_d->m_hovered = false;

            if (!m_d->m_menu) {
                QApplication::restoreOverrideCursor();
            } else {
                if (QApplication::overrideCursor()) {
                    m_d->m_cursor = QApplication::overrideCursor()->shape();
                    QApplication::changeOverrideCursor(Qt::ArrowCursor);
                }
            }

            e->accept();
        } else {
            e->ignore();
        }
    } else {
        e->ignore();
    }
}

void RectangleSelection::contextMenuEvent(QContextMenuEvent *e)
{
    e->ignore();
}
