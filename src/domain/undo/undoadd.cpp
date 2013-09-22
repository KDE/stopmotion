/***************************************************************************
 *   Copyright (C) 2013 Stopmotion contributors                            *
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
#include "undoadd.h"

#include "src/domain/animation/scenevector.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/workspacefile.h"

UndoAdd::UndoAdd(SceneVector& model, int toScene, int toFrame, int count)
		: sv(model), scene(toScene), frame(toFrame) {
	frames.reserve(count);
}

UndoAdd::~UndoAdd() {
	for (std::vector<Frame*>::iterator i = frames.begin();
			i != frames.end(); ++i) {
		delete *i;
	}
}

void UndoAdd::addFrame(Frame* frame) {
	frames.push_back(frame);
}

Command* UndoAdd::execute() {
	UndoRemove* inverse = new UndoRemove(sv, scene, frame, frames.size());
	sv.addFrames(scene, frame, frames);
	// ownership has been passed, so we must forget the frames
	frames.clear();
	delete this;
	return inverse;
}

void UndoAdd::accept(FileNameVisitor& v) const {
	for (std::vector<Frame*>::iterator i = frames.begin();
			i != frames.end(); ++i) {
		(*i)->accept(v);
	}
}

UndoAddFactory::Parameters::Parameters(int scene, int frame, int count)
		: sc(scene), fr(frame), frameCount(count), twfs(0), twfCount(0),
		  parameterCount(0) {
	twfs = (TemporaryWorkspaceFile*) malloc(
			frameCount * sizeof(TemporaryWorkspaceFile));
}

UndoAddFactory::Parameters::~Parameters() {
	for (int i = 0; i != twfCount; ++i) {
		twfs[i].~TemporaryWorkspaceFile();
	}
	free(twfs);
}

const char* UndoAddFactory::Parameters::addFrame(const char* filename) {
	assert (twfCount != frameCount);
	TemporaryWorkspaceFile* p = twfs + twfCount;
	new (p) TemporaryWorkspaceFile(filename);
	++twfCount;
	return p->path();
}

int32_t UndoAddFactory::Parameters::getInteger(int32_t min, int32_t max) {
	assert(parameterCount < 2);
	++parameterCount;
	return parameterCount == 0? sc : fr;
}

int32_t UndoAddFactory::Parameters::getHowMany() {
	assert(parameterCount == 2);
	++parameterCount;
	return frameCount;
}

void UndoAddFactory::Parameters::getString(std::string& out) {
	assert(3 <= parameterCount);
	int index = parameterCount - 3;
	++parameterCount;
	out.assign(twfs[index].basename());
}

void UndoAddFactory::Parameters::retainFiles() {
	// use this to empty out the TemporaryWorkspaceFiles
	WorkspaceFile dummy;
	for (int i = 0; i != twfCount; ++i) {
		dummy = twfs[i];
	}
}

UndoAddFactory::UndoAddFactory(SceneVector& model) : sv(model) {
}

UndoAddFactory::~UndoAddFactory() {
}

Command* UndoAddFactory::create(::Parameters& ps) {
	int scene = ps.getInteger(0, sv.sceneCount() - 1);
	int frame = ps.getInteger(0, sv.frameCount(scene));
	int count = ps.getHowMany();
	UndoAdd* add = new UndoAdd(sv, scene, frame, count);
	std::string frameName;
	for (int i = 0; i != count; ++i) {
		ps.getString(frameName);
		TemporaryWorkspaceFile twf(frameName.c_str(),
				TemporaryWorkspaceFile::alreadyAWorkspaceFile);
		add->addFrame(new Frame(twf));
	}
	return add;
}
