/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "settings.hpp"

// Qt include.
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSettings>

//
// Settings
//

Settings::Settings()
{
    readCfg();
}

Settings::~Settings()
{
}

Settings &Settings::instance()
{
    static Settings s;

    return s;
}

bool Settings::showHelpMsg() const
{
    return m_showHelpMsg;
}

void Settings::setShowHelpMsg(bool on)
{
    m_showHelpMsg = on;

    saveCfg();
}

static const QString s_ui = QStringLiteral("ui");
static const QString s_showHelpMsg = QStringLiteral("showHelpMsg");

void Settings::readCfg()
{
    QSettings s;

    s.beginGroup(s_ui);

    m_showHelpMsg = s.value(s_showHelpMsg, true).toBool();

    s.endGroup();
}

void Settings::saveCfg()
{
    QSettings s;

    s.beginGroup(s_ui);

    s.setValue(s_showHelpMsg, m_showHelpMsg);

    s.endGroup();
}

//
// SettingsDlg
//

SettingsDlg::SettingsDlg(QWidget *parent)
{
    m_ui.setupUi(this);

    m_ui.m_showHelpMsg->setChecked(Settings::instance().showHelpMsg());

    connect(m_ui.m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDlg::onApply);
}

SettingsDlg::~SettingsDlg()
{
}

void SettingsDlg::onApply()
{
    Settings::instance().setShowHelpMsg(m_ui.m_showHelpMsg->isChecked());
}
