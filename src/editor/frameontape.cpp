/*!
	SPDX-FileCopyrightText: 2018-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// GIF editor include.
#include "frameontape.hpp"

// Qt include.
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QMenu>
#include <QContextMenuEvent>
#include <QFileDialog>


//
// FrameOnTapePrivate
//

class FrameOnTapePrivate {
public:
	FrameOnTapePrivate( const ImageRef & img, int counter, int height, FrameOnTape * parent )
		:	m_counter( counter )
		,	m_current( false )
		,	m_label( new QLabel( parent ) )
		,	m_checkBox( new QCheckBox( parent ) )
		,	m_vlayout( new QVBoxLayout( parent ) )
		,	m_frame( nullptr )
		,	q( parent )
	{
		m_checkBox->setChecked( true );

		m_label->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
		m_label->setText( FrameOnTape::tr( "#%1" ).arg( m_counter ) );
		m_vlayout->setSpacing( 0 );
		m_vlayout->setContentsMargins( 0, 0, 0, 0 );
	}

	//! Set current state.
	void setCurrent( bool on );

	//! Counter.
	int m_counter;
	//! Is current?
	bool m_current;
	//! Counter label.
	QLabel * m_label;
	//! Check box.
	QCheckBox * m_checkBox;
	//! Layout.
	QVBoxLayout * m_vlayout;
	//! Frame.
	Frame * m_frame;
	//! Parent.
	FrameOnTape * q;
}; // class FrameOnTapePrivate

void
FrameOnTapePrivate::setCurrent( bool on )
{
	m_current = on;

	if( m_current )
		q->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	else
		q->setFrameStyle( QFrame::Panel | QFrame::Raised );
}


//
// FrameOnTape
//

FrameOnTape::FrameOnTape( const ImageRef & img, int counter, int height, QWidget * parent )
	:	QFrame( parent )
	,	d( new FrameOnTapePrivate( img, counter, height, this ) )
{
	setLineWidth( 2 );
	d->setCurrent( false );

	d->m_frame = new Frame( img, Frame::ResizeMode::FitToHeight, parent,
		height - qMax( d->m_label->sizeHint().height(), d->m_checkBox->sizeHint().height() ) -
			frameWidth() * 2 );

	d->m_vlayout->addWidget( d->m_frame );

	auto hlayout = new QHBoxLayout;
	hlayout->setContentsMargins( 0, 0, 0, 0 );
	hlayout->addWidget( d->m_checkBox );
	hlayout->addWidget( d->m_label );

	d->m_vlayout->addLayout( hlayout );

	d->setCurrent( false );

	setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );

	connect( d->m_checkBox, &QCheckBox::stateChanged, this,
		[this] ( int state ) { emit this->checked( this->d->m_counter, state != 0 ); } );
	connect( d->m_frame, &Frame::clicked, this,
		[this] ()
		{
			this->d->setCurrent( true );

			emit this->clicked( this->d->m_counter );
		} );
}

FrameOnTape::~FrameOnTape() noexcept
{
}

const ImageRef &
FrameOnTape::image() const
{
	return d->m_frame->image();
}

void
FrameOnTape::setImagePos( qsizetype pos )
{
	d->m_frame->setImagePos( pos );
}

void
FrameOnTape::clearImage()
{
	d->m_frame->clearImage();
}

void
FrameOnTape::applyImage()
{
	d->m_frame->applyImage();
}

bool
FrameOnTape::isChecked() const
{
	return d->m_checkBox->isChecked();
}

void
FrameOnTape::setChecked( bool on )
{
	d->m_checkBox->setChecked( on );
}

int
FrameOnTape::counter() const
{
	return d->m_counter;
}

void
FrameOnTape::setCounter( int c )
{
	d->m_counter = c;

	d->m_label->setText( tr( "#%1" ).arg( c ) );
}

bool
FrameOnTape::isCurrent() const
{
	return d->m_current;
}

void
FrameOnTape::setCurrent( bool on )
{
	d->setCurrent( on );
}

void
FrameOnTape::contextMenuEvent( QContextMenuEvent * e )
{
	QMenu menu( this );

	if( !d->m_frame->image().m_isEmpty )
	{
		menu.addAction( QIcon( QStringLiteral( ":/img/document-save-as.png" ) ),
			tr( "Save this frame" ),
			[this] ()
			{
				auto fileName = QFileDialog::getSaveFileName( this,
					tr( "Choose file to save to..." ), QString(), tr( "PNG (*.png)" ) );

				if( !fileName.isEmpty() )
				{
					if( !fileName.endsWith( QStringLiteral( ".png" ), Qt::CaseInsensitive ) )
						fileName.append( QStringLiteral( ".png" ) );

					const auto img = this->d->m_frame->image().m_gif.at(
						this->d->m_frame->image().m_pos );
					img.save( fileName );
				}
			} );

		menu.addSeparator();
	}

	menu.addAction( QIcon( QStringLiteral( ":/img/list-remove.png" ) ),
		tr( "Uncheck till end" ),
		[this] () { emit this->checkTillEnd( this->d->m_counter, false ); } );

	menu.addAction( QIcon( QStringLiteral( ":/img/list-add.png" ) ),
		tr( "Check till end" ),
		[this] () { emit this->checkTillEnd( this->d->m_counter, true ); } );

	menu.exec( e->globalPos() );
}
