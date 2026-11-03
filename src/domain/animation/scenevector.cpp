/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "scenevector.h"

#include <exception>

#include "scene.h"
#include "frame.h"
#include "sound.h"

class SceneOutOfRangeException : public std::exception {
public:
	SceneOutOfRangeException() {
	}
    const char* what() const noexcept override {
    	return "Internal error: Scene out of range!";
    }
};

SceneVector::SceneVector() : totalSoundCount(0) {
}

SceneVector::~SceneVector() {
	clear();
}

void SceneVector::clear() {
	for (Scene* scene : scenes) {
		delete scene;
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

void SceneVector::addFrames(int scene, int where,
		std::vector<std::unique_ptr<Frame>>& frames) {
	int sounds = 0;
	for (const auto& frame : frames) {
		sounds += frame->soundCount();
	}
	getMutableScene(scene)->addFrames(where, frames);
	totalSoundCount += sounds;
}

void SceneVector::preallocateFrames(int scene, int count) {
	getMutableScene(scene)->preallocateFrames(count);
}

void SceneVector::removeFrames(int scene, int frame, int count,
		std::vector<std::unique_ptr<Frame>>& out) {
	int start = static_cast<int>(out.size());
	getMutableScene(scene)->removeFrames(frame, count, out);
	for (auto i = out.begin() + start; i != out.end(); ++i) {
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
			to->addFrame(from->removeFrame(fromFrame), toFrame + i);
		}
	} else if (toFrame < fromFrame) {
		// this will do for now, even though it is stupid
		for (int i = 0; i != frameCount; ++i) {
			to->addFrame(from->removeFrame(fromFrame + i), toFrame + i);
		}
	} else if (fromFrame + frameCount < toFrame) {
		// this will do for now, even though it is stupid
		for (int i = 0; i != frameCount; ++i) {
			// perform a rotation
			to->addFrame(from->removeFrame(fromFrame), toFrame - 1);
		}
	}
}

int SceneVector::soundCount(int scene, int frame) const {
	return getScene(scene)->getFrame(frame)->soundCount();
}

void SceneVector::addSound(int scene, int frame, int soundNumber,
		std::unique_ptr<Sound>&& sound) {
	getMutableScene(scene)->addSound(frame, soundNumber, std::move(sound));
	++totalSoundCount;
}

std::string SceneVector::setSoundName(int scene, int frame, int soundNumber,
		std::string soundName) {
	return getMutableScene(scene)->setSoundName(frame, soundNumber,
			std::move(soundName));
}

std::unique_ptr<Sound> SceneVector::removeSound(int scene, int frame,
		int soundNumber) {
	std::unique_ptr<Sound> r = getMutableScene(scene)->removeSound(frame, soundNumber);
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
	for (const Scene* scene : scenes) {
		scene->accept(v);
	}
}

const char* SceneVector::getImagePath(int scene, int frame) const {
	return getScene(scene)->getFrame(frame)->getImagePath();
}

void SceneVector::playSounds(int scene, int frame, AudioDriver* driver) const {
	getScene(scene)->getFrame(frame)->playSounds(driver);
}
