/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>

// GIF recorder include.
#include "ui_sizedlg.h"

//
// SizeDlg
//

//! Size dialog.
class SizeDlg : public QDialog
{
    Q_OBJECT

public:
    SizeDlg(int w,
            int h,
            QWidget *parent);
    ~SizeDlg() override = default;

    int requestedWidth() const;
    int requestedHeight() const;

private:
    Q_DISABLE_COPY(SizeDlg)

    Ui::SizeDlg m_ui;
}; // class SizeDlg
