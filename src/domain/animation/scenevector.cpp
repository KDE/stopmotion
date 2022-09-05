/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#include "scenevector.h"

#include <exception>

#include "scene.h"
#include "frame.h"

class SceneOutOfRangeException : public std::exception {
public:
	SceneOutOfRangeException() {
	}
    const char* what() const throw() {
    	return "Internal error: Scene out of range!";
    }
};

SceneVector::SceneVector() : totalSoundCount(0) {
}

SceneVector::~SceneVector() {
	clear();
}

void SceneVector::clear() {
	for (std::vector<Scene*>::iterator i = scenes.begin();
			i != scenes.end(); ++i) {
		delete *i;
	}
	scenes.clear();
	totalSoundCount = 0;
}

int SceneVector::sceneCount() const {
	return scenes.size();
}

void SceneVector::addScene(int where, Scene* newScene) {
	if (where < 0 || sceneCount() < where)
		throw SceneOutOfRangeException();
	scenes.insert(scenes.begin() + where, newScene);
	totalSoundCount += newScene->soundCount();
}

void SceneVector::addScene(int where) {
	preallocateScenes(1);
	scenes.insert(scenes.begin() + where, new Scene());
}

void SceneVector::preallocateScenes(int count) {
	scenes.reserve(sceneCount() + count);
}

Scene* SceneVector::removeScene(int from) {
	int sc = sceneCount();
	if (from < 0 || sc <= from)
		throw SceneOutOfRangeException();
	Scene* s = scenes[from];
	scenes.erase(scenes.begin() + from);
	totalSoundCount -= s->soundCount();
	return s;
}

void SceneVector::moveScene(int from, int to) {
	int size = sceneCount();
	if (from < 0 || size <= from || to < 0 || size < to)
		throw SceneOutOfRangeException();
	Scene* s = removeScene(from);
	if (from < to)
		--to;
	addScene(to, s);
}

const Scene* SceneVector::getScene(int which) const {
	if (which < 0 || sceneCount() <= which)
		throw SceneOutOfRangeException();
	return scenes[which];
}

Scene* SceneVector::getMutableScene(int which) {
	if (which < 0 || sceneCount() <= which)
		throw SceneOutOfRangeException();
	return scenes[which];
}

int SceneVector::frameCount(int scene) const {
	return getScene(scene)->getSize();
}

void SceneVector::addFrame(int scene, int where, Frame* frame) {
	getMutableScene(scene)->addFrame(frame, where);
	totalSoundCount += frame->soundCount();
}

void SceneVector::addFrames(int scene, int where,
		const std::vector<Frame*>& frames) {
	getMutableScene(scene)->addFrames(where, frames);
	for (std::vector<Frame*>::const_iterator i = frames.begin();
			i != frames.end(); ++i) {
		totalSoundCount += (*i)->soundCount();
	}
}

void SceneVector::preallocateFrames(int scene, int count) {
	getMutableScene(scene)->preallocateFrames(count);
}

Frame* SceneVector::removeFrame(int scene, int frame) {
	Frame* r = getMutableScene(scene)->removeFrame(frame);
	totalSoundCount -= r->soundCount();
	return r;
}

void SceneVector::removeFrames(int scene, int frame, int count,
		std::vector<Frame*>& out) {
	int start = static_cast<int>(out.size());
	getMutableScene(scene)->removeFrames(frame, count, out);
	for (std::vector<Frame*>::const_iterator i = out.begin() + start;
			i != out.end(); ++i) {
		totalSoundCount -= (*i)->soundCount();
	}
}

void SceneVector::moveFrames(int fromScene, int fromFrame, int frameCount,
		int toScene, int toFrame) {
	Scene* from = getMutableScene(fromScene);
	Scene* to = getMutableScene(toScene);
	const int fromSize = from->getSize();
	const int toSize = to->getSize();
	if (fromSize < fromFrame + frameCount
			 	 || toSize < toFrame)
		 throw FrameOutOfRangeException();
	if (toScene != fromScene) {
		preallocateFrames(toScene, frameCount);
		// this will do for now, even though it is quadratic
		for (int i = 0; i != frameCount; ++i) {
			Frame* f = from->removeFrame(fromFrame);
			to->addFrame(f, toFrame + i);
		}
	} else if (toFrame < fromFrame) {
		// this will do for now, even though it is stupid
		for (int i = 0; i != frameCount; ++i) {
			Frame* f = from->removeFrame(fromFrame + i);
			to->addFrame(f, toFrame + i);
		}
	} else if (fromFrame + frameCount < toFrame) {
		// this will do for now, even though it is stupid
		for (int i = 0; i != frameCount; ++i) {
			// perform a rotation
			Frame* f = from->removeFrame(fromFrame);
			to->addFrame(f, toFrame - 1);
		}
	}
}

int SceneVector::soundCount(int scene, int frame) const {
	return getScene(scene)->getFrame(frame)->soundCount();
}

void SceneVector::addSound(int scene, int frame, int soundNumber,
		Sound* sound) {
	getMutableScene(scene)->addSound(frame, soundNumber, sound);
	++totalSoundCount;
}

const char* SceneVector::setSoundName(int scene, int frame, int soundNumber,
		const char* soundName) {
	return getMutableScene(scene)->setSoundName(frame, soundNumber, soundName);
}

Sound* SceneVector::removeSound(int scene, int frame, int soundNumber) {
	Sound* r = getMutableScene(scene)->removeSound(frame, soundNumber);
	if (r)
		--totalSoundCount;
	return r;
}

void SceneVector::replaceImage(int sceneNumber, int frameNumber,
		WorkspaceFile& otherImage) {
	getMutableScene(sceneNumber)->replaceImage(frameNumber, otherImage);
}

int SceneVector::soundCount() const {
	return totalSoundCount;
}

void SceneVector::accept(FileNameVisitor& v) const {
	for (std::vector<Scene*>::const_iterator i = scenes.begin();
			i != scenes.end();
			++i) {
		(*i)->accept(v);
	}
}

const char* SceneVector::getImagePath(int scene, int frame) const {
	return getScene(scene)->getFrame(frame)->getImagePath();
}

void SceneVector::playSounds(int scene, int frame, AudioDriver* driver) const {
	getScene(scene)->getFrame(frame)->playSounds(driver);
}
