/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_CROP_HPP_INCLUDED
#define GIF_EDITOR_CROP_HPP_INCLUDED

// Qt include.
#include <QScopedPointer>
#include <QWidget>

class Frame;

//
// CropFrame
//

class CropFramePrivate;

//! Crop frame.
class CropFrame final : public QWidget
{
    Q_OBJECT

public:
    CropFrame(Frame *parent = nullptr);
    ~CropFrame() noexcept override;

    //! \return Crop rectangle.
    QRect cropRect() const;

public slots:
    //! Start.
    void start();
    //! Stop.
    void stop();

private slots:
    //! Frame resized.
    void frameResized();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    Q_DISABLE_COPY(CropFrame)

    QScopedPointer<CropFramePrivate> m_d;
}; // class CropFrame

#endif // GIF_EDITOR_CROP_HPP_INCLUDED
