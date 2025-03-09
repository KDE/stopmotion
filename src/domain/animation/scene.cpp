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

#include "scene.h"

#include "frame.h"
#include "src/domain/filenamevisitor.h"

class Sound;
class WorkspaceFile;

FrameOutOfRangeException::FrameOutOfRangeException() {
}

const char* FrameOutOfRangeException::what() const throw() {
	return "Internal error: Frame out of range!";
}


Scene::Scene() {
}


Scene::~Scene() {
	unsigned int numElem = frames.size();
	for (unsigned int i = 0; i < numElem; ++i) {
		delete frames[i];
	}
}


int Scene::getSize() const {
	return frames.size();
}


const Frame* Scene::getFrame(int frameNumber) const {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber];
}


void Scene::cleanFrames(int fromFrame, int toFrame) {
	if (fromFrame < 0 || toFrame < fromFrame || getSize() <= toFrame)
		throw FrameOutOfRangeException();
	frames.erase(frames.begin() + fromFrame, frames.begin() + toFrame);
}


void Scene::addFrame(Frame* f, int index) {
	if (index < 0 || getSize() < index)
		throw FrameOutOfRangeException();
	frames.insert(frames.begin() + index, f);
}

void Scene::addFrames(int where, const std::vector<Frame*>& fs) {
	int sceneSize = getSize();
	if (where < 0 || sceneSize < where)
		throw FrameOutOfRangeException();
	std::vector<Frame*>::size_type newFramesCount = fs.size();
	preallocateFrames(newFramesCount);
	frames.reserve(sceneSize + newFramesCount);
	frames.insert(frames.begin() + where,
			fs.begin(), fs.end());
}

void Scene::preallocateFrames(int count) {
	frames.reserve(frames.size() + count);
}


void Scene::addSavedFrame(Frame *f) {
	frames.push_back(f);
}

Frame* Scene::removeFrame(int frame) {
	if (frame < 0 || getSize() <= frame)
		throw FrameOutOfRangeException();
	Frame* f = frames[frame];
	frames.erase(frames.begin() + frame);
	return f;
}

void Scene::removeFrames(int frame, int count, std::vector<Frame*>& out) {
	if (count < 0 || frame < 0 || getSize() < frame + count)
		throw FrameOutOfRangeException();
	out.reserve(out.size() + count);
	std::vector<Frame*>::iterator begin = frames.begin() + frame;
	std::vector<Frame*>::iterator end = begin + count;
	out.insert(out.end(), begin, end);
	frames.erase(begin, end);
}

void Scene::moveFrames(int fromFrame, int toFrame, int movePosition ) {
	int size = getSize();
	if (fromFrame < 0 || toFrame < fromFrame || size <= toFrame
			|| movePosition < 0 || size <= movePosition)
		throw FrameOutOfRangeException();
	if (movePosition < fromFrame) {
		for (int i = fromFrame, j = movePosition; i <= toFrame; ++i, ++j) {
			Frame *f = frames[i];
			frames.erase(frames.begin() + i);
			frames.insert(frames.begin() + j, f);
		}
	} else {
		for (int i = fromFrame; i <= toFrame; ++i) {
			Frame *f = frames[fromFrame];
			frames.erase(frames.begin() + fromFrame);
			frames.insert(frames.begin() + movePosition, f);
		}
	}
}

void Scene::addSound(int frameNumber, int soundNumber, Sound* sound) {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	frames[frameNumber]->addSound(soundNumber, sound);
}

Sound* Scene::removeSound(int frameNumber, int index) {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber]->removeSound(index);
}

const Sound* Scene::getSound(int frameNumber, int index) const {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber]->getSound(index);
}

int Scene::soundCount(int frameNumber) const {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber]->soundCount();
}

const char* Scene::setSoundName(int frameNumber, int soundNumber,
		const char* soundName) {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber]->setSoundName(soundNumber, soundName);
}

const char* Scene::getSoundName(int frameNumber, int soundNumber) const {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber]->getSoundName(soundNumber);
}

void Scene::replaceImage(int frameNumber, WorkspaceFile& otherImage) {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	frames[frameNumber]->replaceImage(otherImage);
}

int Scene::newSound(int frameNumber, WorkspaceFile& file) {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber]->newSound(file);
}

int Scene::soundCount() const {
	int s = 0;
	for (const Frame* frame : frames) {
		s += frame->soundCount();
	}
	return s;
}

void Scene::accept(FileNameVisitor& v) const {
	v.reportNewScene();
	for (const Frame* frame : frames) {
		frame->accept(v);
	}
}
