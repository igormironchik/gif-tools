/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QScopedPointer>
#include <QThread>

//
// EventMonitor
//

struct EventMonitorPrivate;

//! Mosue events monitor.
class EventMonitor final : public QThread
{
    Q_OBJECT

signals:
    void buttonPress();
    void buttonRelease();

public:
    EventMonitor();
    ~EventMonitor() override;

    void stopListening();

protected:
    void run() override;

private:
    friend struct EventMonitorPrivate;

    Q_DISABLE_COPY(EventMonitor)

    QScopedPointer<EventMonitorPrivate> m_d;
}; // class EventMonitor
