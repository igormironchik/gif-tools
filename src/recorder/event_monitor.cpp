/*!
    SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// gif-recorder include.
#include "event_monitor.hpp"

#ifdef Q_OS_LINUX
// Xlib include.
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/extensions/record.h>
#endif

//
// EventMonitorPrivate
//

struct EventMonitorPrivate {
    EventMonitorPrivate(EventMonitor *parent)
        : m_q(parent)
    {
    }

    EventMonitor *m_q;

#ifdef Q_OS_LINUX
    Display *m_display = nullptr;
    Display *m_display_datalink = nullptr;
    XRecordContext m_context;
    XRecordRange *m_range = nullptr;

    static void callback(XPointer ptr,
                         XRecordInterceptData *data);
    void handleRecordEvent(XRecordInterceptData *data);
    bool filterWheelEvent(int detail);
#endif
}; // struct EventMonitorPrivate

#ifdef Q_OS_LINUX

void EventMonitorPrivate::callback(XPointer ptr,
                                   XRecordInterceptData *data)
{
    ((EventMonitorPrivate *)ptr)->handleRecordEvent(data);
}

void EventMonitorPrivate::handleRecordEvent(XRecordInterceptData *data)
{
    if (data->category == XRecordFromServer) {
        xEvent *event = (xEvent *)data->data;

        switch (event->u.u.type) {
        case ButtonPress: {
            if (filterWheelEvent(event->u.u.detail))
                emit m_q->buttonPress();
        } break;

        case ButtonRelease: {
            if (filterWheelEvent(event->u.u.detail))
                emit m_q->buttonRelease();
        } break;

        default:
            break;
        }
    }

    XRecordFreeData(data);
}

bool EventMonitorPrivate::filterWheelEvent(int detail)
{
    return !(detail >= 4 && detail <= 7);
}

#endif // Q_OS_LINUX

//
// EventMonitor
//

EventMonitor::EventMonitor()
    : m_d(new EventMonitorPrivate(this))
{
}

EventMonitor::~EventMonitor()
{
#ifdef Q_OS_LINUX
    XRecordFreeContext(m_d->m_display, m_d->m_context);
    XFree(m_d->m_range);
    XCloseDisplay(m_d->m_display);
    XCloseDisplay(m_d->m_display_datalink);
#endif // Q_OS_LINUX
}

void EventMonitor::stopListening()
{
#ifdef Q_OS_LINUX
    XRecordDisableContext(m_d->m_display_datalink, m_d->m_context);
    XSync(m_d->m_display_datalink, true);
#endif // Q_OS_LINUX
}

void EventMonitor::run()
{
#ifdef Q_OS_LINUX
    m_d->m_display = XOpenDisplay(nullptr);
    if (!m_d->m_display) {
        return;
    }

    XRecordClientSpec clients = XRecordAllClients;
    m_d->m_range = XRecordAllocRange();
    if (!m_d->m_range) {
        return;
    }

    memset(m_d->m_range, 0, sizeof(XRecordRange));
    m_d->m_range->device_events.first = ButtonPress;
    m_d->m_range->device_events.last = ButtonRelease;

    m_d->m_context = XRecordCreateContext(m_d->m_display, 0, &clients, 1, &m_d->m_range, 1);
    if (!m_d->m_context) {
        return;
    }

    XSync(m_d->m_display, true);

    m_d->m_display_datalink = XOpenDisplay(nullptr);
    if (!m_d->m_display_datalink) {
        return;
    }

    XSync(m_d->m_display_datalink, true);

    if (!XRecordEnableContext(m_d->m_display, m_d->m_context, m_d->callback, (XPointer)m_d.data())) {
        return;
    }
#endif // Q_OS_LINUX
}
