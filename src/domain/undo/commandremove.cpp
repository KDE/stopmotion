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

#include "commandremove.h"

#include <stdint.h>
#include <memory>
#include <vector>

#include "commandadd.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/undo/command.h"

class ErrorHandler;
class Frame;

CommandRemove::CommandRemove(AnimationImpl& model,
		int scene, int fromFrame, int count)
		: sv(model), sc(scene), fr(fromFrame), frameCount(count) {
}

CommandRemove::~CommandRemove() {
}

Command* CommandRemove::execute() {
	std::unique_ptr<CommandAdd> inv(new CommandAdd(sv, sc, fr, frameCount));
	std::vector<Frame*> removed;
	sv.removeFrames(sc, fr, frameCount, removed);
	for (std::vector<Frame*>::iterator i = removed.begin();
			i != removed.end(); ++i) {
		inv->addFrame(*i);
	}
	delete this;
	return inv.release();
}

CommandRemoveFactory::CommandRemoveFactory(AnimationImpl& model)
		: sv(model) {
}

CommandRemoveFactory::~CommandRemoveFactory() {
}

Command* CommandRemoveFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return 0;
	int32_t scene = ps.getInteger(0, sceneCount - 1);
	int frameCount = sv.frameCount(scene);
	if (frameCount == 0)
		return 0;
	int32_t frame = ps.getInteger(0, frameCount - 1);
	int32_t count = ps.getHowMany();
	return new CommandRemove(sv, scene, frame, count);
}
