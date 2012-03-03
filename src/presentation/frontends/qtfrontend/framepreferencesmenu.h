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
#ifndef FRAMEPREFERENCESMENU_H
#define FRAMEPREFERENCESMENU_H

#include "menuframe.h"
#include "src/application/soundhandler.h"

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>


/**
 * A customized GUI menu class for the fram preferences menu.
 *
 * Inherits from MenuFrame but adds some functionality around loading the
 * present preferences when opened.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FramePreferencesMenu : public MenuFrame
{
	Q_OBJECT
public:
	
	/**
	 * Creates and sets up the preferences menu. 
	 * @param parent the parent widget
	 * @param soundHandler the sound handler
	 * @param name the name of the menu
	 */
	FramePreferencesMenu( QWidget * parent = 0, SoundHandler *soundHandler = 0, 
			const char * name = 0);
	
	/**
	 * Retranslates the strings in the frame preferences menu.
	 */
	void retranslateStrings();
	
private:
	SoundHandler *soundHandler;
	QListBox *soundsList;
	QLabel *soundsLabel;
	QPushButton *closeButton;
	QPushButton *addSoundButton;
	QPushButton *removeSoundsButton;
	QPushButton *changeNameButton;
	QGridLayout *grid;
	QSpacerItem *spacer;
	QSpacerItem *space;
	
public slots:
	/**
	 * Opens the menu.
	 */
	void open();
	
	/**
	 * Closes the menu and turns the focus to the newFocusWidget.
	 */
	void close();
};

#endif
