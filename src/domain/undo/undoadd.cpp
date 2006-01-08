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
#include "undoadd.h"



UndoAdd::UndoAdd(int fromIndex, const vector<char*>& frameNames, int activeScene)
		: fromIndex(fromIndex), activeScene(activeScene)
{
	//Performing a deep-copy
	unsigned int numElem = frameNames.size();
	for (unsigned int i = 0; i < numElem; i++) {
		this->frameNames.push_back(frameNames[i]);
	}
}


UndoAdd::~UndoAdd()
{
	unsigned int numElem = frameNames.size();
	for (unsigned int i = 0; i < numElem; i++) {
		delete [] frameNames[i];
		frameNames[i] = NULL;
	}
}


void UndoAdd::undo(AnimationModel *a)
{
	//Setting the active scene to the scene which the frames were originally added to.
	a->setActiveScene(activeScene);
	
	// Give animation model order to remove frames in the range 'fromIndex -> end of vector'.
	vector<char*> newImagePaths = a->removeFrames(fromIndex, fromIndex + 
			frameNames.size() - 1 );

	// Deallocates the old allocated image paths.
	unsigned int numElem = frameNames.size();
	for (unsigned int i = 0; i < numElem; i++) {
		delete [] frameNames[i];
		frameNames[i] = NULL;
	}
	
	// Setting new image paths.
	numElem = newImagePaths.size();
	for (unsigned int i = 0; i < numElem; i++) {
		frameNames[i] = newImagePaths[i];
	}
}


void UndoAdd::redo(AnimationModel *a)
{
	//Setting the active scene to the scene which the frames were originally added to.
	a->setActiveScene(activeScene);
	
	vector<char*> newImagePaths = a->addFrames(frameNames, fromIndex);
	
	// Deallocates the old allocated image paths.
	unsigned int numElem = frameNames.size();
	for (unsigned int i = 0; i < numElem; i++) {
		delete [] frameNames[i];
		frameNames[i] = NULL;
	}
	
	// Setting new image paths.
	numElem = newImagePaths.size();
	for (unsigned int i = 0; i < numElem; i++) {
		frameNames[i] = newImagePaths[i];
	}
}
