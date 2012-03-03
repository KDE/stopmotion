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
#include "animationmodel.h"

#include "src/foundation/logger.h"


AnimationModel::AnimationModel()
{
	frontend = NULL;
}


AnimationModel::~AnimationModel()
{
	frontend = NULL;
}


void AnimationModel::attatch(Observer *o)
{
	if(o != NULL) {
		observers.push_back(o);
	}
	else {
		Logger::get().logDebug("Trying to attatch a NULL pointer to the AnimationModel");
	}
}


void AnimationModel::detatch(Observer *o)
{
	//Sequential search for the observer. Complexity: O(N/2) on the average.
	for(unsigned int i = 0; i < observers.size(); i++) {
		if (observers[i] == o) {
			observers.erase(observers.begin() + i);
			break;
		}
	}
}


void AnimationModel::registerFrontend(Frontend *frontend)
{
	if(this->frontend != NULL) {
		delete this->frontend;
	}
	this->frontend = frontend;
}


Frontend* AnimationModel::getFrontend()
{
	if (frontend) {
		return frontend;
	}
	return NULL;
}


void AnimationModel::notifyAdd(const vector<char*>& frames, unsigned int index)
{
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateAdd(frames, index, frontend);
	}
}


void AnimationModel::notifyRemove(unsigned int fromFrame, unsigned int toFrame)
{
	Logger::get().logDebug("Notifying the observers of the removal of the frame");
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateRemove(fromFrame, toFrame);
	}
}


void AnimationModel::notifyMove(unsigned int fromFrame, unsigned int toFrame, 
								unsigned int movePosition)
{
	Logger::get().logDebug("Notifying the observers of the moving of the frame");
	
	unsigned int numElem = observers.size();
	for(unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateMove(fromFrame, toFrame, movePosition);
	}
}


void AnimationModel::notifyNewActiveFrame(int frameNumber)
{
	Logger::get().logDebug("Notifying observers about new active frame");
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateNewActiveFrame(frameNumber);
	}
}


void AnimationModel::notifyClear()
{
	Logger::get().logDebug("Notifying observers that the model is erased");
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateClear();
	}
}


void AnimationModel::notifyPlayFrame(int frameNumber)
{
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updatePlayFrame(frameNumber);
	}
}


void AnimationModel::notifyNewScene( int index )
{
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateNewScene(index);
	}
}


void AnimationModel::notifyRemoveScene( int sceneNumber )
{
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateRemoveScene(sceneNumber);
	}
}


void AnimationModel::notifyMoveScene( int sceneNumber, int movePosition )
{
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateMoveScene(sceneNumber, movePosition);
	}
}


void AnimationModel::notifyNewActiveScene( int sceneNumber, vector<char*> framePaths,
		Frontend *frontend )
{
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateNewActiveScene(sceneNumber, framePaths, frontend);
	}
}

void AnimationModel::notifyAnimationChanged(int frameNumber)
{
	unsigned int numElem = observers.size();
	for (unsigned int i = 0; i < numElem; i++) {
		observers[i]->updateAnimationChanged(frameNumber);
	}
}
