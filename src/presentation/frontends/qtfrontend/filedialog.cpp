/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "filedialog.h"

#include "graphics/icons/home.xpm"

#include <qtoolbutton.h>
#include <qpixmap.h>
#include <qtooltip.h>


FileDialog::FileDialog( const QString & dirName, const QString & filter, 
		QWidget * parent, const char * name, bool modal )
		: QFileDialog(dirName, filter, parent, name, modal)
{
	QToolButton *button = new QToolButton(this);
	button->setPixmap( QPixmap(home) );
	QToolTip::add( button, tr( "Go to home directory" ) );
	
	connect( button, SIGNAL( clicked() ), this, SLOT( goToHomeDir() ) );
	this->addToolButton(button);
}


FileDialog::FileDialog( QWidget * parent, const char * name, bool modal )
		: QFileDialog(parent, name, modal)
{
	QToolButton *button = new QToolButton(this);
	button->setPixmap( QPixmap(home) );
	QToolTip::add( button, tr( "Go to home directory" ) );
	
	connect( button, SIGNAL( clicked() ), this, SLOT( goToHomeDir() ) );
	this->addToolButton(button);
}


void FileDialog::goToHomeDir()
{
	setDir( getenv("HOME") );
}
