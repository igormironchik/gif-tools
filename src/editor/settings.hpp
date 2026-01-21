/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_SETTINGS_HPP_INCLUDED
#define GIF_EDITOR_SETTINGS_HPP_INCLUDED

// GIF editor include.
#include "ui_settings.h"

// Qt include.
#include <QDialog>

//
// Settings
//

//! Settings.
class Settings
{
    Settings();
    ~Settings();

public:
    static Settings &instance();

    //! \return Show help messages?
    bool showHelpMsg() const;
    //! Turn on/off show help messages.
    void setShowHelpMsg(bool on = true);

private:
    void readCfg();
    void saveCfg();

private:
    //! Show help messages?
    bool m_showHelpMsg = true;
}; // class Settings

//
// SettingsDlg
//

//! Settings dialog.
class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDlg(QWidget *parent = nullptr);
    ~SettingsDlg() override;

private slots:
    //! OK.
    void onApply();

private:
    Ui::SettingsDlg m_ui;
}; // class SettingsDlg

#endif // GIF_EDITOR_SETTINGS_HPP_INCLUDED
