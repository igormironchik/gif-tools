/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF recorder include.
#include "settings.hpp"

// Qt include.
#include <QCheckBox>
#include <QSpinBox>

//
// Settings
//

Settings::Settings(int fpsValue,
                   bool grabCursorValue,
                   bool drawMouseClicks,
                   QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.m_fps->setValue(fpsValue);
    m_ui.m_cursor->setChecked(grabCursorValue);
    m_ui.m_click->setChecked(drawMouseClicks);
}

int Settings::fps() const
{
    return m_ui.m_fps->value();
}

bool Settings::grabCursor() const
{
    return m_ui.m_cursor->isChecked();
}

bool Settings::drawMouseClicks() const
{
    return m_ui.m_click->isChecked();
}
