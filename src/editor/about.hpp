/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_ABOUT_HPP_INCLUDED
#define GIF_EDITOR_ABOUT_HPP_INCLUDED

// Qt include.
#include <QWidget>

//
// About
//

//! Widget about.
class About final : public QWidget
{
    Q_OBJECT

public:
    explicit About(QWidget *parent);
    ~About() noexcept override = default;
}; // class About

#endif // GIF_EDITOR_ABOUT_HPP_INCLUDED
