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
		return 0;
	int fs = ps.getInteger(0, sceneCount - 1);
	int framesInScene = sv.frameCount(fs);
	if (framesInScene == 0)
		return 0;
	int ff = ps.getInteger(0, framesInScene - 1);
	int fc = ps.getInteger(0, framesInScene - ff);
	int ts = ps.getInteger(0, sceneCount - 1);
	int tf = ps.getInteger(0, sv.frameCount(ts));
	return new CommandMove(sv, fs, ff, fc, ts, tf);
}
