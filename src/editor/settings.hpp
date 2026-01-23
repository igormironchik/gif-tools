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
    //! \return Rect of the application's window.
    QRect appWinRect() const;
    //! Set rect of the application's window.
    void setAppWinRect(const QRect &r);
    //! \return Is application's window maximized?
    bool isAppWinMaximized() const;
    //! Set whether application's window maximized.
    void setAppWinMaximized(bool on = true);
    //! \return Pen color.
    QColor penColor() const;
    //! Set pen color.
    void setPenColor(const QColor &c);
    //! \return Brush color.
    QColor brushColor() const;
    //! Set brush color.
    void setBrushColor(const QColor &c);
    //! \return Pen width.
    int penWidth() const;
    //! Set pen width.
    void setPenWidth(int w);

private:
    void readCfg();
    void saveCfg();

private:
    //! Show help messages?
    bool m_showHelpMsg = true;
    //! Rect of the application's window.
    QRect m_appWinRect = {-1, -1, -1, -1};
    //! Is application's window maximized?
    bool m_isAppWinMaximized = false;
    //! Pen color.
    QColor m_penColor = Qt::black;
    //! Brush color.
    QColor m_brushColor = Qt::transparent;
    //! Pen widht.
    int m_penWidth = 2;
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
