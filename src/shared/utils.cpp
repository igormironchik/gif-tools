/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "utils.hpp"

// Qt include.
#include <QStringList>
#include <QtResource>

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
