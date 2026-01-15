/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>

// GIF recorder include.
#include "ui_settings.h"

//
// Settings
//

//! Settings dialog.
class Settings : public QDialog
{
    Q_OBJECT

public:
    Settings(int fpsValue,
             bool grabCursorValue,
             bool drawMouseClicks,
             bool drawKeyboardKeysPresses,
             QWidget *parent);
    ~Settings() override = default;

    int fps() const;
    bool grabCursor() const;
    bool drawMouseClicks() const;
    bool drawKeyboardKeysPresses() const;

private:
    Q_DISABLE_COPY(Settings)

    Ui::Settings m_ui;
}; // class Settings
