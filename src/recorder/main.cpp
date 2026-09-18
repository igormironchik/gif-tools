/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QApplication>
#include <QMessageBox>
#include <QTranslator>

#ifdef Q_OS_WIN
#include <QStyleFactory>
#endif

// GIF recorder include.
#include "event_monitor.hpp"
#include "mainwindow.hpp"

// gif-widgets include.
#include "utils.hpp"

#ifdef GIF_BREEZE
#include <KIconTheme>
#endif

int main(int argc,
         char **argv)
{
#ifdef GIF_BREEZE
    KIconTheme::initTheme();
#endif
    QApplication app(argc, argv);

    int ret = -1;

#ifdef Q_OS_LINUX
    QString platform = QApplication::platformName();

    if (platform == "xcb") {
#endif
        initTheme(app);

        initSharedResources();

        QIcon appIcon(QStringLiteral(":/icon/icon_256x256.png"));
        appIcon.addFile(QStringLiteral(":/icon/icon_128x128.png"));
        appIcon.addFile(QStringLiteral(":/icon/icon_64x64.png"));
        appIcon.addFile(QStringLiteral(":/icon/icon_48x48.png"));
        appIcon.addFile(QStringLiteral(":/icon/icon_32x32.png"));
        appIcon.addFile(QStringLiteral(":/icon/icon_22x22.png"));
        appIcon.addFile(QStringLiteral(":/icon/icon_16x16.png"));
        app.setWindowIcon(appIcon);

        QTranslator appTranslator;
        const auto locale = QLocale::system();

        if (!hasEnglish(locale.uiLanguages())) {
            if (appTranslator.load(locale, QStringLiteral("gif_"), QString(), QStringLiteral(":/tr/"))) {
                QApplication::installTranslator(&appTranslator);
            }
        }

        EventMonitor m;

        MainWindow w(&m);
        w.show();

        m.start();

        ret = QApplication::exec();

        m.stopListening();
        m.quit();
        m.wait();

#ifdef Q_OS_LINUX
    } else {
        QMessageBox::critical(nullptr,
                              QObject::tr("Unable to start application"),
                              QObject::tr("This application can work under X11 only, Wayland is not supported."));

        ret = QApplication::exec();
    }
#endif

    return ret;
}
