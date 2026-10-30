/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
