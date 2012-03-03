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
#ifndef UNDOMOVE_H
#define UNDOMOVE_H

#include "undo.h"


/**
 *Superclass for all undo command objects. 
 *The undo objects can be called upon to undo operations the user has done.
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class UndoMove : public Undo
{
public:

	/**
	 * Sets up the UndoAdd command object with the information needed to undo and
	 * redo the add commands.
	 * @param fromFrame the first of the frames which were moved.
	 * @param toFrame the last of the frames which were moved.
	 * @param movePosition the position the frames was moved to.
	 * @param activeScene the scene the frames were moved within.
	 */
	UndoMove(unsigned int fromFrame, unsigned int toFrame, 
		unsigned int movePosition, int activeScene);
	
	
	/**
	 * Abstract function for undoing the command represented by this undo object.
	 * @param a the model to perform the undo command on.
	 */
	void undo(AnimationModel *a);
	
	/**
	 * Abstract function for redoing (performing) the command represented by this 
	 * undo object.
	 * @param a the model to perform the redo command on.
	 */
	void redo(AnimationModel *a);
	
private:
	unsigned int fromFrame, toFrame, movePosition;
	int activeScene;
};

#endif
