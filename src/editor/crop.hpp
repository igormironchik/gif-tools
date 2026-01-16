/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_CROP_HPP_INCLUDED
#define GIF_EDITOR_CROP_HPP_INCLUDED

// GIF editor include.
#include "rectangle.hpp"

//
// CropFrame
//

//! Crop frame.
class CropFrame final : public RectangleSelection
{
    Q_OBJECT

signals:
    //! Apply editing.
    void applyEdit();

public:
    CropFrame(Frame *parent = nullptr);
    ~CropFrame() noexcept override;

    //! \return Crop rectangle.
    QRect cropRect() const;

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    Q_DISABLE_COPY(CropFrame)
}; // class CropFrame

#endif // GIF_EDITOR_CROP_HPP_INCLUDED
