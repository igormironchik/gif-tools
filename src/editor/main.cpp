/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QApplication>
#include <QLocale>
#include <QTranslator>

// GIF editor include.
#include "mainwindow.hpp"
#include "settings.hpp"

// gif-widgets include.
#include "utils.hpp"

int main(int argc,
         char **argv)
{
    QApplication app(argc, argv);

    app.setOrganizationName(QStringLiteral("Igor Mironchik"));
    app.setOrganizationDomain(QStringLiteral("github.com/igormironchik"));
    app.setApplicationName(QStringLiteral("GIF Editor"));

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
    if (appTranslator.load(QStringLiteral("./tr/gif-editor_") + QLocale::system().name())) {
        app.installTranslator(&appTranslator);
    }

    MainWindow w;
    w.resize(800, 600);
    w.show();

    return app.exec();
}
