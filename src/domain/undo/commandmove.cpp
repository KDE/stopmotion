/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commandmove.h"

#include "src/domain/animation/animationimpl.h"
#include "src/domain/undo/command.h"

class ErrorHandler;

CommandMove::CommandMove(AnimationImpl& model, int fromScene, int fromFrame, int count,
		int toScene, int toFrame)
		: sv(model), fromSc(fromScene), fromFr(fromFrame), frameCount(count),
		  toSc(toScene), toFr(toFrame) {
}

CommandMove::~CommandMove() {
}

template<typename T> void swap(T& a, T& b) {
	T t(a);
	a = b;
	b = t;
}

Command* CommandMove::execute() {
	sv.moveFrames(fromSc, fromFr, frameCount, toSc, toFr);
	if (fromSc != toSc) {
		swap(fromSc, toSc);
		swap(fromFr, toFr);
	} else if (fromFr + frameCount < toFr) {
		int t = toFr;
		toFr = fromFr;
		fromFr = t - frameCount;
	} else if (toFr < fromFr) {
		int t = toFr;
		toFr = fromFr + frameCount;
		fromFr = t;
	}
	// else it is a command that does nothing; which is it's own inverse!
	return this;
}

CommandMoveFactory::CommandMoveFactory(AnimationImpl& model) : sv(model) {
}

CommandMoveFactory::~CommandMoveFactory() {
}

Command* CommandMoveFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return nullptr;
	int fs = ps.getInteger(0, sceneCount - 1);
	int framesInScene = sv.frameCount(fs);
	if (framesInScene == 0)
		return nullptr;
	int ff = ps.getInteger(0, framesInScene - 1);
	int fc = ps.getInteger(0, framesInScene - ff);
	int ts = ps.getInteger(0, sceneCount - 1);
	int tf = ps.getInteger(0, sv.frameCount(ts));
	return new CommandMove(sv, fs, ff, fc, ts, tf);
}
