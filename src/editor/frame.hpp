/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_FRAME_HPP_INCLUDED
#define GIF_EDITOR_FRAME_HPP_INCLUDED

// Qt include.
#include <QScopedPointer>
#include <QWidget>

// qgiflib include.
#include <qgiflib.hpp>

//
// ImageRef
//

//! Reference to full image.
struct ImageRef final {
    const QGifLib::Gif &m_gif;
    qsizetype m_pos;
    bool m_isEmpty;
}; // struct ImageRef

//
// Frame
//

class FramePrivate;

//! This is just an image with frame that fit the given size or height.
class Frame final : public QWidget
{
    Q_OBJECT

signals:
    //! Clicked.
    void clicked();
    //! Resized.
    void resized();

public:
    //! Resize mode.
    enum class ResizeMode {
        //! Fit to size.
        FitToSize,
        //! Fit to height.
        FitToHeight
    }; // enum class ResizeMode

    Frame(const ImageRef &img,
          ResizeMode mode,
          QWidget *parent = nullptr,
          int height = -1);
    ~Frame() noexcept override;

    //! \return Image.
    const ImageRef &image() const;
    //! Set image.
    void setImagePos(qsizetype pos);
    //! Clear image.
    void clearImage();
    //! Apply image.
    void applyImage();
    //! \return Thumbnail image rect.
    QRect thumbnailRect() const;
    //! \return Image rect.
    QRect imageRect() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    Q_DISABLE_COPY(Frame)

    QScopedPointer<FramePrivate> m_d;
}; // class Frame

#endif // GIF_EDITOR_FRAME_HPP_INCLUDED
