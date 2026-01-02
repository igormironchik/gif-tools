/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "view.hpp"
#include "crop.hpp"
#include "frame.hpp"
#include "frameontape.hpp"
#include "tape.hpp"

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

QRect View::cropRect() const
{
    if (m_d->m_crop) {
        return m_d->m_crop->cropRect();
    } else {
        return QRect();
    }
}

void View::startCrop()
{
    if (!m_d->m_crop) {
        m_d->m_crop = new CropFrame(m_d->m_currentFrame);
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
    const auto x = m_d->m_scroll->horizontalScrollBar()->sliderPosition();
    const auto viewWidth = m_d->m_scroll->viewport()->width();
    const auto frameX = m_d->m_tape->xOfFrame(idx);
    const auto frameWidth = m_d->m_tape->currentFrame()->width();
    const auto spacing = m_d->m_tape->spacing();

    m_d->m_scroll->scrollContentsTo((frameX + frameWidth + spacing) - viewWidth);
}
