/*!
	SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// gif-recorder include.
#include "event_monitor.hpp"

#ifdef Q_OS_LINUX
// Xlib include.
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/Xlibint.h>
#endif


//
// EventMonitorPrivate
//

struct EventMonitorPrivate {
	EventMonitorPrivate( EventMonitor * parent )
		:	q( parent )
	{
	}

	EventMonitor * q;

#ifdef Q_OS_LINUX
	Display * display = nullptr;
	Display * display_datalink = nullptr;
	XRecordContext context;
	XRecordRange * range = nullptr;

    static void callback( XPointer ptr, XRecordInterceptData * data );
    void handleRecordEvent( XRecordInterceptData * data );
	bool filterWheelEvent( int detail );
#endif
}; // struct EventMonitorPrivate


#ifdef Q_OS_LINUX

void
EventMonitorPrivate::callback( XPointer ptr, XRecordInterceptData * data )
{
	((EventMonitorPrivate*) ptr)->handleRecordEvent( data );
}

void
EventMonitorPrivate::handleRecordEvent( XRecordInterceptData * data )
{
	if( data->category == XRecordFromServer )
	{
		xEvent * event = (xEvent *) data->data;

		switch( event->u.u.type )
		{
			case ButtonPress:
			{
				if( filterWheelEvent( event->u.u.detail ) )
					emit q->buttonPress();
			}
				break;

			case ButtonRelease:
			{
				if( filterWheelEvent( event->u.u.detail ) )
					emit q->buttonRelease();
			}
				break;

			default:
				break;
		}
	}

	XRecordFreeData( data );
}

bool
EventMonitorPrivate::filterWheelEvent( int detail )
{
	return !( detail >= 4 && detail <= 7 );
}

#endif // Q_OS_LINUX


//
// EventMonitor
//

EventMonitor::EventMonitor()
	:	d( new EventMonitorPrivate( this ) )
{
}

EventMonitor::~EventMonitor()
{
#ifdef Q_OS_LINUX
	XRecordFreeContext( d->display, d->context );
	XFree( d->range );
	XCloseDisplay( d->display );
	XCloseDisplay( d->display_datalink );
#endif // Q_OS_LINUX
}

void
EventMonitor::stopListening()
{
#ifdef Q_OS_LINUX
	XRecordDisableContext( d->display_datalink, d->context );
	XSync( d->display_datalink, true );
#endif // Q_OS_LINUX
}

void
EventMonitor::run()
{
#ifdef Q_OS_LINUX
	d->display = XOpenDisplay( nullptr );
	if( !d->display ) return;

	XRecordClientSpec clients = XRecordAllClients;
	d->range = XRecordAllocRange();
	if( !d->range ) return;

	memset( d->range, 0, sizeof( XRecordRange ) );
	d->range->device_events.first = ButtonPress;
	d->range->device_events.last  = ButtonRelease;

	d->context = XRecordCreateContext( d->display, 0, &clients, 1, &d->range, 1 );
	if( !d->context ) return;

	XSync( d->display, true );

	d->display_datalink = XOpenDisplay( nullptr );
	if( !d->display_datalink) return;

	XSync( d->display_datalink, true );

	if( !XRecordEnableContext( d->display, d->context, d->callback, (XPointer) d.data() ) )
		return;
#endif // Q_OS_LINUX
}
