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
#include "src/presentation/frontends/qtfrontend/flexiblespinbox.h"

#include <QKeyEvent>


FlexibleSpinBox::FlexibleSpinBox(QWidget *parent)
	: QSpinBox(parent)
{
}


void FlexibleSpinBox::setMaximumValue(int maxValue)
{
	this->setMaximum(maxValue);
	if (maxValue > 0) {
		setEnabled(true);
	}
	else {
		setEnabled(false);
	}
}


void FlexibleSpinBox::keyPressEvent( QKeyEvent * k ) 
{
	switch ( k->key() ) 
	{
		case Qt::Key_Return: case Qt::Key_Enter:
		{
			emit spinBoxTriggered(this->value()-1);
			break;
		}
		case Qt::Key_Escape:
		{
			emit spinBoxCanceled();
			break;
		}
		default:
		{
			QSpinBox::keyPressEvent(k);
			break;
		}
	}
	
}
	

void FlexibleSpinBox::showEvent(QShowEvent *)
{
	this->setFocus();
	this->selectAll();
}

void FlexibleSpinBox::setIndexSecond(int, int index) {
	setValue(index + 1);
}
