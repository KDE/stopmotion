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
#include "undomove.h"


UndoMove::UndoMove(unsigned int fromFrame, unsigned int toFrame, 
		unsigned int movePosition, int activeScene)
		: fromFrame(fromFrame), toFrame(toFrame), movePosition(movePosition), 
		activeScene(activeScene)
{
}


void UndoMove::undo(AnimationModel *a)
{
	//Setting the active scene to the scene which the frames were originally added to.
	a->setActiveScene(activeScene);
	
	if (movePosition < fromFrame) {
		a->moveFrames(movePosition, movePosition+(toFrame-fromFrame), toFrame);
	}
	else {
		a->moveFrames(movePosition-(toFrame-fromFrame), movePosition, fromFrame);
	}
}


void UndoMove::redo(AnimationModel *a)
{
	//Setting the active scene to the scene which the frames were originally added to.
	a->setActiveScene(activeScene);
	a->moveFrames(fromFrame, toFrame, movePosition);
}
