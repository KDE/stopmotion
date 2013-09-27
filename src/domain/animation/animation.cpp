/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors.              *
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
#include "src/domain/undo/undoaddsound.h"
#include "src/domain/undo/undoremovesound.h"
#include "src/domain/undo/undorenamesound.h"
#include "src/domain/undo/undoaddscene.h"
#include "src/domain/undo/undoremovescene.h"
#include "src/domain/undo/undomovescene.h"
#include "src/domain/undo/undosetimage.h"

#include <vector>
#include <iostream>
#include <sstream>

namespace {
const char* commandAddFrames = "add-frame";
const char* commandRemoveFrames = "delete-frame";
const char* commandMoveFrames = "move-frame";
const char* commandSetImage = "set-image";
const char* commandAddSound = "add-sound";
const char* commandRemoveSound = "delete-sound";
const char* commandRenameSound = "rename-sound";
const char* commandAddScene = "new-scene";
const char* commandRemoveScene = "delete-scene";
const char* commandMoveScene = "move-scene";
}

Executor* makeAnimationCommandExecutor(SceneVector& model) {
	std::auto_ptr<Executor> ex(makeExecutor());
	std::auto_ptr<CommandFactory> add(new UndoAddFactory(model));
	ex->addCommand(commandAddFrames, add, true);
	std::auto_ptr<CommandFactory> remove(new UndoRemoveFactory(model));
	ex->addCommand(commandRemoveFrames, remove, false);
	std::auto_ptr<CommandFactory> move(new UndoMoveFactory(model));
	ex->addCommand(commandMoveFrames, move, false);
	std::auto_ptr<CommandFactory> setImage(new UndoSetImageFactory(model));
	ex->addCommand(commandSetImage, move, false);
	std::auto_ptr<CommandFactory> addSound(new UndoAddSoundFactory(model));
	ex->addCommand(commandAddSound, addSound, true);
	std::auto_ptr<CommandFactory> removeSound(new UndoRemoveSoundFactory(model));
	ex->addCommand(commandRemoveSound, removeSound, true);
	std::auto_ptr<CommandFactory> renameSound(new UndoRenameSoundFactory(model));
	ex->addCommand(commandRenameSound, renameSound, false);
	std::auto_ptr<CommandFactory> addScene(new UndoAddSceneFactory(model));
	ex->addCommand(commandAddScene, addScene, true);
	std::auto_ptr<CommandFactory> removeScene(new UndoRemoveSceneFactory(model));
	ex->addCommand(commandRemoveScene, removeScene, false);
	std::auto_ptr<CommandFactory> moveScene(new UndoMoveSceneFactory(model));
	ex->addCommand(commandMoveScene, moveScene, false);
	return ex.release();
}


Animation::Animation()
		: serializer(0), executor(0), audioDriver(0),
		  activeFrame(-1), activeScene(-1),
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


void Animation::addFrames(const vector<const char*>& frameNames,
		int32_t index) {
	if (getActiveSceneNumber() < 0) {
		newScene(0);
	}

	bool isAddingAborted = false;
	int count = frameNames.size();
	UndoAddFactory::Parameters params(getActiveSceneNumber(),
			index, count);
	bool showingProgress = 1 < count;
	if (showingProgress) {
		frontend->showProgress("Importing frames from disk ...", count);
	}
	std::string error;
	int added = 0;
	for (vector<const char*>::const_iterator i = frameNames.begin();
			i != frameNames.end(); ++i) {
		try {
			params.addFrame(*i);
			++added;
		} catch (CopyFailedException&) {
			error += "Cannot read file ";
			error += *i;
			error += "\n";
		}
		if (frontend->isOperationAborted()) {
			return;
		}
		if (showingProgress)
			frontend->updateProgress(added);
	}
	if (0 < added) {
		executor->execute(commandAddFrames, params);
		isChangesSaved = false;
	}
	if (showingProgress)
		frontend->hideProgress();
	if (!error.empty())
		frontend->reportError(error.c_str(), 0);
	setActiveFrame(index + added - 1);
}


void Animation::removeFrames(int32_t fromFrame, int32_t toFrame) {
	assert(fromFrame <= toFrame);
	executor->execute(commandRemoveFrames, activeFrame, fromFrame,
			toFrame - fromFrame - 1);
	isChangesSaved = false;
}


void Animation::moveFrames(int32_t fromFrame, int32_t toFrame,
		int32_t movePosition) {
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


int Animation::addSound(int32_t frameNumber, const char *soundFile) {
	Logger::get().logDebug("Adding sound in animation");
	std::auto_ptr<Frame::Sound> sound(new Frame::Sound());
	std::stringstream ss;
	std::stringstream::pos_type zeroOffset = ss.tellp();
	ss << "Sound" << WorkspaceFile::getSoundNumber();
	int size = (ss.tellp() - zeroOffset) + 1;
	char* soundName = new char[size];
	strncpy(soundName, ss.str().c_str(), size);
	const char* oldName = sound->setName(soundName);
	assert(oldName == NULL);
	int32_t index = scenes.soundCount(activeScene, frameNumber);
	try {
		executor->execute(commandAddSound, activeScene, frameNumber,
				index, soundFile, soundName);
	} catch (CouldNotOpenFileException&) {
		frontend->reportError(
				"Cannot open the selected audio file for reading.\n"
				"Check that you have the right permissions set.\n"
				"The animation will run without this sound if you\n"
				"choose to play.", 0);
		return -1;
	} catch (InvalidAudioFormatException&) {
		frontend->reportError(
				"The selected audio file is not a recognized\n"
				"audio format. The animation will run without\n"
				"this sound if you choose to play.", 0);
		return -2;
	}
	WorkspaceFile::nextSoundNumber();
}


void Animation::removeSound(int32_t frameNumber,int32_t soundNumber) {
	executor->execute(commandRemoveSound,
			activeScene, frameNumber, soundNumber);
}


void Animation::setSoundName(int32_t frameNumber, int32_t soundNumber,
		const char *soundName) {
	executor->execute(commandRenameSound, activeScene,
			frameNumber, soundNumber, soundName);
}


Frame* Animation::getFrame(int frameNumber, int sceneNumber) {
	if (frameNumber < scenes.frameCount(sceneNumber)) {
		Logger::get().logDebug("Retrieving frame from Animation");
		return scenes.getScene(sceneNumber)->getFrame(frameNumber);
	}
	else {
		Logger::get().logWarning("Requesting a frame which is not "
				"in the animation (frame number out of bounds)");
		return NULL;
	}
}


Frame* Animation::getFrame(int frameNumber) {
	if (activeScene >= 0) {
		return getFrame(frameNumber, activeScene);
	}
	else {
		return NULL;
	}
}


unsigned int Animation::getModelSize() {
	unsigned int modelSize = 0;
	unsigned int sceneCount = scenes.sceneCount();
	for (unsigned int i = 0; i < sceneCount; ++i) {
		modelSize += scenes.frameCount(i);
	}
	return modelSize;
}


unsigned int Animation::getSceneSize(int sceneNumber) {
	if (sceneNumber > -1 && sceneNumber < scenes.sceneCount()) {
		return scenes.frameCount(sceneNumber);
	}
	return 0;
}


unsigned int Animation::getNumberOfScenes() {
	return scenes.sceneCount();
}


void Animation::setActiveFrame(int frameNumber) {
	if (activeScene >= 0 || frameNumber == -1) {
		this->activeFrame = frameNumber;
		this->notifyNewActiveFrame(frameNumber);
	}
}


void Animation::playFrame(int frameNumber) {
	if (isAudioDriverInitialized) {
		Frame *f = getFrame(frameNumber);
		if (f) {
			f->playSounds(audioDriver);
		}
	}
	notifyPlayFrame(frameNumber);
}


int Animation::getActiveFrameNumber() {
	return activeFrame;
}


const char* Animation::getProjectFile() {
	return serializer->getProjectFile();
}

const char* Animation::getProjectPath() {
	return serializer->getProjectPath();
}


void Animation::clear() {
	serializer->cleanup();
	scenes.clear();
	executor->clearHistory();
	activeFrame = -1;
	activeScene = -1;
	isChangesSaved = true;
	WorkspaceFile::clear();
	notifyClear();
}


bool Animation::openProject(const char *filename) {
	clear();
	assert(filename != 0);
	scenes = serializer->open(filename);
	if (scenes.sceneCount() > 0) {
		loadSavedScenes();
		return true;
	}
	return false;
}


bool Animation::saveProject(const char *filename) {
	assert(filename != 0);
	return serializer->save(filename, scenes, frontend);
}


bool Animation::newProject() {
	clear();
	// do something here?
	return true;
}


void Animation::loadSavedScenes() {
	Logger::get().logDebug("Loading scenes in Animation:");

	unsigned int numElem = scenes.sceneCount();
	for (unsigned int i = 0; i < numElem; ++i) {
		notifyNewScene(i);
	}
	setActiveScene(numElem - 1);
}


bool Animation::isUnsavedChanges() {
	return !isChangesSaved;
}


void Animation::setActiveScene(int sceneNumber) {
	if (sceneNumber != activeScene) {
		activateScene(sceneNumber);
	}
}

void Animation::setImagePath(int32_t sceneNumber, int32_t frameNumber,
		const char* newImagePath) {
	executor->execute(commandSetImage, sceneNumber, frameNumber, newImagePath);
	if (activeScene == sceneNumber)
		notifyAnimationChanged(frameNumber);
}

class NullFrameIterator : public FrameIterator {
public:
	~NullFrameIterator() {
	}
	int count() const {
		return 0;
	}
	bool isAtEnd() const {
		return true;
	}
	const char* getName() {
		return 0;
	}
	void next() {
	}
};

void Animation::activateScene(int sceneNumber) {
	activeScene = sceneNumber;
	if (sceneNumber >= 0) {
		if (0 < scenes.frameCount(sceneNumber)) {
			std::auto_ptr<FrameIterator> frameIt(
					scenes.makeFrameIterator(sceneNumber));
			notifyNewActiveScene(sceneNumber, *frameIt, frontend);
			setActiveFrame(0);
		}
		else {
			NullFrameIterator dummy;
			notifyNewActiveScene(sceneNumber, dummy, frontend);
			setActiveFrame(-1);
		}
	}
	else {
		setActiveFrame(-1);
		NullFrameIterator dummy;
		notifyNewActiveScene(sceneNumber, dummy,frontend);
	}
}


void Animation::newScene(int32_t index) {
	executor->execute(commandAddScene, index);
	this->notifyNewScene( index );
	this->setActiveScene( index );
}


void Animation::removeScene(int32_t sceneNumber) {
	assert(sceneNumber >= 0);
	executor->execute(commandRemoveScene, sceneNumber);
	if (sceneNumber == scenes.sceneCount()) {
		activateScene(sceneNumber - 1);
	} else {
		activateScene(sceneNumber);
	}
	this->notifyRemoveScene(sceneNumber);
}


void Animation::moveScene(int32_t sceneNumber, int32_t movePosition) {
	if (sceneNumber != movePosition) {
		executor->execute(commandMoveScene, sceneNumber, movePosition);
		setActiveScene(-1);
		notifyMoveScene(sceneNumber, movePosition);
	}
}


int Animation::getActiveSceneNumber() {
	return activeScene;
}


bool Animation::initAudioDevice() {
	isAudioDriverInitialized = audioDriver->initialize();
	if (!isAudioDriverInitialized) {
		frontend->reportError(
					"Cannot play sound. Check that you have the right\n"
					"permissions and other programs do not block\n"
					"the audio device. Audio will be disabled until you\n"
					"have fixed the problem.", 0);
	}
	return isAudioDriverInitialized;
}


void Animation::shutdownAudioDevice() {
	audioDriver->shutdown();
	isAudioDriverInitialized = false;
}


void Animation::animationChanged(const char *alteredFile) {
	assert(alteredFile != NULL);
	if (activeScene == -1) {
		return;
	}

	int size = scenes.frameCount(activeScene);
	Scene* scene = scenes.getScene(activeScene);
	int changedFrame = -1;
	for (int i = 0; i < size; ++i) {
		Frame *f = scene->getFrame(i);
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


bool Animation::exportToVideo(VideoEncoder * encoder) {
	VideoFactory factory(serializer, frontend);
	frontend->showProgress("Exporting ...", 0);
	if (factory.createVideoFile(encoder) != NULL) {
		frontend->hideProgress();
		return true;
	}
	frontend->hideProgress();
	return false;
}


bool Animation::exportToCinerella(const char*) {
	return false;
}

void Animation::accept(FileNameVisitor& v) const {
	scenes.accept(v);
}

void Animation::undo() {
	executor->undo();
}

void Animation::redo() {
	executor->redo();
}

void Animation::clearHistory() {
	executor->clearHistory();
}
