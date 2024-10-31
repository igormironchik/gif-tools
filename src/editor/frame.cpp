/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "frame.hpp"

// Qt include.
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QRunnable>
#include <QThreadPool>

//
// FramePrivate
//

class FramePrivate
{
public:
    FramePrivate(const ImageRef &img, Frame::ResizeMode mode, Frame *parent)
        : m_image(img)
        , m_mode(mode)
        , m_dirty(false)
        , m_q(parent)
    {
    }

    //! Create thumbnail.
    void createThumbnail(int height);
    //! Frame widget was resized.
    void resized(int height = -1);

    //! Image reference.
    ImageRef m_image;
    //! Thumbnail.
    QImage m_thumbnail;
    //! Resize mode.
    Frame::ResizeMode m_mode;
    //! Dirty frame. We need to resize the image to actual size before drawing.
    bool m_dirty;
    //! Width.
    int m_width = 0;
    //! Height.
    int m_height = 0;
    //! Desired height.
    int m_desiredHeight = -1;
    //! Parent.
    Frame *m_q;
}; // class FramePrivate

class ThumbnailCreator final : public QRunnable
{
public:
    ThumbnailCreator(QImage img, int width, int height, int desiredHeight, Frame::ResizeMode mode)
        : m_img(img)
        , m_width(width)
        , m_height(height)
        , m_desiredHeight(desiredHeight)
        , m_mode(mode)
    {
        setAutoDelete(false);
    }

    const QImage &image() const
    {
        return m_thumbnail;
    }

    void run() override
    {
        if (m_img.width() > m_width || m_img.height() > m_height) {
            m_thumbnail = m_img.scaledToHeight(m_desiredHeight > 0 ? m_desiredHeight : m_height, Qt::SmoothTransformation);
        } else {
            m_thumbnail = m_img;
        }
    }

private:
    QImage m_img;
    int m_width;
    int m_height;
    int m_desiredHeight;
    Frame::ResizeMode m_mode;
    QImage m_thumbnail;
};

void FramePrivate::createThumbnail(int height)
{
    m_dirty = false;

    if (!m_image.m_isEmpty) {
        m_height = m_q->height();
        m_width = m_q->width();
        m_desiredHeight = height;

        if (m_mode == Frame::ResizeMode::FitToHeight) {
            ThumbnailCreator c(m_image.m_gif.at(m_image.m_pos), m_q->width(), m_q->height(), height, m_mode);

            QThreadPool::globalInstance()->start(&c);

            while (!QThreadPool::globalInstance()->waitForDone(5)) {
                QApplication::processEvents();
            }

            m_thumbnail = c.image();
        } else {
            const auto img = m_image.m_gif.at(m_image.m_pos);

            if (img.width() > m_q->width() || img.height() > m_q->height()) {
                m_thumbnail = img.scaled(m_q->width(), m_q->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            } else {
                m_thumbnail = img;
            }
        }
    }
}

void FramePrivate::resized(int height)
{
    if (m_dirty || height != m_desiredHeight || m_q->width() != m_width || m_q->height() != m_height) {
        createThumbnail(height);

        m_q->updateGeometry();

        emit m_q->resized();
    }
}

//
// Frame
//

Frame::Frame(const ImageRef &img, ResizeMode mode, QWidget *parent, int height)
    : QWidget(parent)
    , m_d(new FramePrivate(img, mode, this))
{
    switch (mode) {
    case ResizeMode::FitToSize:
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        break;

    case ResizeMode::FitToHeight: {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_d->resized(height);
    } break;
    }
}

Frame::~Frame() noexcept
{
}

const ImageRef &Frame::image() const
{
    return m_d->m_image;
}

void Frame::setImagePos(qsizetype pos)
{
    m_d->m_image.m_pos = pos;
    m_d->m_desiredHeight = -1;
    m_d->m_width = 0;
    m_d->m_height = 0;
}

void Frame::clearImage()
{
    m_d->m_image.m_isEmpty = true;
    m_d->m_thumbnail = QImage();
    m_d->m_desiredHeight = -1;
    m_d->m_width = 0;
    m_d->m_height = 0;
    update();
}

void Frame::applyImage()
{
    m_d->m_image.m_isEmpty = false;

    m_d->resized();

    update();
}

QRect Frame::thumbnailRect() const
{
    const int x = (width() - m_d->m_thumbnail.width()) / 2;
    const int y = (height() - m_d->m_thumbnail.height()) / 2;

    QRect r = m_d->m_thumbnail.rect();
    r.moveTopLeft(QPoint(x, y));

    return r;
}

QRect Frame::imageRect() const
{
    if (!m_d->m_image.m_isEmpty) {
        const auto img = m_d->m_image.m_gif.at(m_d->m_image.m_pos);

        return QRect(0, 0, img.width(), img.height());
    } else {
        return {};
    }
}

QSize Frame::sizeHint() const
{
    return (m_d->m_thumbnail.isNull() ? QSize(10, 10) : m_d->m_thumbnail.size());
}

void Frame::paintEvent(QPaintEvent *)
{
    if (m_d->m_dirty) {
        m_d->resized();
    }

    QPainter p(this);
    p.drawImage(thumbnailRect(), m_d->m_thumbnail, m_d->m_thumbnail.rect());
}

void Frame::resizeEvent(QResizeEvent *e)
{
    if (m_d->m_mode == ResizeMode::FitToSize ||
        (m_d->m_mode == ResizeMode::FitToHeight && e->size().height() != m_d->m_thumbnail.height())) {
        m_d->m_dirty = true;
    }

    e->accept();
}

void Frame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        emit clicked();

        e->accept();
    } else {
        e->ignore();
    }
}
