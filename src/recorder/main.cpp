/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QApplication>

// GIF recorder include.
#include "event_monitor.hpp"
#include "mainwindow.hpp"

// gif-widgets include.
#include "utils.hpp"

int main(int argc,
         char **argv)
{
    QApplication app(argc, argv);

    initSharedResources();

    QIcon appIcon(QStringLiteral(":/icon/icon_256x256.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_128x128.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_64x64.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_48x48.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_32x32.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_22x22.png"));
    appIcon.addFile(QStringLiteral(":/icon/icon_16x16.png"));
    app.setWindowIcon(appIcon);

    EventMonitor m;

    MainWindow w(&m);
    w.resize(800, 600);
    w.show();

    m.start();

    const auto ret = QApplication::exec();

    m.stopListening();
    m.quit();
    m.wait();

    return ret;
}
