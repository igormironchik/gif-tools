/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "utils.hpp"

// Qt include.
#include <QIcon>
#include <QModelIndex>
#include <QPixmapCache>
#include <QStringList>
#include <QStyle>
#include <QStyleHints>
#include <QWidget>
#include <QWindow>
#include <QtResource>

#ifdef MD_BREEZE
#include <KColorSchemeManager>
#include <KConfigGroup>
#include <KIconEngine>
#include <KIconLoader>
#include <KIconTheme>
#include <KSharedConfig>
#endif // MD_BREEZE

#ifdef Q_OS_WIN
#include <Windows.h>
#include <dwmapi.h>

#include <QStyleFactory>
#endif

void initTheme(QApplication &app)
{
#ifdef Q_OS_WIN
    app.setStyle(QStyleFactory::create("Breeze"));
#endif

#if defined(MD_BREEZE) && defined(Q_OS_WIN)
    const auto isDark = KColorSchemeManager::instance()->activeSchemeId().toLower().endsWith(QStringLiteral("dark"));

    setFallbackPathForIcons(isDark);

    if (isDark) {
        app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    } else {
        app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }
#endif
}

void refreshStyleRecursively(QWidget *widget,
                             const QPalette &p)
{
    if (!widget) {
        return;
    }

    widget->setPalette(p);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);

    for (QObject *child : std::as_const(widget->children())) {
        if (QWidget *childWidget = qobject_cast<QWidget *>(child)) {
            refreshStyleRecursively(childWidget, p);
        }
    }

    widget->repaint();
}

void applyTheme(const QString &name,
                bool isDark)
{
#if defined(MD_BREEZE) && defined(Q_OS_WIN)
    auto upper = name;
    upper[0] = upper[0].toUpper();
    const auto scheme = upper + (isDark ? QStringLiteral("Dark") : QStringLiteral("Light"));
    const auto idx = KColorSchemeManager::instance()->indexForSchemeId(scheme);

    if (idx.isValid()) {
        qApp->styleHints()->setColorScheme(isDark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
        auto cfg = KSharedConfig::openConfig();
        const auto iconThemeName = QStringLiteral("breeze") + (isDark ? QStringLiteral("-dark") : QString());
        auto cg = cfg->group(QStringLiteral("Icons"));
        cg.writeEntry(QStringLiteral("Theme"), iconThemeName);
        cg.sync();
        KIconTheme::forceThemeForTests(iconThemeName);
        KIconLoader::global()->reconfigure(qApp->applicationName());
        QPixmapCache::clear();
        KColorSchemeManager::instance()->activateScheme(idx);
    }
#endif

    const auto windows = QApplication::topLevelWidgets();

    for (const auto &w : windows) {
        if (w) {
            refreshStyleRecursively(w, qApp->palette());

#ifdef Q_OS_WIN
            if (w->winId()) {
                HWND hwnd = reinterpret_cast<HWND>(w->winId());
                BOOL useDarkMode = isDark;
                DWORD attribute = DWMWA_USE_IMMERSIVE_DARK_MODE;
                DwmSetWindowAttribute(hwnd, attribute, &useDarkMode, sizeof(useDarkMode));
                SendMessage(hwnd, WM_NCACTIVATE, FALSE, 0);
                SendMessage(hwnd, WM_NCACTIVATE, TRUE, 0);
            }
#endif
        }
    }
}

void initSharedResources()
{
    Q_INIT_RESOURCE(qt);
    Q_INIT_RESOURCE(icon);
    Q_INIT_RESOURCE(tr);
}

static const QChar s_dash = QLatin1Char('-');
static const QString s_english = QStringLiteral("en");

bool hasEnglish(const QStringList &langs)
{
    for (const auto &lang : langs) {
        const auto i = lang.indexOf(s_dash);

        if (i != -1) {
            if (lang.left(i).toLower() == s_english) {
                return true;
            }
        }
    }

    return false;
}
