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
#ifndef SOUNDHANDLER_H
#define SOUNDHANDLER_H

#include "src/config.h"

#include <QObject>
#include <QStatusBar>
#include <QListWidget>

class FrameBar;

/**
 * This class handles request related to sounds (adding, removing, etc).
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class SoundHandler : public QObject {
	Q_OBJECT
public:
	/**
	 * Creates and initializes the Soundhandler.
	 * @param parent the parent of the soundhandler.
	 * @param sb a statusbar for displaying status information to the user.
	 * @param homeDir the home directory of the user.
	 * @param name the name of the soundhandler
	 */
	SoundHandler ( QObject *parent = 0, QStatusBar *sb = 0,
			const FrameBar* frameBar = 0, const char* homeDir = 0,
			const char *name = 0 );
	
	/**
	 * Add the soundsList to the handler so that it can be cleared and filled as
	 * needed.
	 * @param soundsList the listbox to add to the handler.
	 */
	//void setSoundsList(Q3ListBox *soundsList);
	void setSoundsList(QListWidget *soundsList);
	
public slots:

	/**
	 * Attaches a sound to a frame in the animation.
	 */
	void addSound();
	
	/**
	 * Removes a sound from a frame in the animation.
	 */
	void removeSound();
	
	/**
	 * Sets the name of a sound in a frame in the animation.
	 */
	void setSoundName();

private:
	QStatusBar *statusBar;
	const FrameBar* frameBar;
	const char* homeDir;
	//Q3ListBox *soundsList;
	QListWidget *soundsList;
};

#endif
