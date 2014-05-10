/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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
#include "src/technical/projectserializer.h"
#include "workspacefile.h"
#include "scenevector.h"
#include "sound.h"
#include "scene.h"
#include "frame.h"
#include "animationimpl.h"
#include "src/domain/observernotifier.h"
#include "src/presentation/observer.h"

#include "src/domain/undo/commandadd.h"
#include "src/domain/undo/command.h"
#include "src/domain/undo/executor.h"
#include "src/domain/undo/addallcommands.h"
#include "src/domain/undo/filelogger.h"

#include <string.h>
#include <errno.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>

namespace {
bool ensureUnlinked(const char* path) {
	return 0 == unlink(path) || errno == ENOENT;
}
}

FailedToInitializeCommandLogger::FailedToInitializeCommandLogger() {
}

const char* FailedToInitializeCommandLogger::what() const throw () {
	return "Failed to initialize command logger";
}

Animation::Animation()
		: scenes(0), executor(0), logger(0), serializer(0), audioDriver(0),
		  isAudioDriverInitialized(false), frontend(0) {
	std::auto_ptr<SceneVector> scs(new SceneVector);
	std::auto_ptr<ObserverNotifier> on(new ObserverNotifier(scs.get(), 0));
	scs.release();
	std::auto_ptr<ProjectSerializer> szer(new ProjectSerializer);
	std::auto_ptr<OSSDriver> ad(new OSSDriver("/dev/dsp"));
	std::auto_ptr<Executor> ex(makeAnimationCommandExecutor(*on));
	std::auto_ptr<FileCommandLogger> lgr(new FileCommandLogger);
	ex->setCommandLogger(lgr->getLogger());
	scenes = on.release();
	serializer = szer.release();
	executor = ex.release();
	logger = lgr.release();
	audioDriver = ad.release();
}


Animation::~Animation() {
	delete scenes;
	scenes = NULL;
	delete serializer;
	serializer = NULL;
	delete audioDriver;
	audioDriver = NULL;
	delete executor;
	executor = NULL;
	delete logger;
	logger = NULL;
}


void Animation::addFrames(int scene, int frame,
		StringIterator& frameNames) {
	if (sceneCount() == 0) {
		newScene(0);
	}

	int count = frameNames.count();
	CommandAddFactory::Parameters params(scene, frame, count);
	bool showingProgress = 1 < count;
	if (showingProgress) {
		frontend->showProgress(Frontend::importingFramesFromDisk, count);
	}
	std::string error;
	// error.empty() is false if string is "\0"! So we set this explicitly on error.
	bool isError = false;
	int added = 0;
	for (; !frameNames.atEnd(); frameNames.next()) {
		try {
			params.addFrame(frameNames.get());
			++added;
		} catch (CopyFailedException&) {
			isError = true;
			error += "Cannot read file ";
			error += frameNames.get();
			error += "\n";
		}
		if (frontend->isOperationAborted()) {
			return;
		}
		if (showingProgress)
			frontend->updateProgress(added);
	}
	if (0 < added) {
		executor->execute(Commands::addFrames, params);
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
	executor->execute(Commands::removeFrames, scene, frame, count);
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
	executor->execute(Commands::moveFrames,
			fromScene, fromFrame, count,
			toScene, toFrame);
}


int Animation::addSound(int32_t scene, int32_t frameNumber,
		const char *soundFile) {
	TemporaryWorkspaceFile soundFileWs(soundFile);
	std::auto_ptr<Sound> sound(new Sound());
	std::stringstream ss;
	std::stringstream::pos_type zeroOffset = ss.tellp();
	ss << "Sound " << WorkspaceFile::getSoundNumber();
	int size = (ss.tellp() - zeroOffset) + 1;
	char* soundName = new char[size];
	std::string sc = ss.str();
	strncpy(soundName, sc.c_str(), size);
	const char* oldName = sound->setName(soundName);
	assert(oldName == NULL);
	int32_t index = soundCount(scene, frameNumber);
	try {
		executor->execute(Commands::addSound, scene, frameNumber,
				index, soundFileWs.basename(), soundName);
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
	soundFileWs.retainFile();
	WorkspaceFile::nextSoundNumber();
	return 0;
}


void Animation::removeSound(int32_t sceneNumber, int32_t frameNumber,
		int32_t soundNumber) {
	executor->execute(Commands::removeSound,
			sceneNumber, frameNumber, soundNumber);
}


void Animation::setSoundName(int32_t sceneNumber, int32_t frameNumber,
		int32_t soundNumber, const char *soundName) {
	executor->execute(Commands::renameSound, sceneNumber,
			frameNumber, soundNumber, soundName);
}


int Animation::frameCount() const {
	int modelSize = 0;
	int s = sceneCount();
	for (int i = 0; i < s; ++i) {
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


const char* Animation::getProjectFile() {
	return serializer->getProjectFile();
}


void Animation::clear() {
	logger->setLogFile(0);
	scenes->clear();
	executor->clearHistory();
	WorkspaceFile::clear();
}


void Animation::setScenes(const std::vector<Scene*>& sv) {
	int count = sv.size();
	scenes->preallocateScenes(count);
	scenes->clear();
	for (int i = 0; i != count; ++i) {
		scenes->addScene(i, sv[i]);
	}
}

bool Animation::loadFromDat(const char* filename,
		const char* projectFilename) {
	std::vector<Scene*> sv;
	if (ProjectSerializer::openDat(sv, filename)) {
		setScenes(sv);
		serializer->resetProjectFile(projectFilename);
		return true;
	}
	return false;
}


void Animation::openProject(const char *filename) {
	logger->setLogFile(0);
	clear();
	initializeCommandLog();
	assert(filename != 0);
	vector<Scene*> newScenes = serializer->openSto(filename);
	setScenes(newScenes);
}

FILE* Animation::initializeCommandLog() {
	WorkspaceFile commandLogger(WorkspaceFile::commandLogFile);
	FILE* fh = fopen(commandLogger.path(), "w");
	if (!fh)
		throw FailedToInitializeCommandLogger();
	logger->setLogFile(fh);
	return fh;
}

void Animation::saveProject(const char *filename) {
	assert(filename != 0);
	WorkspaceFile newDat(WorkspaceFile::newModelFile);
	if (!ensureUnlinked(newDat.path())) {
		Logger::get().logWarning("newModelFile not removed prior to saving");
	}
	serializer->save(filename, *scenes, frontend);
	WorkspaceFile currentDat(WorkspaceFile::currentModelFile);
	if (!ensureUnlinked(currentDat.path())) {
		Logger::get().logWarning("currentModelFile not removed after saving");
	}
	logger->setLogFile(0);
	initializeCommandLog();
	if (rename(newDat.path(), currentDat.path()) < 0)
		throw FailedToInitializeCommandLogger();
	executor->clearHistory();
}


void Animation::newProject() {
	WorkspaceFile newDat(WorkspaceFile::newModelFile);
	WorkspaceFile currentDat(WorkspaceFile::currentModelFile);
	if (!ensureUnlinked(newDat.path())
			|| !ensureUnlinked(currentDat.path())) {
		throw FailedToInitializeCommandLogger();
	}
	clear();
	serializer->resetProjectFile();
	initializeCommandLog();
}

const char* Animation::getImagePath(int scene, int frame) const {
	return scenes->getScene(scene)->getFrame(frame)->getImagePath();
}

const char* Animation::getSoundName(int scene, int frame,
		int soundNumber) const {
	return scenes->getScene(scene)->getFrame(frame)->getSoundName(soundNumber);
}

bool Animation::isUnsavedChanges() {
	return executor->canUndo();
}

void Animation::setImagePath(int32_t sceneNumber, int32_t frameNumber,
		const char* newImagePath) {
	TemporaryWorkspaceFile twf(newImagePath);
	executor->execute(Commands::setImage, sceneNumber, frameNumber, twf.basename());
	twf.retainFile();
}

void Animation::duplicateImage(int32_t sceneNumber, int32_t frameNumber) {
	const char* currentPath = getImagePath(sceneNumber, frameNumber);
	TemporaryWorkspaceFile twf(currentPath, TemporaryWorkspaceFile::forceCopy);
	executor->execute(Commands::setImage, sceneNumber, frameNumber, twf.basename());
	twf.retainFile();
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
	executor->execute(Commands::addScene, index);
}


void Animation::removeScene(int32_t sceneNumber) {
	assert(sceneNumber >= 0);
	executor->execute(Commands::removeScene, sceneNumber);
}


void Animation::moveScene(int32_t sceneNumber, int32_t movePosition) {
	if (sceneNumber != movePosition) {
		executor->execute(Commands::moveScene, sceneNumber, movePosition);
	}
}


bool Animation::initAudioDevice() {
	isAudioDriverInitialized = audioDriver->initialize();
	if (!isAudioDriverInitialized && 0 < scenes->soundCount()) {
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


bool Animation::exportToVideo(VideoEncoder * encoder) {
	VideoFactory factory(scenes, frontend);
	frontend->showProgress(Frontend::exporting, 0);
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

void Animation::resync(std::exception& e) {
	if (frontend)
		frontend->reportError(e.what(), 1);
	scenes->resync();
}

const char* Animation::getSoundPath(int scene, int frame, int sound) const {
	return scenes->getScene(scene)->getSound(frame, sound)->getAudio()
			->getSoundPath();
}

int Animation::soundCount() const {
	return scenes->soundCount();
}

void Animation::setCommandLoggerFile(FILE* file) {
	logger->setLogFile(file);
}

class GetLine {
	FILE* fh;
	char* buffer;
	size_t size;
public:
	GetLine(FILE* handle) : fh(handle), buffer(0), size(0) {
	}
	~GetLine() {
		free(buffer);
	}
	const char* get() const {
		return buffer;
	}
	bool next() {
		return 0 < getline(&buffer, &size, fh);
	}
};

void Animation::replayCommandLog(FILE* file) {
	long startPos = ftell(file);
	long length = 0;
	if (frontend && startPos != -1 && 0 == fseek(file, 0, SEEK_END)) {
		length = ftell(file);
		if (0 != fseek(file, startPos, SEEK_SET))
			throw FileException("replayCommandLog", errno);
		if (length < 1000) {
			length = 0;
		} else {
			frontend->showProgress(Frontend::restoringProject, length - startPos);
		}
	}
	GetLine lineIterator(file);
	int r = 0;
	while (0 < (r = lineIterator.next())) {
		executor->executeFromLog(lineIterator.get());
		if (0 < length) {
			long pos = ftell(file) - startPos;
			frontend->updateProgress(pos);
		}
	}
	if (0 < length)
		frontend->hideProgress();
	if (r < 0)
		throw FileException("replayCommandLog", errno);
}

bool Animation::canUndo() {
	return executor->canUndo();
}

bool Animation::canRedo() {
	return executor->canRedo();
}

void Animation::setUndoRedoObserver(UndoRedoObserver* observer) {
	executor->setUndoRedoObserver(observer);
}
