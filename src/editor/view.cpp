/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "view.hpp"
#include "crop.hpp"
#include "drawarrow.hpp"
#include "drawrect.hpp"
#include "frame.hpp"
#include "frameontape.hpp"
#include "tape.hpp"
#include "text.hpp"

// Qt include.
#include <QApplication>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

//
// ScrollArea
//

class ScrollArea : public QScrollArea
{
public:
    ScrollArea(QWidget *parent)
        : QScrollArea(parent)
    {
    }

    ~ScrollArea() override = default;

    void setZeroContentMargins()
    {
        QScrollArea::setContentsMargins(0, 0, 0, 0);
    }

    void scrollContentsTo(int x)
    {
        horizontalScrollBar()->setValue(x);
    }
};

//
// ViewPrivate
//

class ViewPrivate
{
public:
    ViewPrivate(const QGifLib::Gif &data,
                View *parent)
        : m_tape(nullptr)
        , m_currentFrame(new Frame({data,
                                    0,
                                    true},
                                   Frame::ResizeMode::FitToSize,
                                   parent))
        , m_crop(nullptr)
        , m_text(nullptr)
        , m_rect(nullptr)
        , m_arrow(nullptr)
        , m_scroll(nullptr)
        , m_q(parent)
    {
    }

    //! Tape.
    Tape *m_tape;
    //! Current frame.
    Frame *m_currentFrame;
    //! Crop.
    CropFrame *m_crop;
    //! Text.
    TextFrame *m_text;
    //! Rect.
    RectFrame *m_rect;
    //! Arrow.
    ArrowFrame *m_arrow;
    //! Scroll area for tape.
    ScrollArea *m_scroll;
    //! Parent.
    View *m_q;
}; // class ViewPrivate

//
// View
//

View::View(const QGifLib::Gif &data,
           QWidget *parent)
    : QWidget(parent)
    , m_d(new ViewPrivate(data,
                          this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_d->m_currentFrame);

    m_d->m_scroll = new ScrollArea(this);
    m_d->m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_d->m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_d->m_scroll->setMinimumHeight(150);
    m_d->m_scroll->setMaximumHeight(150);
    m_d->m_scroll->setWidgetResizable(true);
    m_d->m_scroll->setZeroContentMargins();

    m_d->m_tape = new Tape(m_d->m_scroll);
    m_d->m_scroll->setWidget(m_d->m_tape);

    layout->addWidget(m_d->m_scroll);
    m_d->m_scroll->setFixedHeight(150);

    connect(m_d->m_tape, &Tape::currentFrameChanged, this, &View::frameSelected);
}

View::~View() noexcept
{
}

Tape *View::tape() const
{
    return m_d->m_tape;
}

Frame *View::currentFrame() const
{
    return m_d->m_currentFrame;
}

QRect View::selectedRect() const
{
    if (m_d->m_crop) {
        return m_d->m_crop->cropRect();
    } else if (m_d->m_text) {
        return m_d->m_text->selectionRect();
    } else if (m_d->m_rect) {
        return m_d->m_rect->selectionRect();
    } else if (m_d->m_arrow) {
        return m_d->m_arrow->selectionRect();
    } else {
        return QRect();
    }
}

void View::startCrop()
{
    if (!m_d->m_crop) {
        m_d->m_crop = new CropFrame(m_d->m_currentFrame);
        m_d->m_crop->setStartMessage(
            tr("Select a region for cropping with the mouse, when ready press Enter. "
               "Press Escape for cancelling."));
        connect(m_d->m_crop, &CropFrame::applyEdit, this, &View::applyEdit);
        m_d->m_crop->setGeometry(QRect(0, 0, m_d->m_currentFrame->width(), m_d->m_currentFrame->height()));
        m_d->m_crop->show();
        m_d->m_crop->raise();
        m_d->m_crop->start();
    }
}

void View::stopCrop()
{
    if (m_d->m_crop) {
        m_d->m_crop->stop();
        m_d->m_crop->deleteLater();
        m_d->m_crop = nullptr;
    }
}

void View::startText()
{
    if (!m_d->m_text) {
        m_d->m_text = new TextFrame(tape(), m_d->m_currentFrame);
        m_d->m_text->setStartMessage(
            tr("Select a region for text with the mouse, when ready press Enter or use "
               "context menu. You can switch between text mode and rectangle selection with "
               "context meny at any time. You can choose any frame from the tape to apply "
               "text on that frame. Text may be different on each frame. If you clicked on "
               "the frame, but don't want the text to be on it - uncheck this frame on the "
               "tape. When ready click \"Apply\" button on the tool bar. "
               "Press Escape for cancelling."));
        m_d->m_text->setGeometry(QRect(0, 0, m_d->m_currentFrame->width(), m_d->m_currentFrame->height()));
        m_d->m_text->show();
        m_d->m_text->raise();
        m_d->m_text->start();
    }
}

TextFrame *View::textFrame() const
{
    return m_d->m_text;
}

CropFrame *View::cropFrame() const
{
    return m_d->m_crop;
}

RectFrame *View::rectFrame() const
{
    return m_d->m_rect;
}

ArrowFrame *View::arrowFrame() const
{
    return m_d->m_arrow;
}

void View::stopText()
{
    if (m_d->m_text) {
        m_d->m_text->stop();
        m_d->m_text->clear();
        m_d->m_text->deleteLater();
        m_d->m_text = nullptr;
    }
}

void View::startRect()
{
    if (!m_d->m_rect) {
        m_d->m_rect = new RectFrame(tape(), m_d->m_currentFrame);
        m_d->m_rect->setStartMessage(
            tr("Select a region for drawing a rectangle with the mouse, when ready press Enter. "
               "You can choose any frame from the tape to apply "
               "rectangle on that frame. If you clicked on "
               "the frame, but don't want the rectangle to be on it - uncheck this frame on the "
               "tape. Press Escape for cancelling."));
        connect(m_d->m_rect, &RectFrame::applyEdit, this, &View::applyEdit);
        m_d->m_rect->setGeometry(QRect(0, 0, m_d->m_currentFrame->width(), m_d->m_currentFrame->height()));
        m_d->m_rect->show();
        m_d->m_rect->raise();
        m_d->m_rect->start();
    }
}

void View::stopRect()
{
    if (m_d->m_rect) {
        m_d->m_rect->stop();
        m_d->m_rect->deleteLater();
        m_d->m_rect = nullptr;
    }
}

void View::startArrow()
{
    if (!m_d->m_arrow) {
        m_d->m_arrow = new ArrowFrame(tape(), m_d->m_currentFrame);
        m_d->m_arrow->setStartMessage(
            tr("Select a region for drawing an arrow with the mouse, when ready press Enter. "
               "You can choose any frame from the tape to apply "
               "arrow on that frame. If you clicked on "
               "the frame, but don't want the arrow to be on it - uncheck this frame on the "
               "tape. Press Escape for cancelling."));
        connect(m_d->m_arrow, &ArrowFrame::applyEdit, this, &View::applyEdit);
        m_d->m_arrow->setGeometry(QRect(0, 0, m_d->m_currentFrame->width(), m_d->m_currentFrame->height()));
        m_d->m_arrow->show();
        m_d->m_arrow->raise();
        m_d->m_arrow->start();
    }
}

void View::stopArrow()
{
    if (m_d->m_arrow) {
        m_d->m_arrow->stop();
        m_d->m_arrow->deleteLater();
        m_d->m_arrow = nullptr;
    }
}

void View::startTextEditing()
{
    if (m_d->m_text) {
        m_d->m_text->startTextEditing();
    }
}

void View::frameSelected(int idx)
{
    if (idx >= 1 && idx <= m_d->m_tape->count()) {
        m_d->m_currentFrame->setImagePos(idx - 1);
        m_d->m_currentFrame->applyImage();
    } else {
        m_d->m_currentFrame->clearImage();
    }
}

void View::scrollTo(int idx)
{
    const auto viewWidth = m_d->m_scroll->viewport()->width();
    const auto frameX = m_d->m_tape->xOfFrame(idx);
    const auto frameWidth = m_d->m_tape->currentFrame()->width();
    const auto spacing = m_d->m_tape->spacing();

    m_d->m_scroll->scrollContentsTo((frameX + frameWidth + spacing) - viewWidth);
}
