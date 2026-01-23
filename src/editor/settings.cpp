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

QRect Settings::appWinRect() const
{
    return m_appWinRect;
}

void Settings::setAppWinRect(const QRect &r)
{
    m_appWinRect = r;

    saveCfg();
}

bool Settings::isAppWinMaximized() const
{
    return m_isAppWinMaximized;
}

QColor Settings::penColor() const
{
    return m_penColor;
}

void Settings::setPenColor(const QColor &c)
{
    m_penColor = c;

    saveCfg();
}

QColor Settings::brushColor() const
{
    return m_brushColor;
}

void Settings::setBrushColor(const QColor &c)
{
    m_brushColor = c;

    saveCfg();
}

int Settings::penWidth() const
{
    return m_penWidth;
}

void Settings::setPenWidth(int w)
{
    m_penWidth = w;

    saveCfg();
}

void Settings::setAppWinMaximized(bool on)
{
    m_isAppWinMaximized = on;

    saveCfg();
}

static const QString s_ui = QStringLiteral("ui");
static const QString s_showHelpMsg = QStringLiteral("showHelpMsg");
static const QString s_winGeometry = QStringLiteral("winGeometry");
static const QString s_winX = QStringLiteral("x");
static const QString s_winY = QStringLiteral("y");
static const QString s_winWidth = QStringLiteral("width");
static const QString s_winHeight = QStringLiteral("height");
static const QString s_winMaximized = QStringLiteral("maximized");
static const QString s_drawing = QStringLiteral("drawing");
static const QString s_penColor = QStringLiteral("penColor");
static const QString s_brushColor = QStringLiteral("brushColor");
static const QString s_penWidth = QStringLiteral("penWidth");

void Settings::readCfg()
{
    QSettings s;

    s.beginGroup(s_ui);

    m_showHelpMsg = s.value(s_showHelpMsg, true).toBool();

    s.endGroup();

    s.beginGroup(s_winGeometry);

    m_appWinRect.setX(s.value(s_winX, -1).toInt());
    m_appWinRect.setY(s.value(s_winY, -1).toInt());
    m_appWinRect.setWidth(s.value(s_winWidth, -1).toInt());
    m_appWinRect.setHeight(s.value(s_winHeight, -1).toInt());
    m_isAppWinMaximized = s.value(s_winMaximized, false).toBool();

    s.endGroup();

    s.beginGroup(s_drawing);

    m_penColor = QColor(s.value(s_penColor, QColor(Qt::black).name(QColor::HexArgb)).toString());
    m_brushColor = QColor(s.value(s_brushColor, QColor(Qt::transparent).name(QColor::HexArgb)).toString());
    m_penWidth = s.value(s_penWidth, 2).toInt();

    s.endGroup();
}

void Settings::saveCfg()
{
    QSettings s;

    s.beginGroup(s_ui);

    s.setValue(s_showHelpMsg, m_showHelpMsg);

    s.endGroup();

    s.beginGroup(s_winGeometry);

    s.setValue(s_winX, m_appWinRect.x());
    s.setValue(s_winY, m_appWinRect.y());
    s.setValue(s_winWidth, m_appWinRect.width());
    s.setValue(s_winHeight, m_appWinRect.height());
    s.setValue(s_winMaximized, m_isAppWinMaximized);

    s.endGroup();

    s.beginGroup(s_drawing);

    s.setValue(s_penColor, m_penColor.name(QColor::HexArgb));
    s.setValue(s_brushColor, m_brushColor.name(QColor::HexArgb));
    s.setValue(s_penWidth, m_penWidth);

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
