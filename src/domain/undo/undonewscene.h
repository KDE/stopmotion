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
#ifndef UNDONEWSCENE_H
#define UNDONEWSCENE_H

#include "undo.h"


/**
 * The UndoNewScene class for undoing newScene(...) calls to the domain.
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class UndoNewScene : public Undo
{
public:

	/**
	 * Sets up the UndoNewScene command object with the information needed to undo and
	 * redo the add command.
	 * @param sceneNumber the index of the new scene.
	 */
	UndoNewScene(int sceneNumber);
	
	/**
	 * Cleans up after the undo object.
	 */
	virtual ~UndoNewScene();
	
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
	int sceneNumber;	
};

#endif
