/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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
#include "scene.h"
#include "frame.h"

#include <memory>

SceneVector::SceneVector() {
}

SceneVector::~SceneVector() {
	for (std::vector<Scene*>::iterator i = scenes.begin();
			i != scenes.end(); ++i) {
		delete *i;
	}
}

int SceneVector::sceneCount() const {
	return scenes.size();
}

void SceneVector::addScene(int where, Scene* newScene) {
	scenes.insert(scenes.begin() + where, newScene);
}

void SceneVector::addScene(int where) {
	std::auto_ptr s(new Scene());
	addScene(where, s.get());
	s.release();
}

void SceneVector::preallocateScenes(int count) {
	scenes.reserve(scenes.size() + count);
}

Scene* SceneVector::removeScene(int from) {
	Scene* s = scenes[from];
	scenes.erase(scenes.begin() + from);
	return s;
}

void SceneVector::moveScene(int from, int to) {
	Scene* s = removeScene(from);
	addScene(to, s);
}

const Scene* SceneVector::getScene(int which) const {
	return scenes[which];
}

int SceneVector::frameCount(int scene) const {
	return scenes[scene]->getSize();
}

void SceneVector::addFrame(int scene, int where, Frame* frame) {
	scenes[scene]->addFrame(frame, where);
}

void SceneVector::addFrames(int scene, int where,
		const std::vector<Frame*>& frames) {
	Scene* sc = scenes[scene];
	assert(where <= sc->getSize());
	std::vector<Frame*>& sceneFrames = sc->getFrames();
	sceneFrames.reserve(sceneFrames.size() + frames.size());
	sceneFrames.insert(sceneFrames.begin() + where,
			frames.begin(), frames.end());
}

void SceneVector::preallocateFrames(int scene, int count) {
	scenes[scene]->preallocateFrames(count);
}

Frame* SceneVector::removeFrame(int scene, int frame) {
	return scenes[scene]->removeFrame(frame);
}

void SceneVector::removeFrames(int scene, int frame, int count,
		std::vector<Frame*>& out) {
	out.reserve(out.size() + count);
	Scene* sc = scenes[scene];
	std::vector<Frame*>& sceneFrames = sc->getFrames();
	assert(frame + count <= sceneFrames.size());
	std::vector<Frame*>::iterator begin = sceneFrames.begin() + frame;
	std::vector<Frame*>::iterator end = sceneFrames.begin() + (frame + count);
	out.insert(out.end(), begin, end);
	sceneFrames.erase(begin, end);
}

void SceneVector::moveFrames(int fromScene, int fromFrame, int frameCount,
		int toScene, int toFrame) {
	Scene* from = scenes[fromScene];
	Scene* to = scenes[toScene];
	assert(fromFrame + frameCount < from->getSize());
	assert(toFrame <= to->getSize());
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
	} else if (fromFrame < toFrame) {
		// this will do for now, even though it is stupid
		for (int i = 0; i != frameCount; ++i) {
			Frame* f = from->removeFrame(fromFrame);
			to->addFrame(f, toFrame + frameCount - 1);
		}
	}
}

int SceneVector::soundCount(int scene, int frame) {
	return scenes[scene]->getFrame(frame)->getNumberOfSounds();
}

void SceneVector::addSound(int scene, int frame, int soundNumber,
		Frame::Sound* sound) {
	scenes[scene]->getFrame(frame)->addSound(sound, soundNumber);
}

const char* SceneVector::setSoundName(int scene, int frame, int soundNumber,
		const char* soundName) {
	return scenes[scene]->setSoundName(frame, soundNumber, soundName);
}

Frame::Sound* SceneVector::removeSound(int scene, int frame, int soundNumber) {
	return scenes[scene]->getFrame(frame)->removeSound(soundNumber);
}