/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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
#ifndef EDITMENUHANDLER_H
#define EDITMENUHANDLER_H

#include "src/config.h"

#include "src/presentation/frontends/qtfrontend/menuframe.h"
#include "src/presentation/frontends/qtfrontend/framebar/framebar.h"
#include "modelhandler.h"

#include <qobject.h>
#include <qstatusbar.h>


/**
 * This class handles request related to the editmenu (undo, redo, copy, cut, etc)
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class EditMenuHandler : public QObject
{
	Q_OBJECT
public:
	/**
	 * Creates and initializes the Soundhandler.
	 * @param parent the parent of the soundhandler.
	 * @param sb a statusbar for displaying status information to the user.
	 * @param frameBar the frameBar containing thumbnails
	 * @param name the name of the soundhandler
	 */
	EditMenuHandler( QObject *parent = 0, QStatusBar *sb = 0, FrameBar *frameBar = 0, 
			const char *name = 0 );
	
	
	/**
	 * Sets the gotoMenu property so that it can be closed.
	 * @param gotoMenu the gotoMenu.
	 */
	void setGotoMenu(MenuFrame *gotoMenu);

public slots:
	
	/**
	 * Goes to the frame with the number frameNumber on the framebar.
	 * @param frameNumber the number of the frame to go to.
	 */
	void gotoFrame(int frameNumber);
	
	/**
	 * Closes the goto menu and moves the focus to the mainwindowgui.
	 */
	void closeGotoMenu();
	
	/**
	 * Undoes the last undoable operation the user have done.
	 */
	void undo();
	
	/**
	 * Redoes the last undo the user have done.
	 */
	void redo();
	
	/**
	 * Copies the selection to the global clipboard and removes it from
	 * the project.
	 */
	void cut();
	
	/**
	 * Copies a selection to the global clipboard.
	 */
	void copy();
	
	/**
	 * Pastes a selection from the global clipboard to the program.
	 */
	void paste();
	
private:
	QStatusBar *statusBar;
	FrameBar *frameBar;
	MenuFrame *gotoMenu;
	
signals:
	/**
	 * Adds the frames pointed to by a list.
	 * @param l the list containing pointers to the frames
	 */
	void addFrames(const QStringList &l);
};

#endif
