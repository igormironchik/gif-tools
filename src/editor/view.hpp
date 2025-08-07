/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_VIEW_HPP_INCLUDED
#define GIF_EDITOR_VIEW_HPP_INCLUDED

// Qt include.
#include <QScopedPointer>
#include <QWidget>

// gif-editor include.
#include "frame.hpp"

// qgiflib include.
#include <qgiflib.hpp>

class Tape;

//
// View
//

class ViewPrivate;

//! View with current frame and tape with frames.
class View final : public QWidget
{
    Q_OBJECT

public:
    explicit View(const QGifLib::Gif &data,
                  QWidget *parent = nullptr);
    ~View() noexcept override;

    //! \return Tape.
    Tape *tape() const;
    //! \return Current frame.
    Frame *currentFrame() const;

    //! \return Crop rectangle.
    QRect cropRect() const;

public slots:
    //! Start crop.
    void startCrop();
    //! Stop crop.
    void stopCrop();
    //! Scroll to frame.
    void scrollTo(int idx);

private slots:
    //! Frame selected.
    void frameSelected(int idx);

private:
    Q_DISABLE_COPY(View)

    QScopedPointer<ViewPrivate> m_d;
}; // class View

#endif // GIF_EDITOR_VIEW_HPP_INCLUDED
