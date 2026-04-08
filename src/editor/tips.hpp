/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_TIPS_HPP_INCLUDED
#define GIF_EDITOR_TIPS_HPP_INCLUDED

// Qt include.
#include <QScrollArea>

//
// Tips
//

//! Tips & tricks widget.
class Tips final : public QScrollArea
{
    Q_OBJECT

public:
    explicit Tips(QWidget *parent);
    ~Tips() noexcept override = default;
}; // class Tips

#endif // GIF_EDITOR_TIPS_HPP_INCLUDED
