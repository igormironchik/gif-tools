/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF recorder include.
#include "sizedlg.hpp"

//
// SizeDlg
//

SizeDlg::SizeDlg(int w,
                 int h,
                 QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.m_width->setValue(w);
    m_ui.m_height->setValue(h);
}

int SizeDlg::requestedWidth() const
{
    return m_ui.m_width->value();
}

int SizeDlg::requestedHeight() const
{
    return m_ui.m_height->value();
}
