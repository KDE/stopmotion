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
#include "scene.h"
#include "src/domain/filenamevisitor.h"

#include <sstream>
#include <cstring>
#include <unistd.h>

Scene::Scene() {
}


Scene::~Scene() {
	unsigned int numElem = frames.size();
	for (unsigned int i = 0; i < numElem; ++i) {
		delete frames[i];
	}
}


vector<Frame*>& Scene::getFrames() {
	return frames;
}


int Scene::getSize() const {
	return frames.size();
}


Frame* Scene::getFrame(int frameNumber) {
	return frames[frameNumber];
}


void Scene::cleanFrames(unsigned int fromFrame, unsigned int toFrame) {
	frames.erase(frames.begin() + fromFrame, frames.begin() + toFrame);
}


void Scene::addFrame(Frame* f, int index) {
	frames.insert(frames.begin() + index, f);
}

void Scene::preallocateFrames(int count) {
	frames.reserve(frames.size() + count);
}


void Scene::addSavedFrame(Frame *f)
{
	frames.push_back(f);
}

Frame* Scene::removeFrame(unsigned int frame) {
	Frame* f = frames[frame];
	frames.erase(frames.begin() + frame);
	return f;
}

void Scene::moveFrames( unsigned int fromFrame, unsigned int toFrame,
		unsigned int movePosition ) {
	if (movePosition < fromFrame) {
		for (unsigned int i = fromFrame, j = movePosition; i <= toFrame; ++i, ++j) {
			Frame *f = frames[i];
			frames.erase(frames.begin() + i);
			frames.insert(frames.begin() + j, f);
		}
	}
	else {
		for (unsigned int i = fromFrame; i <= toFrame; ++i) {
			Frame *f = frames[fromFrame];
			frames.erase(frames.begin() + fromFrame);
			frames.insert(frames.begin() + movePosition, f);
		}
	}
}


void Scene::accept(FileNameVisitor& v) const {
	for (frameVector::const_iterator i = frames.begin();
			i != frames.end();
			++i) {
		(*i)->accept(v);
	}
}
