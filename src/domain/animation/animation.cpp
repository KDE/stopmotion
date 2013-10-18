/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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
#include "scenevector.h"
#include "sound.h"
#include "animationimpl.h"
#include "src/domain/observernotifier.h"
#include "src/presentation/observer.h"

#include "src/domain/undo/command.h"
#include "src/domain/undo/executor.h"

#include "src/domain/undo/commandadd.h"
#include "src/domain/undo/commandremove.h"
#include "src/domain/undo/commandmove.h"
#include "src/domain/undo/commandaddsound.h"
#include "src/domain/undo/commandremovesound.h"
#include "src/domain/undo/commandrenamesound.h"
#include "src/domain/undo/commandaddscene.h"
#include "src/domain/undo/commandremovescene.h"
#include "src/domain/undo/commandmovescene.h"
#include "src/domain/undo/commandsetimage.h"

#include <string.h>
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

Executor* makeAnimationCommandExecutor(AnimationImpl& model) {
	std::auto_ptr<Executor> ex(makeExecutor());
	std::auto_ptr<CommandFactory> add(new CommandAddFactory(model));
	ex->addCommand(commandAddFrames, add, true);
	std::auto_ptr<CommandFactory> remove(new CommandRemoveFactory(model));
	ex->addCommand(commandRemoveFrames, remove, false);
	std::auto_ptr<CommandFactory> move(new CommandMoveFactory(model));
	ex->addCommand(commandMoveFrames, move, false);
	std::auto_ptr<CommandFactory> setImage(new CommandSetImageFactory(model));
	ex->addCommand(commandSetImage, move, false);
	std::auto_ptr<CommandFactory> addSound(new CommandAddSoundFactory(model));
	ex->addCommand(commandAddSound, addSound, true);
	std::auto_ptr<CommandFactory> removeSound(new UndoRemoveSoundFactory(model));
	ex->addCommand(commandRemoveSound, removeSound, true);
	std::auto_ptr<CommandFactory> renameSound(new CommandRenameSoundFactory(model));
	ex->addCommand(commandRenameSound, renameSound, false);
	std::auto_ptr<CommandFactory> addScene(new CommandAddSceneFactory(model));
	ex->addCommand(commandAddScene, addScene, true);
	std::auto_ptr<CommandFactory> removeScene(new UndoRemoveSceneFactory(model));
	ex->addCommand(commandRemoveScene, removeScene, false);
	std::auto_ptr<CommandFactory> moveScene(new CommandMoveSceneFactory(model));
	ex->addCommand(commandMoveScene, moveScene, false);
	return ex.release();
}


Animation::Animation()
		: scenes(0), serializer(0), executor(0), audioDriver(0),
		  isAudioDriverInitialized(false), frontend(0) {
	std::auto_ptr<SceneVector> scs(new SceneVector);
	std::auto_ptr<ObserverNotifier> on(new ObserverNotifier(scs.get(), 0));
	scs.release();
	std::auto_ptr<ProjectSerializer> szer(new ProjectSerializer);
	std::auto_ptr<Executor> ex(makeAnimationCommandExecutor(*on));
	std::auto_ptr<OSSDriver> ad(new OSSDriver("/dev/dsp"));
	scenes = on.release();
	serializer = szer.release();
	executor = ex.release();
	audioDriver = ad.release();
}


Animation::~Animation() {
	delete scenes;
	serializer->cleanup();
	delete serializer;
	serializer = NULL;
	delete audioDriver;
	audioDriver = NULL;
	delete executor;
	executor = NULL;
}


void Animation::addFrames(int scene, int frame,
		const vector<const char*>& frameNames) {
	if (sceneCount() == 0) {
		newScene(0);
	}

	int count = frameNames.size();
	CommandAddFactory::Parameters params(scene, frame, count);
	bool showingProgress = 1 < count;
	if (showingProgress) {
		frontend->showProgress("Importing frames from disk ...", count);
	}
	std::string error;
	// error.empty() is false if string is "\0"! So we set this explicitly on error.
	bool isError = false;
	int added = 0;
	for (vector<const char*>::const_iterator i = frameNames.begin();
			i != frameNames.end(); ++i) {
		try {
			params.addFrame(*i);
			++added;
		} catch (CopyFailedException&) {
			isError = true;
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
		params.retainFiles();
	}
	if (showingProgress)
		frontend->hideProgress();
	if (isError)
		frontend->reportError(error.c_str(), 0);
}


void Animation::removeFrames(int32_t scene, int32_t frame, int32_t count) {
	assert(0 <= count);
	assert(frame + count <= frameCount(scene));
	executor->execute(commandRemoveFrames, scene, frame, count);
}


void Animation::moveFrames(int32_t fromScene, int32_t fromFrame,
		int32_t count, int32_t toScene, int32_t toFrame) {
	assert(0 <= count);
	assert(fromFrame + count <= frameCount(fromScene));
	assert(toFrame <= frameCount(toScene));
	if (toScene == fromScene
			&& fromFrame <= toFrame && toFrame < fromFrame + count) {
		// Attempt to move frames into the same position; ineffective.
		return;
	}
	executor->execute(commandMoveFrames,
			fromScene, fromFrame, count,
			toScene, toFrame);
}


int Animation::addSound(int32_t scene, int32_t frameNumber,
		const char *soundFile) {
	Logger::get().logDebug("Adding sound in animation");
	std::auto_ptr<Sound> sound(new Sound());
	std::stringstream ss;
	std::stringstream::pos_type zeroOffset = ss.tellp();
	ss << "Sound" << WorkspaceFile::getSoundNumber();
	int size = (ss.tellp() - zeroOffset) + 1;
	char* soundName = new char[size];
	strncpy(soundName, ss.str().c_str(), size);
	const char* oldName = sound->setName(soundName);
	assert(oldName == NULL);
	int32_t index = soundCount(scene, frameNumber);
	try {
		executor->execute(commandAddSound, scene, frameNumber,
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
	return 0;
}


void Animation::removeSound(int32_t sceneNumber, int32_t frameNumber,
		int32_t soundNumber) {
	executor->execute(commandRemoveSound,
			sceneNumber, frameNumber, soundNumber);
}


void Animation::setSoundName(int32_t sceneNumber, int32_t frameNumber,
		int32_t soundNumber, const char *soundName) {
	executor->execute(commandRenameSound, sceneNumber,
			frameNumber, soundNumber, soundName);
}


const Frame* Animation::getFrame2(int sceneNumber, int frameNumber) const {
	if (frameNumber < frameCount(sceneNumber)) {
		Logger::get().logDebug("Retrieving frame from Animation");
		return scenes->getScene(sceneNumber)->getFrame(frameNumber);
	}
	else {
		Logger::get().logWarning("Requesting a frame which is not "
				"in the animation (frame number out of bounds)");
		return NULL;
	}
}


int Animation::frameCount() const {
	int modelSize = 0;
	int s = sceneCount();
	for (unsigned int i = 0; i < s; ++i) {
		modelSize += frameCount(i);
	}
	return modelSize;
}


int Animation::frameCount(int sceneNumber) const {
	if (sceneNumber > -1 && sceneNumber < sceneCount()) {
		return scenes->frameCount(sceneNumber);
	}
	return 0;
}


int Animation::sceneCount() const {
	return scenes->sceneCount();
}


/*void Animation::playFrame(int frameNumber) {
	if (isAudioDriverInitialized) {
		const Frame *f = getFrame(frameNumber);
		if (f) {
			f->playSounds(audioDriver);
		}
	}
	scenes->notifyPlayFrame(activeScene, frameNumber);
}*/


const char* Animation::getProjectFile() {
	return serializer->getProjectFile();
}

const char* Animation::getProjectPath() {
	return serializer->getProjectPath();
}


void Animation::clear() {
	serializer->cleanup();
	scenes->clear();
	executor->clearHistory();
	WorkspaceFile::clear();
}


//TODO make all this exception-safe
bool Animation::openProject(const char *filename) {
	clear();
	assert(filename != 0);
	vector<Scene*> newScenes = serializer->open(filename);
	int count = newScenes.size();
	scenes->preallocateScenes(count);
	for (int i = 0; i != count; ++i) {
		scenes->addScene(i, newScenes[i]);
	}
	if (count > 0) {
		loadSavedScenes();
		return true;
	}
	return false;
}


//TODO do this in a less stupid way.
bool Animation::saveProject(const char *filename) {
	assert(filename != 0);
	std::vector<const Scene*> s;
	int count = scenes->sceneCount();
	for (int i = 0; i != count; ++i) {
		s.push_back(scenes->getScene(i));
	}
	return serializer->save(filename, s, frontend);
}


bool Animation::newProject() {
	clear();
	// do something here?
	return true;
}


//TODO change this to something that sets all the scenes at once, notifying
// once also.
void Animation::loadSavedScenes() {
	Logger::get().logDebug("Loading scenes in Animation:");

	unsigned int numElem = scenes->sceneCount();
	for (unsigned int i = 0; i < numElem; ++i) {
//		notifyNewScene(i);
	}
}


bool Animation::isUnsavedChanges() {
	return executor->canUndo();
}


void Animation::setImagePath(int32_t sceneNumber, int32_t frameNumber,
		const char* newImagePath) {
	executor->execute(commandSetImage, sceneNumber, frameNumber, newImagePath);
}

void Animation::attach(Observer* o) {
	scenes->addObserver(o);
}

void Animation::detach(Observer* o) {
	scenes->removeObserver(o);
}

void Animation::registerFrontend(Frontend* fe) {
	scenes->registerFrontend(fe);
	frontend = fe;
}

Frontend* Animation::getFrontend() {
	return frontend;
}

int Animation::soundCount(int scene, int frame) const {
	return scenes->soundCount(scene, frame);
}


void Animation::newScene(int32_t index) {
	executor->execute(commandAddScene, index);
}


void Animation::removeScene(int32_t sceneNumber) {
	assert(sceneNumber >= 0);
	executor->execute(commandRemoveScene, sceneNumber);
}


void Animation::moveScene(int32_t sceneNumber, int32_t movePosition) {
	if (sceneNumber != movePosition) {
		executor->execute(commandMoveScene, sceneNumber, movePosition);
	}
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
	// linear search through the frames isn't very efficient;
	// we should add something cleverer if it gets to be a problem.
	assert(alteredFile != NULL);
	for (int s = 0; s != sceneCount(); ++s) {
		const Scene* scene = scenes->getScene(s);
		int frameCount = scene->getSize();
		for (int f = 0; f != frameCount; ++f) {
			const Frame *frame = scene->getFrame(f);
			if (strcmp(frame->getImagePath(), alteredFile) == 0) {
				scenes->replaceImage(s, f);
			}
		}
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
	scenes->accept(v);
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
