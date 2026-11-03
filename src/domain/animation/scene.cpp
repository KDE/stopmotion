/*
 * SPDX-FileCopyrightText: 2005-2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "scene.h"

#include <iterator>

#include "frame.h"
#include "sound.h"
#include "src/domain/filenamevisitor.h"

class Sound;
class WorkspaceFile;

FrameOutOfRangeException::FrameOutOfRangeException() {
}

const char* FrameOutOfRangeException::what() const noexcept {
	return "Internal error: Frame out of range!";
}


Scene::Scene() {
}


Scene::~Scene() {
}


int Scene::getSize() const {
	return frames.size();
}


const Frame* Scene::getFrame(int frameNumber) const {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber].get();
}


void Scene::cleanFrames(int fromFrame, int toFrame) {
	if (fromFrame < 0 || toFrame < fromFrame || getSize() <= toFrame)
		throw FrameOutOfRangeException();
	frames.erase(frames.begin() + fromFrame, frames.begin() + toFrame);
}


void Scene::addFrame(std::unique_ptr<Frame>&& f, int index) {
	if (index < 0 || getSize() < index)
		throw FrameOutOfRangeException();
	// leaves f untouched if this throws
	frames.insert(frames.begin() + index, std::move(f));
}

void Scene::addFrames(int where, std::vector<std::unique_ptr<Frame>>& fs) {
	int sceneSize = getSize();
	if (where < 0 || sceneSize < where)
		throw FrameOutOfRangeException();
	// only this can throw; afterwards the frames are moved without throwing
	frames.reserve(sceneSize + fs.size());
	frames.insert(frames.begin() + where,
			std::make_move_iterator(fs.begin()),
			std::make_move_iterator(fs.end()));
}

void Scene::preallocateFrames(int count) {
	frames.reserve(frames.size() + count);
}


void Scene::addSavedFrame(std::unique_ptr<Frame>&& f) {
	frames.push_back(std::move(f));
}

std::unique_ptr<Frame> Scene::removeFrame(int frame) {
	if (frame < 0 || getSize() <= frame)
		throw FrameOutOfRangeException();
	std::unique_ptr<Frame> f = std::move(frames[frame]);
	frames.erase(frames.begin() + frame);
	return f;
}

void Scene::removeFrames(int frame, int count, std::vector<std::unique_ptr<Frame>>& out) {
	if (count < 0 || frame < 0 || getSize() < frame + count)
		throw FrameOutOfRangeException();
	out.reserve(out.size() + count);
	FrameVector::iterator begin = frames.begin() + frame;
	FrameVector::iterator end = begin + count;
	out.insert(out.end(), std::make_move_iterator(begin),
			std::make_move_iterator(end));
	frames.erase(begin, end);
}

void Scene::moveFrames(int fromFrame, int toFrame, int movePosition ) {
	int size = getSize();
	if (fromFrame < 0 || toFrame < fromFrame || size <= toFrame
			|| movePosition < 0 || size <= movePosition)
		throw FrameOutOfRangeException();
	if (movePosition < fromFrame) {
		for (int i = fromFrame, j = movePosition; i <= toFrame; ++i, ++j) {
			std::unique_ptr<Frame> f = std::move(frames[i]);
			frames.erase(frames.begin() + i);
			frames.insert(frames.begin() + j, std::move(f));
		}
	} else {
		for (int i = fromFrame; i <= toFrame; ++i) {
			std::unique_ptr<Frame> f = std::move(frames[fromFrame]);
			frames.erase(frames.begin() + fromFrame);
			frames.insert(frames.begin() + movePosition, std::move(f));
		}
	}
}

void Scene::addSound(int frameNumber, int soundNumber,
		std::unique_ptr<Sound>&& sound) {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	frames[frameNumber]->addSound(soundNumber, std::move(sound));
}

std::unique_ptr<Sound> Scene::removeSound(int frameNumber, int index) {
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

std::string Scene::setSoundName(int frameNumber, int soundNumber,
		std::string soundName) {
	if (frameNumber < 0 || getSize() <= frameNumber)
		throw FrameOutOfRangeException();
	return frames[frameNumber]->setSoundName(soundNumber, std::move(soundName));
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
	for (const auto& frame : frames) {
		s += frame->soundCount();
	}
	return s;
}

void Scene::accept(FileNameVisitor& v) const {
	v.reportNewScene();
	for (const auto& frame : frames) {
		frame->accept(v);
	}
}
