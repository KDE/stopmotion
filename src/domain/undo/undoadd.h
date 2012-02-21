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
#ifndef UNDOADD_H
#define UNDOADD_H

#include "undo.h"

#include <vector>


/**
 *The UndoAdd class for undoing addFrames(...) calls to the domain.
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class UndoAdd : public Undo
{
public:

	/**
	 * Sets up the UndoAdd command object with the information needed to undo and
	 * redo the add command.
	 * @param fromIndex the index of the first frame added. Used for undoing the 
	 * command
	 * @param frameNames the name of the frame added. Used for redoing the command.
	 * @param activeScene the scene the frames were added to.
	 */
	UndoAdd(int fromIndex,  const vector<char*>& frameNames, int activeScene);
	
	virtual ~UndoAdd();
	
	/**
	 * Abstract function for undoing the command represented by this undo object.
	 * @param a the model to perform the undo command on.
	 */
	void undo(AnimationModel *a);
	
	/**
	 * Abstract function for redoing (performing) the command represented by this 
	 *undo object.
	 * @param a the model to perform the redo command on.
	 */
	void redo(AnimationModel *a);
	
private:
	vector<char*> frameNames;
	int fromIndex;
	int activeScene;
};

#endif
