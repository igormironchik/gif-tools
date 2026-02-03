/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "delay.hpp"

// Qt include.
#include <QSpinBox>

//
// DelayDlg
//

DelayDlg::DelayDlg(int ms,
                   QWidget *parent)
{
    m_ui.setupUi(this);

    m_ui.m_delay->setValue(ms);
}

DelayDlg::~DelayDlg()
{
}

int DelayDlg::delay() const
{
    return m_ui.m_delay->value();
}
