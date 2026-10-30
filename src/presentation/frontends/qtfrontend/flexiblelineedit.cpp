/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "flexiblelineedit.h"
#include <QKeyEvent>



FlexibleLineEdit::FlexibleLineEdit(QWidget *parent)
		: QLineEdit(parent)
{
}


void FlexibleLineEdit::keyPressEvent( QKeyEvent * k ) 
{
	switch ( k->key() ) 
	{
		case Qt::Key_Return: case Qt::Key_Enter:
		{
			emit returnPressed();
			break;
		}
		case Qt::Key_Escape:
		{
			emit lineEditCanceled();
			k->ignore();
			break;
		}
		default:
		{
			QLineEdit::keyPressEvent(k);
			k->ignore();
			break;
		}
	}
	
}
