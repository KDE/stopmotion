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
#ifndef UNDOHISTORY_H
#define UNDOHISTORY_H

#include "undo.h"

#include <vector>

using namespace std;


/**
 *Factory for storing, handling and retrieving undos.
 *
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class UndoHistory
{
public:
	
	UndoHistory();
	
	~UndoHistory();
	
	/**
	 * Add an undo command object to the list of undoable commands.
	 * @param undo the undo command to add to the list of undoable commands.
	 */
	void addUndo (Undo *undo);

	/**
	 * Calls undo on the current undo in the history.
	 * @param a the animationmodel the undo operation is performed on.
	 * @return true if there is no more undoes to undo in the container
	 */
	bool undo(AnimationModel *a);
	
	/**
	 * Calls redo on the next undo in the history
	 * @param a the animationmodel the redo operation is performed on.
	 * @return true if there are no more undos to redo in the container.
	 */
	bool redo(AnimationModel *a);
	
	/**
	 * Clears the undo history.
	 */
	void clear();

private:
	vector<Undo*> undos;
	unsigned int currentUndo;
	static const unsigned int MAX_UNDOES = 200;
};

#endif
