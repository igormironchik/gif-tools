/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "tips.hpp"
#include "constants.hpp"

// Qt include.
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

//
// Tips
//

Tips::Tips(QWidget *parent)
    : QScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto t = new QLabel(this);
    t->setContentsMargins(10, 10, 10, 10);
    t->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    t->setWordWrap(true);
    t->setText(tr("<h1>Tips & Tricks</h1>"
                  "<ul><li>"
                  "GIF editor can play GIFs. There is a button for this on the tool bar. You can use "
                  "Space key for start/stop playing."
                  "</li>"
                  "<li>"
                  "On the tape you can use context menu for such actions like - saving the frame into disk, "
                  "setting delay after the frame..."
                  "</li>"
                  "<li>"
                  "You can use different edit tools, like"
                  "<ul>"
                  "<li>"
                  "<p>Crop entire GIF. When crop is started...</p>"
                  "<p>%1</p>"
                  "</li>"
                  "<li>"
                  "<p>Add text into GIF. When tool is started...</p>"
                  "<p>%2</p>"
                  "</li>"
                  "<li>"
                  "<p>Add a rectangle into GIF. When tool is started...</p>"
                  "<p>%3</p>"
                  "</li>"
                  "<li>"
                  "<p>Add an arrow into GIF. When tool is started...</p>"
                  "<p>%4</p>"
                  "</li>"
                  "</ul>"
                  "</li>"
                  "</ul>")
                   .arg(TranslatedStrings::cropHelp(), TranslatedStrings::textHelp(), TranslatedStrings::rectHelp(), TranslatedStrings::arrowHelp()));

    setWidget(t);
}
