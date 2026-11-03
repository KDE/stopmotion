/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "commandadd.h"

#include <cassert>
#include <cstdlib>
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
}

void CommandAdd::addFrame(std::unique_ptr<Frame> frame) {
	frames.push_back(std::move(frame));
}

Command* CommandAdd::execute() {
	std::unique_ptr<CommandRemove> inverse(
			new CommandRemove(sv, scene, frame, frames.size()));
	sv.addFrames(scene, frame, frames);
	// the frames have been moved out
	frames.clear();
	delete this;
	return inverse.release();
}

void CommandAdd::accept(FileNameVisitor& v) const {
	for (const auto& f : frames) {
		f->accept(v);
	}
}

CommandAddFactory::Parameters::Parameters(int scene, int frame, int count)
		: sc(scene), fr(frame), frameCount(count), twfs(nullptr), twfCount(0),
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
		return nullptr;
	int scene = ps.getInteger(0, sceneCount - 1);
	int frame = ps.getInteger(0, sv.frameCount(scene));
	int count = ps.getHowMany();
	std::unique_ptr<CommandAdd> add(new CommandAdd(sv, scene, frame, count));
	std::string frameName;
	for (int i = 0; i != count; ++i) {
		ps.getString(frameName, "?*.jpg");
		WorkspaceFile wf(frameName.c_str());
		add->addFrame(std::make_unique<Frame>(wf));
	}
	return add.release();
}
