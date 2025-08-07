/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_FRAMEONTAPE_HPP_INCLUDED
#define GIF_EDITOR_FRAMEONTAPE_HPP_INCLUDED

// Qt include.
#include <QFrame>
#include <QScopedPointer>

// GIF editor include.
#include "frame.hpp"

//
// FrameOnTape
//

class FrameOnTapePrivate;

//! Frame on tape.
class FrameOnTape final : public QFrame
{
    Q_OBJECT

signals:
    //! Clicked.
    void clicked(int idx);
    //! Checked.
    void checked(int idx,
                 bool on);
    //! Check/uncheck till end action activated.
    void checkTillEnd(int idx,
                      bool on);

public:
    FrameOnTape(const ImageRef &img,
                int counter,
                int height,
                QWidget *parent = nullptr);
    ~FrameOnTape() noexcept override;

    //! \return Image.
    const ImageRef &image() const;
    //! Set image.
    void setImagePos(qsizetype pos);
    //! Clear image.
    void clearImage();
    //! Apply image.
    void applyImage();

    //! \return Is frame checked.
    bool isChecked() const;
    //! Set checked.
    void setChecked(bool on = true);

    //! \return Counter.
    int counter() const;
    //! Set counter.
    void setCounter(int c);

    //! \return Is this frame current?
    bool isCurrent() const;
    //! Set current flag.
    void setCurrent(bool on = true);

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    Q_DISABLE_COPY(FrameOnTape)

    QScopedPointer<FrameOnTapePrivate> m_d;
}; // class FrameOnTape

#endif // GIF_EDITOR_FRAMEONTAPE_HPP_INCLUDED
