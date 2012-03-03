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
#ifndef UNDO_H
#define UNDO_H

#include "src/domain/animation/animationmodel.h"


/**
 *Superclass for all undo command objects. 
 *The undo objects can be called upon to undo operations the user has done.
 *
 *The undo command objects work directly on the model, not through the domain
 *DomainFacade, or we would get undos from the undo commands
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class Undo
{
public:
	virtual ~Undo() {}
	
	/**
	 * Abstract function for undoing the command represented by this undo object.
	 * @param a the model to perform the undo command on.
	 */
	virtual void undo(AnimationModel *a) = 0;
	
	/**
	 * Abstract function for redoing (performing) the command represented by this 
	 *undo object.
	 * @param a the model to perform the redo command on.
	 */
	virtual void redo(AnimationModel *a) = 0;
};

#endif
