/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_DELAY_HPP_INCLUDED
#define GIF_EDITOR_DELAY_HPP_INCLUDED

// GIF editor include.
#include "ui_delay.h"

// Qt include.
#include <QDialog>

//
// DelayDlg
//

//! Delay dialog.
class DelayDlg : public QDialog
{
    Q_OBJECT

public:
    explicit DelayDlg(int ms, QWidget *parent = nullptr);
    ~DelayDlg() override;

    //! \return currently set delay.
    int delay() const;

private:
    Ui::DelayDlg m_ui;
}; // class DelayDlg

#endif // GIF_EDITOR_DELAY_HPP_INCLUDED
