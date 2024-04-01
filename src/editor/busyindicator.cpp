/*!
	SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "busyindicator.hpp"

// Qt include.
#include <QPainter>
#include <QVariantAnimation>
#include <QPainterPath>


//
// BusyIndicatorPrivate
//

class BusyIndicatorPrivate {
public:
	BusyIndicatorPrivate( BusyIndicator * parent )
		:	outerRadius( 10 )
		,	innerRadius( static_cast< int > ( outerRadius * 0.6 ) )
		,	size( outerRadius * 2, outerRadius * 2 )
		,	running( true )
		,	animation( nullptr )
		,	q( parent )
	{
	}

	void init();

	int outerRadius;
	int innerRadius;
	int percent = 0;
	QSize size;
	QColor color;
	bool running;
	bool showPercent = false;
	QVariantAnimation * animation;
	BusyIndicator * q;
}; // class BusyIndicatorPrivate

void
BusyIndicatorPrivate::init()
{
	animation = new QVariantAnimation( q );
	animation->setStartValue( 0.0 );
	animation->setEndValue( 359.0 );
	animation->setDuration( 1000 );
	animation->setLoopCount( -1 );

	QObject::connect( animation, &QVariantAnimation::valueChanged,
		q, &BusyIndicator::_q_update );

	color = q->palette().color( QPalette::Highlight );

	animation->start();
}


//
// BusyIndicator
//

BusyIndicator::BusyIndicator( QWidget * parent )
	:	QWidget( parent )
	,	d( new BusyIndicatorPrivate( this ) )
{
	d->init();
}

BusyIndicator::~BusyIndicator() noexcept
{
	d->animation->stop();
}

bool
BusyIndicator::isRunning() const
{
	return d->running;
}

void
BusyIndicator::setRunning( bool on )
{
	if( on != d->running )
	{
		d->running = on;

		if( d->running )
		{
			show();
			d->animation->start();
		}
		else
		{
			hide();
			d->animation->stop();
		}
	}
}

const QColor &
BusyIndicator::color() const
{
	return d->color;
}

void
BusyIndicator::setColor( const QColor & c )
{
	if( d->color != c )
	{
		d->color = c;
		update();
	}
}

int
BusyIndicator::radius() const
{
	return d->outerRadius;
}

void
BusyIndicator::setRadius( int r )
{
	if( d->outerRadius != r )
	{
		d->outerRadius = r;
		d->innerRadius = static_cast< int > ( d->outerRadius * 0.6 );
		d->size = QSize( d->outerRadius * 2, d->outerRadius * 2 );

		updateGeometry();
	}
}

int
BusyIndicator::percent() const
{
	return d->percent;
}

void
BusyIndicator::setPercent( int p )
{
	if( d->percent != p )
	{
		d->percent = p;
		
		if( d->showPercent )
			update();
	}
}

bool
BusyIndicator::showPercent() const
{
	return d->showPercent;
}

void
BusyIndicator::setShowPercent( bool on )
{
	if( d->showPercent != on )
	{
		d->showPercent = on;
		
		update();
	}
}

QSize
BusyIndicator::minimumSizeHint() const
{
	return d->size;
}

QSize
BusyIndicator::sizeHint() const
{
	return d->size;
}

void
BusyIndicator::paintEvent( QPaintEvent * )
{
	QPainter p( this );
	p.setRenderHint( QPainter::Antialiasing );
	p.translate( width() / 2, height() / 2 );

	QPainterPath path;
	path.setFillRule( Qt::OddEvenFill );
	path.addEllipse( - d->outerRadius, - d->outerRadius,
		d->outerRadius * 2, d->outerRadius * 2 );
	path.addEllipse( - d->innerRadius, - d->innerRadius,
		d->innerRadius * 2, d->innerRadius * 2 );

	p.setPen( Qt::NoPen );

	QConicalGradient gradient( 0, 0, - d->animation->currentValue().toReal() );
	gradient.setColorAt( 0.0, Qt::transparent );
	gradient.setColorAt( 0.05, d->color );
	gradient.setColorAt( 1.0, Qt::transparent );

	p.setBrush( gradient );

	p.drawPath( path );
	
	if( d->showPercent )
	{
		p.setBrush( d->color );
		p.setPen( d->color );
		auto f = p.font();
		f.setPixelSize( qRound( (double) d->innerRadius * 0.8 ) );
		p.setFont( f );
		p.drawText( QRect( -d->innerRadius, -d->innerRadius,
				d->innerRadius * 2, d->innerRadius * 2 ),
			Qt::AlignHCenter | Qt::AlignVCenter,
			QString( "%1%2" ).arg( QString::number( d->percent ),
				QStringLiteral( "%" ) ) );
	}
}

void
BusyIndicator::_q_update( const QVariant & )
{
	update();
}
