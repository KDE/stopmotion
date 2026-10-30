/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "menuframe.h"

#include <cstddef>

#include <QMouseEvent>


MenuFrame::MenuFrame(QWidget * parent, const char * name)
	: QFrame(parent)
{
	focusSpinBox = nullptr;
	focusLineEdit = nullptr;
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
	if (focusSpinBox != nullptr) {
		focusSpinBox->setFocus();
		focusSpinBox->selectAll();
	}
	else if (focusLineEdit != nullptr) {
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
