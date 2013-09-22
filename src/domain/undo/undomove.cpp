/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.                   *
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

#include "undomove.h"

	SceneVector sv;
	int fromSc;
	int fromFr;
	int frameCount;
	int toSc;
	int toFr;
UndoMove::UndoMove(SceneVector& model, int fromScene, int fromFrame, int count,
		int toScene, int toFrame)
		: sv(model), fromSc(fromScene), fromFr(fromFrame), frameCount(count),
		  toSc(toScene), toFr(toFrame) {
}

UndoMove::~UndoMove() {
}

template<typename T> void swap(T& a, T& b) {
	T t(a);
	a = b;
	b = t;
}

Command* UndoMove::execute() {
	sv.moveFrames(fromSc, fromFr, frameCount, toSc, toFr);
	swap(fromSc, toSc);
	swap(fromFr, toFr);
	return this;
}

UndoMoveFactory::UndoMoveFactory(SceneVector& model) : sv(model) {
}

UndoMoveFactory::~UndoMoveFactory() {
}

Command* UndoMoveFactory::create(Parameters& ps) {
	int fs = ps.getInteger(0, sv.sceneCount() - 1);
	int framesInScene = sv.frameCount(fs);
	int ff = ps.getInteger(0, framesInScene - 1);
	int fc = ps.getInteger(0, framesInScene - ff);
	int ts = ps.getInteger(0, sv.sceneCount() - 1);
	int tf = ps.getInteger(0, sv.frameCount(ts));
	return UndoMove(fs, ff, fc, ts, tf);
}
