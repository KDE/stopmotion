/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FLEXIBLELINEEDIT_H
#define FLEXIBLELINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>


/**
 * An extended line edit class.
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FlexibleLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	/**
	 * Constructs and initializes the line edit object.
	 * @param parent the parent widget
	 */
	FlexibleLineEdit(QWidget *parent = nullptr);
	
	/**
	 * Checks what kind of key event the user has requested and 
	 * does actions based on this. 
	 * @param k the key event
	 */
	void keyPressEvent( QKeyEvent * k ) override;
	
signals:
	/**
	 * Emitted if editing was canceled.
	 */
	void lineEditCanceled();
};

#endif
