/*!
	SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GIF_EDITOR_BUSYINDICATOR_HPP_INCLUDED
#define GIF_EDITOR_BUSYINDICATOR_HPP_INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>


//
// BusyIndicator
//

class BusyIndicatorPrivate;

/*!
	BusyIndicator is a widget that shows activity of
	the application, more precisely busy of the application.
	This widget is a ring that spinning.
*/
class BusyIndicator
	:	public QWidget
{
	Q_OBJECT

	/*!
		\property running

		This property holds whether the busy indicator is currently
		indicating activity.

		\note The indicator is only visible when this property
		is set to true.

		The default value is true.
	*/
	Q_PROPERTY( bool running READ isRunning WRITE setRunning )
	/*!
		\property color

		\brief color used to paint indicator

		By default this color is QPalette::Highlight.
	*/
	Q_PROPERTY( QColor color READ color WRITE setColor )
	/*!
		\property radius

		\brief radius of the busy indicator.

		By default, this property is 10.
	*/
	Q_PROPERTY( int radius READ radius WRITE setRadius )
	/*!
		\property percent
		
		\brief percent displayed in the center
	*/
	Q_PROPERTY( int percent READ percent WRITE setPercent )
	/*!
		\property showPercent
		
		\brief show percents in the center?
	*/
	Q_PROPERTY( bool showPercent READ showPercent WRITE setShowPercent )

public:
	BusyIndicator( QWidget * parent = nullptr );
	~BusyIndicator() noexcept override;

	//! \return Is busy indicator running?
	bool isRunning() const;
	//! Set busy indicator running property.
	void setRunning( bool on );

	//! \return Color used to paint indicator.
	const QColor & color() const;
	//! Set color used to paint indicator.
	void setColor( const QColor & c );

	//! \return Radius.
	int radius() const;
	//! Set radius.
	void setRadius( int r );
	
	//! \return Percent.
	int percent() const;
	//! Set percent.
	void setPercent( int p );
	
	//! \return Show percents in the center?
	bool showPercent() const;
	//! Set show percents in the center.
	void setShowPercent( bool on = true );

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

protected:
	void paintEvent( QPaintEvent * ) override;

private slots:
	void _q_update( const QVariant & );

private:
	friend class BusyIndicatorPrivate;

	Q_DISABLE_COPY( BusyIndicator )

	QScopedPointer< BusyIndicatorPrivate > d;
}; // class BusyIndicator

#endif // GIF_EDITOR_BUSYINDICATOR_HPP_INCLUDED
