
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
#ifndef MENUFRAME_H
#define MENUFRAME_H

#include <QFrame>
#include <QSpinBox>
#include <QLineEdit>
#include <QMouseEvent>

/**
 * A customized GUI menu class for embedded menues which shouldn't be visible 
 * at all times. The menues are hidden and pops up at various places in the
 * gui interface. 
 *
 * This class allow you to create preferences/etc. menues without resorting to
 * the modal cover-the-gui menues.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class MenuFrame : public QFrame
{
	Q_OBJECT
public:
	/**
	 * Creates and sets up the menu frame. 
	 *
	 * @param parent the parent widget.
	 * @param name the name of the menu.
	 */
	MenuFrame(QWidget * parent = 0, const char * name = 0);
			
	/**
	 * Sets the focus widget to the QSpinBox: focusSpinBox.
	 *
	 * NB: One should only use use one of the two setFocusWidget functions. If both
	 *     are used the spinbox will be used. (A bit hacky but saves me from creating
	 *     a custom(/messy!) class hierarchy and gives me time to go out and have  
	 *     a few beers tonight).
	 *
	 * @param focusSpinBox the focuswidget of this menu.
	 */
	void setFocusWidget(QSpinBox * focusSpinBox = 0);
	
	
	/**
	 * Sets the focus widget to the QLineEdit: focusSpinBox.
	 *
	 * NB: One should only use use one of the two setFocusWidget functions. If both
	 *     are used the spinbox will be used. (A bit hacky but saves me from creating
	 *     a custom(/messy!) class hierarchy and gives me time to go out and have  
	 *     a few beers tonight).
	 *
	 * @param focusLineEdit the focuswidget of this menu.
	 */
	void setFocusWidget(QLineEdit * focusLineEdit = 0);
	
protected:
	/**
	 * Overloaded function to intercept when the user press the mouse inside 
	 * the menu. (So that the mainwindow doesn't get the event).
	 * @param e information about the event.
	 */
	void mousePressEvent( QMouseEvent * e);
	
private:
	QSpinBox *focusSpinBox;
	QLineEdit *focusLineEdit;
	
public slots:
	/**
	 * Opens the menu.
	 */
	virtual void open();
	
	/**
	 * Closes the menu and turns the focus to the newFocusWidget.
	 * @param newFocusWidget the widget who will receive the focus after this menu
	 * closes.
	 */
	virtual void close(QWidget * newFocusWidget);
};

#endif
