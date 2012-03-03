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
#include "animation.h"

#include "src/foundation/logger.h"
#include "src/technical/audio/ossdriver.h"
#include "src/technical/video/videofactory.h"

#include <vector>
#include <qstring.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <qapplication.h>

#include <iostream>

Animation::Animation()
{
	serializer  = new ProjectSerializer();
	audioDriver = new OSSDriver("/dev/dsp");
// 	externalChangeMonitor = new ExternalChangeMonitor(this);
// 	externalChangeMonitor->startMonitoring();
	
	activeFrame = -1;
	activeScene = -1;
	
	isChangesSaved = true;
	isAudioDriverInitialized = false;
}


Animation::~Animation()
{
	unsigned int numElem = scenes.size();
	for(unsigned int i = 0; i < numElem; i++) {
		delete scenes[i];
	}
	
	serializer->cleanup();
	delete serializer;
	serializer = NULL;
	delete audioDriver;
	audioDriver = NULL;
}


const vector<char*> Animation::addFrames(const vector<char*>& frameNames, 
		unsigned int index)
{
	if(this->getActiveSceneNumber() < 0) {
		this->newScene(0);
	}
	
	bool isAddingAborted = false;
	vector<char*> newImagePaths;
	unsigned int numberOfCanceledFrames = 0;
	newImagePaths = scenes[activeScene]->addFrames(frameNames, index, frontend, numberOfCanceledFrames);
	
	unsigned int newImagePathsSize = newImagePaths.size();
		
	if(newImagePathsSize == 1) {
		this->notifyAdd(newImagePaths, index );
	}
	else if( newImagePathsSize > 1 ) { 
		this->notifyAdd(newImagePaths, index );
		isAddingAborted = frontend->isOperationAborted();
	}
	
	// The user has aborted the operation either when importing frames
	// or adding frames. The cleanup routine is equal for both cases, the
	// only difference is the number of frames added to the 'frames' vector.
	if( newImagePathsSize == 0 || isAddingAborted ) {
		scenes[activeScene]->cleanFrames(index, index + numberOfCanceledFrames);
		newImagePaths.clear();
	}
	else {
		this->isChangesSaved = false;
	}
	
	if(newImagePathsSize != 1) {
		frontend->updateProgress(frameNames.size() * 2);
		frontend->hideProgress();
	}
	
	if( newImagePathsSize == 1 ) {
		this->setActiveFrame(index );
	}
	else if( newImagePathsSize > 1 && !isAddingAborted ) {
		this->setActiveFrame(index + newImagePathsSize - 1);
	}
	
	return newImagePaths;
}


const vector<char*> Animation::removeFrames(unsigned int fromFrame, unsigned int toFrame )
{
	assert(fromFrame <= toFrame);
	
	vector<char*> newImagePaths;
	
	if( activeFrame >= 0) {
		newImagePaths = scenes[activeScene]->removeFrames(fromFrame, toFrame);
		
		this->notifyRemove(fromFrame, toFrame);
		
		if( toFrame < scenes[activeScene]->getSize() ) {
			this->setActiveFrame(toFrame);
		}
		else {
			this->setActiveFrame(scenes[activeScene]->getSize()-1);
		}
		isChangesSaved = false;
	}
	
	return newImagePaths;
}


void Animation::moveFrames(unsigned int fromFrame, unsigned int toFrame, 
		unsigned int movePosition)
{
	unsigned int framesSize = scenes[activeScene]->getSize();
	if( (fromFrame < framesSize) && (toFrame < framesSize) && (movePosition < framesSize) && 
			((movePosition < fromFrame) || (movePosition > toFrame)) ) {
		
		scenes[activeScene]->moveFrames(fromFrame, toFrame, movePosition);
		
		isChangesSaved = false;
		this->notifyMove(fromFrame, toFrame, movePosition);
		this->setActiveFrame(movePosition);
	}
}


int Animation::addSound(unsigned int frameNumber, const char *sound)
{
	Logger::get().logDebug("Adding sound in animation");
	int ret = scenes[activeScene]->addSound(frameNumber, sound);
	if (ret == -1) {
		frontend->reportError(
				"Cannot open the selected audio file for reading.\n"
				"Check that you have the right permissions set.\n"
				"The animation will be runned without sound if you\n"
				"choose to play.", 0);
	}
	else if (ret == -2) {
		frontend->reportError(
				"The selected audio file is not valid within the\n" 
				"given audio format. The animation will be runned\n"
				"without sound if you choose to play.", 0);
	}
	return ret;
}


void Animation::removeSound( unsigned int frameNumber, unsigned int soundNumber )
{
	scenes[activeScene]->removeSound(frameNumber, soundNumber);
}


void Animation::setSoundName( unsigned int frameNumber, unsigned int soundNumber, char * soundName )
{
	scenes[activeScene]->setSoundName(frameNumber, soundNumber, soundName);
}


Frame* Animation::getFrame(unsigned int frameNumber, unsigned int sceneNumber)
{
	if(frameNumber < scenes[sceneNumber]->getSize()) {
		Logger::get().logDebug("Retrieving frame from Animation");
		
		return scenes[sceneNumber]->getFrame(frameNumber);
	}
	else {
		Logger::get().logWarning("Requesting a frame which is not " 
				"in the animation (framenumber out of bounds)");
		return NULL;
	}
}


Frame * Animation::getFrame( unsigned int frameNumber )
{
	if(activeScene >= 0) {
		return getFrame(frameNumber, activeScene);
	}
	else {
		return NULL;
	}
}


unsigned int Animation::getModelSize() 
{
	//return scenes[activeScene]->getSize();
	unsigned int modelSize = 0;
	unsigned int size = scenes.size();
	for(unsigned int i=0; i<size; i++) {
		modelSize += scenes[i]->getSize();
	}
	return modelSize;
}


unsigned int Animation::getSceneSize( int sceneNumber )
{
	return scenes[sceneNumber]->getSize();
}


unsigned int Animation::getNumberOfScenes( )
{
	return scenes.size();
}


void Animation::setActiveFrame(int frameNumber)
{
	if(activeScene >= 0 || frameNumber == -1) {
		this->activeFrame = frameNumber;
		this->notifyNewActiveFrame(frameNumber);
	}
}


void Animation::playFrame(int frameNumber)
{
	if (isAudioDriverInitialized) {
		Frame *f = getFrame(frameNumber);
		f->playSounds(audioDriver);
	}
	notifyPlayFrame(frameNumber);
}


int Animation::getActiveFrameNumber()
{
	return activeFrame;
}


const char* Animation::getProjectFile()
{
	return serializer->getProjectFile();
}


void Animation::clear()
{
	serializer->cleanup();
	
	unsigned int size = scenes.size();
	for(unsigned int i=0; i<size; i++) {
		delete scenes[i];
	}
	scenes.clear();
	
	activeFrame = -1;
	activeScene = -1;
	isChangesSaved = true;
	Frame::tmpNum = 0;
	Frame::trashNum = 0;
	notifyClear();
}


bool Animation::openProject(const char *filename)
{
	clear();
	assert(filename != 0);
	scenes = serializer->open(filename);
	if (scenes.size() > 0) {
		loadSavedScenes();
// 		externalChangeMonitor->changeWorkDirectory( serializer->getProjectPath() );
		return true;
	}
	return false;
}


bool Animation::saveProject(const char *filename)
{	
	assert(filename != 0);
	bool tmp = serializer->save(filename, scenes, frontend);
	
	if(tmp) {
//  		externalChangeMonitor->changeWorkDirectory( serializer->getProjectPath() );
	}
	
	return tmp;
}


bool Animation::newProject()
{
	clear();
	// do something
	return true;
}


void Animation::loadSavedScenes()
{
	Logger::get().logDebug("Loading scenes in Animation:");
	
	unsigned int numElem = scenes.size();
	for (unsigned int i = 0; i < numElem; i++) {
		notifyNewScene(i);
	}
	setActiveScene(numElem - 1);
}


bool Animation::isUnsavedChanges()
{
	return !isChangesSaved;
}


void Animation::setActiveScene( int sceneNumber)
{
	if(sceneNumber != activeScene) {
		activateScene(sceneNumber);
	}
}


void Animation::activateScene( int sceneNumber )
{
	this->activeScene = sceneNumber;
	if(sceneNumber >= 0) {
		if(scenes[sceneNumber]->getSize() > 0) {
			this->notifyNewActiveScene(sceneNumber, scenes[sceneNumber]->getImagePaths(),
				frontend);
			setActiveFrame(0);
		}
		else {
			vector<char*> dummy;
			this->notifyNewActiveScene(sceneNumber, dummy,
				frontend);
			setActiveFrame(-1);
		}
	}
	else {
		setActiveFrame(-1);
		vector<char*> dummy;
		this->notifyNewActiveScene(sceneNumber, dummy,
			frontend);
	}
}


void Animation::newScene( int index )
{
	Scene *s = new Scene();
	scenes.insert(scenes.begin() + index, s);
	
	this->notifyNewScene( index );
	
	this->setActiveScene( index );
}


void Animation::removeScene( int sceneNumber )
{
	if(sceneNumber >= 0) {
		if(sceneNumber < (int)scenes.size()-1 ) {
			activateScene( sceneNumber + 1 );
			this->activeScene = sceneNumber;
		}
		else {
			activateScene( sceneNumber - 1 );
		}
		
		delete scenes[sceneNumber];
		scenes.erase(scenes.begin() + sceneNumber);
		this->notifyRemoveScene( sceneNumber);
	}
	
}


void Animation::moveScene( int sceneNumber, int movePosition )
{
	if(sceneNumber != movePosition) {
		this->setActiveScene(-1);
		
		Scene *tmp;
		tmp = scenes[sceneNumber];
		scenes.erase(scenes.begin() + sceneNumber);
		scenes.insert(scenes.begin() + movePosition, tmp);
		
		this->notifyMoveScene(sceneNumber, movePosition);
		//this->setActiveScene(movePosition);
	}
}


int Animation::getActiveSceneNumber( )
{
	return activeScene;
}


bool Animation::initAudioDevice()
{
	isAudioDriverInitialized = audioDriver->initialize();
	if ( !isAudioDriverInitialized ) {
		frontend->reportError(
					"Cannot play sound. Check that you have the right\n"
					"permissions and other programs do not block\n"
					"the audio device. Audio will be disabled until you\n"
					"have fixed the problem.", 0);
	}

	return isAudioDriverInitialized;
}


void Animation::shutdownAudioDevice()
{
	audioDriver->shutdown();
	isAudioDriverInitialized = false;
}


void Animation::animationChanged(const char *alteredFile)
{
	int size = scenes[activeScene]->getSize();
	int changedFrame = -1;
	for(int i = 0; i < size; i++) {
		if(strcmp(scenes[activeScene]->getFrame(i)->getImagePath(), alteredFile) == 0) {
			changedFrame = i;
			break;
		}
	}
	
	if( changedFrame >= 0) {
		notifyAnimationChanged(changedFrame);
	}
}


bool Animation::exportToVideo(VideoEncoder * encoder)
{
	VideoFactory *factory = new VideoFactory(serializer, frontend);
	if ( factory->createVideoFile(encoder) != NULL ) {
		return true;
	}
	frontend->reportError("Video export failed ... to be continued", 0);
	return false;
}


bool Animation::exportToCinerella(const char*)
{
	return false;
}
