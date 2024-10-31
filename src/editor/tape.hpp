/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_TAPE_HPP_INCLUDED
#define GIF_EDITOR_TAPE_HPP_INCLUDED

// Qt include.
#include <QScopedPointer>
#include <QWidget>

// GIF editor include.
#include "frame.hpp"

class FrameOnTape;

//
// Tape
//

class TapePrivate;

//! Tape with frames.
class Tape final : public QWidget
{
    Q_OBJECT

signals:
    //! Frame clicked.
    void clicked(int idx);
    //! Current frame changed.
    void currentFrameChanged(int idx);
    //! Frame checked/unchecked.
    void checkStateChanged(int idx, bool checked);

public:
    Tape(QWidget *parent = nullptr);
    ~Tape() noexcept override;

    //! \return Count of frames.
    int count() const;
    //! Add frame.
    void addFrame(const ImageRef &img);
    //! \return Frame.
    FrameOnTape *frame(int idx) const;
    //! \return Current frame.
    FrameOnTape *currentFrame() const;
    //! Set current frame.
    void setCurrentFrame(int idx);
    //! Clear.
    void clear();
    //! Remove unchecked frames.
    void removeUnchecked();
    //! Remove frame.
    void removeFrame(int idx);
    //! \return X coordinate of left border of the given frame.
    int xOfFrame(int idx) const;
    //! \return Layout spacing.
    int spacing() const;

private slots:
    //! Check/uncheck till end action activated.
    void checkTillEnd(int idx, bool on);

private:
    Q_DISABLE_COPY(Tape)

    QScopedPointer<TapePrivate> m_d;
}; // class Tape

#endif // GIF_EDITOR_TAPE_HPP_INCLUDED
