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
#include "animation.h"

#include "src/foundation/logger.h"
#include "src/technical/audio/ossdriver.h"
#include "src/technical/video/videofactory.h"
#include "workspacefile.h"
#include "src/domain/undo/command.h"
#include "src/domain/undo/executor.h"

#include "src/domain/undo/undoadd.h"
#include "src/domain/undo/undoremove.h"
#include "src/domain/undo/undomove.h"

#include <vector>
#include <iostream>

namespace {
const char* commandAddFrames = "addf";
const char* commandRemoveFrames = "delf";
const char* commandMoveFrames = "movf";
}

Executor* makeAnimationCommandExecutor(SceneVector& model) {
	std::auto_ptr<Executor> ex(makeExecutor());
	std::auto_ptr<CommandFactory> add(new UndoAddFactory(model));
	ex->addCommand(commandAddFrames, add, true);
	std::auto_ptr<CommandFactory> remove(new UndoRemoveFactory(model));
	ex->addCommand(commandRemoveFrames, remove, false);
	std::auto_ptr<CommandFactory> move(new UndoMoveFactory(model));
	ex->addCommand(commandMoveFrames, move, false);
	return ex.release();
}


Animation::Animation()
		: serializer(0), executor(0), audioDriver(0),
		  activeFrame(-1), activeScene(-1), numSounds(-1),
		  isChangesSaved(true), isAudioDriverInitialized(false) {
	serializer  = new ProjectSerializer();
	executor = makeAnimationCommandExecutor(scenes);
	audioDriver = new OSSDriver("/dev/dsp");
}


Animation::~Animation() {
	serializer->cleanup();
	delete serializer;
	serializer = NULL;
	delete audioDriver;
	audioDriver = NULL;
}


const vector<const char*> Animation::addFrames(
		const vector<const char*>& frameNames, unsigned int index) {
	if (getActiveSceneNumber() < 0) {
		newScene(0);
	}

	bool isAddingAborted = false;
	int count = frameNames.size();
	UndoAddFactory::Parameters params(getActiveSceneNumber(),
			index, count);
	vector<const char*> newImagePaths(count);
	bool showingProgress = 1 < count;
	if (showingProgress) {
		frontend->showProgress("Importing frames from disk ...", count);
	}
	std::string error;
	for (vector<const char*>::iterator i = frameNames.begin();
			i != frameNames.end(); ++i) {
		try {
			const char* newPath = params.addFrame(*i);
			newImagePaths.push_back(newPath);
		} catch (CopyFailedException&) {
			error += "Cannot read file " + *i + "\n";
		}
		if (frontend->isOperationAborted()) {
			vector<const char*> dummy(0);
			return dummy;
		}
		if (showingProgress)
			frontend->updateProgress(newImagePaths.size());
	}
	if (0 < newImagePaths.size()) {
		executor->execute(commandAddFrames, params);
		isChangesSaved = false;
	}
	if (showingProgress)
		frontend->hideProgress();
	if (!error.empty())
		frontend->reportError(error.c_str(), 0);
	setActiveFrame(index + newImagePaths.size() - 1);
	return newImagePaths;
}


void Animation::removeFrames(unsigned int fromFrame,
		unsigned int toFrame ) {
	assert(fromFrame <= toFrame);
	executor->execute(commandRemoveFrames, activeFrame, fromFrame,
			toFrame - fromFrame - 1);
	isChangesSaved = false;
}


void Animation::moveFrames(unsigned int fromFrame, unsigned int toFrame, 
		unsigned int movePosition) {
	assert(fromFrame <= toFrame);
	int framesSize = scenes.frameCount(activeScene);
	assert(toFrame < framesSize);
	assert(movePosition < framesSize);
	if (movePosition < fromFrame || toFrame < movePosition) {
		executor->execute(commandMoveFrames,
				activeScene, fromFrame, toFrame - fromFrame + 1,
				activeScene, movePosition);
		isChangesSaved = false;
		this->notifyMove(fromFrame, toFrame, movePosition);
		this->setActiveFrame(movePosition);
	}
}


int Animation::addSound(unsigned int frameNumber, const char *sound)
{
	Logger::get().logDebug("Adding sound in animation");
	++numSounds;
	int ret = scenes[activeScene]->addSound(frameNumber, sound);
	if (ret == -1) {
		frontend->reportError(
				"Cannot open the selected audio file for reading.\n"
				"Check that you have the right permissions set.\n"
				"The animation will be runned without sound if you\n"
				"choose to play.", 0);
		--numSounds;
	}
	else if (ret == -2) {
		frontend->reportError(
				"The selected audio file is not valid within the\n" 
				"given audio format. The animation will be runned\n"
				"without sound if you choose to play.", 0);
		--numSounds;
	}
	return ret;
}


void Animation::removeSound( unsigned int frameNumber, unsigned int soundNumber )
{
	scenes[activeScene]->removeSound(frameNumber, soundNumber);
	--numSounds;
}


void Animation::setSoundName(unsigned int frameNumber, unsigned int soundNumber, char *soundName)
{
	scenes[activeScene]->setSoundName(frameNumber, soundNumber, soundName);
}


Frame* Animation::getFrame(unsigned int frameNumber, unsigned int sceneNumber)
{
	if (frameNumber < scenes[sceneNumber]->getSize()) {
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
	if (activeScene >= 0) {
		return getFrame(frameNumber, activeScene);
	}
	else {
		return NULL;
	}
}


unsigned int Animation::getModelSize() 
{
	unsigned int modelSize = 0;
	unsigned int size = scenes.size();
	for (unsigned int i = 0; i < size; ++i) {
		modelSize += scenes[i]->getSize();
	}
	return modelSize;
}


unsigned int Animation::getSceneSize( int sceneNumber )
{
	if ( sceneNumber > -1 && sceneNumber < int(scenes.size()) ) {
		return scenes[sceneNumber]->getSize();
	}
	return 0;
}


unsigned int Animation::getNumberOfScenes( )
{
	return scenes.size();
}


void Animation::setActiveFrame(int frameNumber)
{
	if (activeScene >= 0 || frameNumber == -1) {
		this->activeFrame = frameNumber;
		this->notifyNewActiveFrame(frameNumber);
	}
}


void Animation::playFrame(int frameNumber)
{
	if (isAudioDriverInitialized) {
		Frame *f = getFrame(frameNumber);
		if (f) {
			f->playSounds(audioDriver);
		}
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

const char* Animation::getProjectPath()
{
	return serializer->getProjectPath();
}


void Animation::clear()
{
	serializer->cleanup();
	
	unsigned int size = scenes.size();
	for (unsigned int i = 0; i < size; ++i) {
		delete scenes[i];
	}
	scenes.clear();
	
	activeFrame = -1;
	activeScene = -1;
	isChangesSaved = true;
	WorkspaceFile::clear();
	notifyClear();
}


bool Animation::openProject(const char *filename)
{
	clear();
	assert(filename != 0);
	scenes = serializer->open(filename);
	if (scenes.size() > 0) {
		loadSavedScenes();
		return true;
	}
	return false;
}


bool Animation::saveProject(const char *filename)
{	
	assert(filename != 0);
	return serializer->save(filename, scenes, frontend);
}


bool Animation::newProject()
{
	clear();
	// do something here?
	return true;
}


void Animation::loadSavedScenes()
{
	Logger::get().logDebug("Loading scenes in Animation:");
	
	unsigned int numElem = scenes.size();
	for (unsigned int i = 0; i < numElem; ++i) {
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
	if (sceneNumber != activeScene) {
		activateScene(sceneNumber);
	}
}


void Animation::activateScene( int sceneNumber )
{
	this->activeScene = sceneNumber;
	if (sceneNumber >= 0) {
		if (scenes[sceneNumber]->getSize() > 0) {
			this->notifyNewActiveScene(sceneNumber, 
					scenes[sceneNumber]->getImagePaths(),frontend);
			setActiveFrame(0);
		}
		else {
			vector<char*> dummy;
			this->notifyNewActiveScene(sceneNumber, dummy,frontend);
			setActiveFrame(-1);
		}
	}
	else {
		setActiveFrame(-1);
		vector<char*> dummy;
		this->notifyNewActiveScene(sceneNumber, dummy,frontend);
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
	if (sceneNumber >= 0) {
		if (sceneNumber < (int)scenes.size()-1 ) {
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
	if (sceneNumber != movePosition) {
		this->setActiveScene(-1);
		
		Scene *tmp;
		tmp = scenes[sceneNumber];
		scenes.erase(scenes.begin() + sceneNumber);
		scenes.insert(scenes.begin() + movePosition, tmp);
		
		this->notifyMoveScene(sceneNumber, movePosition);
	}
}


int Animation::getActiveSceneNumber( )
{
	return activeScene;
}


bool Animation::initAudioDevice()
{
	isAudioDriverInitialized = audioDriver->initialize();
	if ( !isAudioDriverInitialized && numSounds > -1 ) {
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
	assert(alteredFile != NULL);
	if (activeScene == -1) {
		return;
	}
	
	int size = scenes[activeScene]->getSize();
	int changedFrame = -1;
	for (int i = 0; i < size; ++i) {
		Frame *f = scenes[activeScene]->getFrame(i);
		if (f) {
			if (strcmp(f->getImagePath(), alteredFile) == 0) {
				changedFrame = i;
				break;
			}
		}
	}
	
	if (changedFrame >= 0) {
		notifyAnimationChanged(changedFrame);
	}
}


bool Animation::exportToVideo(VideoEncoder * encoder)
{
	VideoFactory factory(serializer, frontend);
	frontend->showProgress("Exporting ...", 0);
	if (factory.createVideoFile(encoder) != NULL) {
		frontend->hideProgress();
		return true;
	}
	frontend->hideProgress();
	return false;
}


bool Animation::exportToCinerella(const char*)
{
	return false;
}

void Animation::Accept(FileNameVisitor& v) const {
	for (sceneVector::const_iterator i = scenes.begin();
			i != scenes.end();
			++i) {
		(*i)->Accept(v);
	}
}
