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
#include "commandadd.h"

#include <assert.h>
#include <stdlib.h>
#include <new>
#include <memory>

#include "commandremove.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/workspacefile.h"
#include "src/domain/undo/command.h"

class ErrorHandler;
class FileNameVisitor;

CommandAdd::CommandAdd(AnimationImpl& model, int toScene, int toFrame, int count)
		: sv(model), scene(toScene), frame(toFrame) {
	frames.reserve(count);
}

CommandAdd::~CommandAdd() {
	for (std::vector<Frame*>::iterator i = frames.begin();
			i != frames.end(); ++i) {
		delete *i;
	}
}

void CommandAdd::addFrame(Frame* frame) {
	frames.push_back(frame);
}

Command* CommandAdd::execute() {
	std::unique_ptr<CommandRemove> inverse(
			new CommandRemove(sv, scene, frame, frames.size()));
	sv.addFrames(scene, frame, frames);
	// ownership has been passed, so we must forget the frames
	frames.clear();
	delete this;
	return inverse.release();
}

void CommandAdd::accept(FileNameVisitor& v) const {
	for (std::vector<Frame*>::const_iterator i = frames.begin();
			i != frames.end(); ++i) {
		(*i)->accept(v);
	}
}

CommandAddFactory::Parameters::Parameters(int scene, int frame, int count)
		: sc(scene), fr(frame), frameCount(count), twfs(0), twfCount(0),
		  parameterCount(0) {
	twfs = static_cast<TemporaryWorkspaceFile*>(malloc(
			frameCount * sizeof(TemporaryWorkspaceFile)));
}

CommandAddFactory::Parameters::~Parameters() {
	for (int i = 0; i != twfCount; ++i) {
		twfs[i].~TemporaryWorkspaceFile();
	}
	free(twfs);
}

const char* CommandAddFactory::Parameters::addFrame(const char* filename) {
	assert (twfCount != frameCount);
	TemporaryWorkspaceFile* p = twfs + twfCount;
	new (p) TemporaryWorkspaceFile(filename, WorkspaceFileType::image());
	++twfCount;
	return p->path();
}

int32_t CommandAddFactory::Parameters::getInteger(int32_t, int32_t) {
	assert(parameterCount < 2);
	++parameterCount;
	return parameterCount == 1? sc : fr;
}

int32_t CommandAddFactory::Parameters::getHowMany() {
	assert(parameterCount == 2);
	++parameterCount;
	return frameCount;
}

void CommandAddFactory::Parameters::getString(std::string& out,
		const char*) {
	assert(3 <= parameterCount);
	int index = parameterCount - 3;
	++parameterCount;
	out.assign(twfs[index].basename());
}

void CommandAddFactory::Parameters::retainFiles() {
	for (int i = 0; i != twfCount; ++i) {
		twfs[i].retainFile();
	}
}

CommandAddFactory::CommandAddFactory(AnimationImpl& model) : sv(model) {
}

CommandAddFactory::~CommandAddFactory() {
}

Command* CommandAddFactory::create(::Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return 0;
	int scene = ps.getInteger(0, sceneCount - 1);
	int frame = ps.getInteger(0, sv.frameCount(scene));
	int count = ps.getHowMany();
	CommandAdd* add = new CommandAdd(sv, scene, frame, count);
	std::string frameName;
	for (int i = 0; i != count; ++i) {
		ps.getString(frameName, "?*.jpg");
		WorkspaceFile wf(frameName.c_str());
		add->addFrame(new Frame(wf));
	}
	return add;
}
