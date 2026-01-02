/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "about.hpp"

// Qt include.
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

//
// About
//

About::About(QWidget *parent)
    : QWidget(parent)
{
    auto l = new QHBoxLayout(this);
    l->setSpacing(50);
    l->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

    auto p = new QLabel(this);
    p->setPixmap(QPixmap(QStringLiteral(":/img/icon_128x128.png")));
    l->addWidget(p);

    auto t = new QLabel(this);
    t->setText(
        tr("GIF editor.\n\n"
           "Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
           "Copyright (c) 2018 Igor Mironchik.\n\n"
           "Licensed under GNU GPL 3.0."));
    l->addWidget(t);

    l->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
}
