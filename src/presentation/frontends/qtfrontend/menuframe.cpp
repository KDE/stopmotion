/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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
#include "menuframe.h"

#include <stddef.h>

#include <QMouseEvent>


MenuFrame::MenuFrame(QWidget * parent, const char * name)
	: QFrame(parent)
{
	focusSpinBox = NULL;
	focusLineEdit = NULL;
	setObjectName(name);
}


void MenuFrame::mousePressEvent( QMouseEvent * )
{
}


void MenuFrame::setFocusWidget( QSpinBox * focusSpinBox )
{
	this->focusSpinBox = focusSpinBox;
}


void MenuFrame::setFocusWidget( QLineEdit * focusLineEdit )
{
	this->focusLineEdit = focusLineEdit;
}


void MenuFrame::open( )
{
	if (focusSpinBox != 0) {
		focusSpinBox->setFocus();
		focusSpinBox->selectAll();
	}
	else if (focusLineEdit != 0) {
		focusLineEdit->setFocus();
		focusLineEdit->selectAll();
	}
	
	this->show();
}


void MenuFrame::close( QWidget * newFocusWidget )
{
	this->hide();
	newFocusWidget->setFocus();
}
