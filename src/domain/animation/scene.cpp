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
#include "scene.h"
#include <sstream>
#include <cstring>
#include <unistd.h>

Scene::Scene()
{
	
}


Scene::~Scene()
{
	unsigned int numElem = frames.size();
	for (unsigned int i = 0; i < numElem; ++i) {
		delete frames[i];
	}
}


vector<Frame*>& Scene::getFrames()
{
	return frames;
}


unsigned int Scene::getSize()
{
	return frames.size();
}


Frame* Scene::getFrame(unsigned int frameNumber)
{
	return frames[frameNumber];
}


const vector<char*> Scene::addFrames(const vector<char*>& frameNames, unsigned int index,
		Frontend *frontend, unsigned int &numberOfCanceledFrames)
{
	Logger::get().logDebug("Adding frames in Animation:");
	
	vector<char*> newImagePaths;
	bool isImportingAborted = false;
	
	unsigned i = 0;
	unsigned numElem = frameNames.size();
	
	if (frameNames.size() == 1) {
		char* file = frameNames[i];
		if (access(file, R_OK) == 0) {
			newImagePaths.push_back( addFrame(file, index) );
		}
		else {
			frontend->reportError("You do not have permission to read that file", 0);
			return newImagePaths;
		}
	}	
	else {
		frontend->showProgress("Importing frames from disk ...", numElem * 2);
		
		unsigned numNotReadable = 0;
		for (; i < numElem; ++i) {
			frontend->updateProgress(i);
			char* file = frameNames[i];
			if (access(file, R_OK) == 0) {
				newImagePaths.push_back( addFrame(frameNames[i], index) );
			}
			else {
				Logger::get().logWarning("Wrong permission:");
				Logger::get().logWarning(file);
				++numNotReadable;	
			}
			
			// Doesn't want to process events for each new frame added.
			if ( (i % 10) == 0) {
				frontend->processEvents();
			}
			if ( frontend->isOperationAborted() ) {
				isImportingAborted = true;
				++i; // does this to make cleaning of frames general
				break;
			}
		}
		frontend->updateProgress(numElem);
		
		if (isImportingAborted) {
			numberOfCanceledFrames = i;
			newImagePaths.clear();
		}
		else {
			numberOfCanceledFrames = numElem;
		}

		if (numNotReadable > 0) {
			stringstream ss;
			ss << "You do not have permission to read " << numNotReadable << " file(s)";
			frontend->reportError(ss.str().c_str(), 0);
		}
	}
	
	// trim to size :)
	vector<char*>(newImagePaths).swap(newImagePaths);
	return newImagePaths;
}


void Scene::cleanFrames(unsigned int fromFrame, unsigned int toFrame)
{
	frames.erase(frames.begin() + fromFrame, frames.begin() + toFrame);
}


char* Scene::addFrame(char* frameName, unsigned int &index)
{
	char *newPath;
	
	Frame *f = new Frame(frameName);
	f->copyToTemp();
	newPath = new char[256];
	strcpy( newPath, f->getImagePath() );
	frames.insert(frames.begin() + index, f);
	++index;
	
	return newPath;
}


void Scene::addSavedFrame(Frame *f)
{
	frames.push_back(f);
}


const vector< char * > Scene::removeFrames( unsigned int fromFrame, const unsigned int toFrame )
{
	vector<char*> newImagePaths;
	char *newPath;
	
	if ( toFrame < frames.size() ) { 
		for (unsigned int i = fromFrame; i <= toFrame; ++i) {
			Frame *f = frames[fromFrame];
			f->moveToTrash();
			newPath = new char[256];
			strcpy(newPath, f->getImagePath());
			newImagePaths.push_back(newPath);
			newPath = NULL;
			delete frames[fromFrame];
			frames.erase(frames.begin() + fromFrame);
		}
	}
	
	// trim to size :)
	vector<char*>(newImagePaths).swap(newImagePaths);
	return newImagePaths;
}


void Scene::moveFrames( unsigned int fromFrame, unsigned int toFrame, 
		unsigned int movePosition )
{
	if (movePosition < fromFrame) {
		for (unsigned int i = fromFrame, j = movePosition; i <= toFrame; ++i, ++j) {
			Frame *f = frames[i];
			frames.erase(frames.begin() + i);
			frames.insert(frames.begin() + j, f);
		}
	}
	else {
		for (unsigned int i = fromFrame; i <= toFrame; ++i) {
			Frame *f = frames[fromFrame];
			frames.erase(frames.begin() + fromFrame);
			frames.insert(frames.begin() + movePosition, f);
		}
	}
}


int Scene::addSound( unsigned int frameNumber, const char *sound )
{
	Logger::get().logDebug("Adding sound in scene");
	return frames[frameNumber]->addSound(sound);
}


void Scene::removeSound( unsigned int frameNumber, unsigned int soundNumber )
{
	frames[frameNumber]->removeSound(soundNumber);
}


void Scene::setSoundName( unsigned int frameNumber, unsigned int soundNumber, 
		char * soundName )
{
	frames[frameNumber]->setSoundName(soundNumber, soundName);
}


vector<char *> Scene::getImagePaths( )
{
	vector<char*> imagePaths;
	unsigned int size = frames.size();
	for (unsigned int i = 0; i < size; ++i) {
		imagePaths.push_back(frames[i]->getImagePath());
	}
	return imagePaths;
}

