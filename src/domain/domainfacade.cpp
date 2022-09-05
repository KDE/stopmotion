/***************************************************************************
 *   Copyright (C) 2005-2017 by Linuxstopmotion contributors;              *
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
#include "domainfacade.h"

#include "animation/animation.h"
#include "animation/workspacefile.h"
#include "src/foundation/logger.h"
#include "src/foundation/preferencestool.h"
#include "src/foundation/uiexception.h"
#include "src/presentation/frontends/frontend.h"
#include "technical/util.h"

#include <stdio.h>
#include <sys/file.h>
#include <exception>

DomainFacade* DomainFacade::domainFacade = 0;

const char* DomainFacade::getImagePath(int scene, int frame) {
	try {
		return animationModel->getImagePath(scene, frame);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
	return 0;
}

int DomainFacade::soundCount(int scene, int frame) const {
	try {
		return animationModel->soundCount(scene, frame);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
	return 0;
}

bool DomainFacade::loadProject(const char* datFilename,
		const char* projectFilename) {
	return animationModel->loadFromDat(datFilename, projectFilename);
}

void DomainFacade::setMostRecentProject() {
	const char *first = DomainFacade::getFacade()->getProjectFile();
	PreferencesTool *prefs = PreferencesTool::get();
	if (first) {
		prefs->setPreference("projectFile", first);
	} else {
		prefs->removePreference("projectFile");
	}
	if (first != 0) {
		Preference prefsFirst("mostRecent");
		if (!prefsFirst.equals(first)) {
			Preference second("secondMostRecent");
			prefs->setPreference("mostRecent", first);
			prefs->setPreference("secondMostRecent", prefsFirst.get());
			if (!second.equals(first)) {
				prefs->setPreference("thirdMostRecent", second.get());
			}
		}
	}
	try {
		prefs->flush();
	} catch (UiException& ex) {
		DomainFacade::getFacade()->getFrontend()->handleException(ex);
	}
}

DomainFacade::DomainFacade() {
	animationModel = new Animation();
	domainFacade = NULL;
}


DomainFacade::~DomainFacade() {
	delete animationModel;
	animationModel = NULL;
}


DomainFacade* DomainFacade::getFacade() {
	if(domainFacade == NULL) {
		domainFacade = new DomainFacade();
	}
	return domainFacade;
}


void DomainFacade::attach(Observer *o) {
	animationModel->attach(o);
}


void DomainFacade::detach(Observer *o) {
	animationModel->detach(o);
}


void DomainFacade::registerFrontend(Frontend *frontend) {
	animationModel->registerFrontend(frontend);
}


Frontend* DomainFacade::getFrontend() {
	return animationModel->getFrontend();
}


void DomainFacade::addFrames(int scene, int frame,
		StringIterator& frameNames) {
	try {
		Logger::get().logDebug("Adding frames in the domainfacade");
		animationModel->addFrames(scene, frame, frameNames);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


void DomainFacade::removeFrames(int scene, int frame, int count) {
	try {
		Logger::get().logDebug("Removing frames in the domainfacade");
		animationModel->removeFrames(scene, frame, count);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


void DomainFacade::moveFrames(int fromScene, int fromFrame,
		int count, int toScene, int toFrame) {
	try {
		animationModel->moveFrames(fromScene, fromFrame, count,
				toScene, toFrame);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


void DomainFacade::addSound(int scene, int frame, const char *filename) {
	try {
		Logger::get().logDebug("Adding sound in domainfacade");
		animationModel->addSound(scene, frame, filename);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


void DomainFacade::removeSound(int sceneNumber, int frameNumber,
		int soundNumber) {
	try {
		animationModel->removeSound(sceneNumber, frameNumber, soundNumber);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


void DomainFacade::setSoundName(int sceneNumber, int frameNumber,
		int soundNumber, const char* soundName) {
	try {
		animationModel->setSoundName(sceneNumber, frameNumber, soundNumber,
				soundName);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


void DomainFacade::openProject(const char *filename) {
	animationModel->openProject(filename);
}


void DomainFacade::saveProject(const char *directory) {
	animationModel->saveProject(directory);
}


bool DomainFacade::newProject() {
	animationModel->newProject();
	setMostRecentProject();
	return true;
}


bool DomainFacade::isUnsavedChanges() {
	return animationModel->isUnsavedChanges();
}


int DomainFacade::getModelSize() const {
	return animationModel->frameCount();
}


int DomainFacade::getSceneSize(int sceneNumber) const {
	try {
		return animationModel->frameCount(sceneNumber);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
	return 0;
}


int DomainFacade::getNumberOfScenes() const {
	return animationModel->sceneCount();
}


int DomainFacade::getNumberOfSounds(int scene, int frame) const {
	try {
		return animationModel->soundCount(scene, frame);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
	return 0;
}


const char* DomainFacade::getProjectFile() {
	return animationModel->getProjectFile();
}


void DomainFacade::undo() {
	animationModel->undo();
}


void DomainFacade::redo() {
	animationModel->redo();
}


void DomainFacade::clearHistory() {
	animationModel->clearHistory();
}


void DomainFacade::newScene(int index) {
	try {
		animationModel->newScene(index);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


void DomainFacade::removeScene(int sceneNumber) {
	try {
		animationModel->removeScene(sceneNumber);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


void DomainFacade::moveScene(int sceneNumber, int movePosition) {
	try {
		animationModel->moveScene(sceneNumber, movePosition);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
}


bool DomainFacade::initAudioDevice() {
	return animationModel->initAudioDevice();
}


void DomainFacade::shutdownAudioDevice() {
	return animationModel->shutdownAudioDevice();
}


bool DomainFacade::exportToVideo(VideoEncoder *encoder, int playbackSpeed) {
	return animationModel->exportToVideo(encoder, playbackSpeed);
}


bool DomainFacade::exportToCinerella(const char *file) {
	return animationModel->exportToCinerella(file);
}


const vector<GrabberDevice> DomainFacade::getGrabberDevices() {
	return Util::getGrabberDevices();
}

const char* DomainFacade::getSoundName(int sceneNumber, int frameNumber,
		int soundNumber) const {
	try {
		return animationModel->getSoundName(sceneNumber, frameNumber,
				soundNumber);
	} catch (UiException& e) {
		animationModel->resync(e);
	} catch (std::exception& e) {
		animationModel->resync(e);
	}
	return 0;
}

void DomainFacade::duplicateImage(int scene, int frame) {
	animationModel->duplicateImage(scene, frame);
}

void DomainFacade::initializeCommandLoggerFile() {
	WorkspaceFile wslf(WorkspaceFile::commandLogFile);
	FILE* log = fopen(wslf.path(), "a");
	if (!log)
		throw FailedToInitializeCommandLogger();
	if (flock(fileno(log), LOCK_EX | LOCK_NB)) {
		fclose(log);
		throw (UiException(UiException::failedToGetExclusiveLock));
	}
	animationModel->setCommandLoggerFile(log);
}

bool DomainFacade::replayCommandLog(const char* filename) {
	FILE* log = fopen(filename, "r");
	if (!log)
		return false;
	try {
		animationModel->replayCommandLog(log);
	} catch (UiException& e) {
		animationModel->resync(e);
		return false;
	} catch (std::exception& e) {
		Logger::get().logFatal("Recovery failed: %s", e.what());
		fclose(log);
		return false;
	}
	return true;
}

bool DomainFacade::canUndo() {
	return animationModel->canUndo();
}

bool DomainFacade::canRedo() {
	return animationModel->canRedo();
}

void DomainFacade::setUndoRedoObserver(UndoRedoObserver* observer) {
	animationModel->setUndoRedoObserver(observer);
}

void DomainFacade::playSounds(int scene, int frame) const {
	animationModel->playSounds(scene, frame);
}
