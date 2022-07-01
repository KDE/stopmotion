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

#include "commandrenamesound.h"

#include <string.h>
#include <memory>
#include <string>

#include "src/domain/animation/animationimpl.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/scene.h"
#include "src/domain/undo/command.h"


CommandRenameSound::CommandRenameSound(AnimationImpl& model, int32_t scene,
		int32_t frame, int32_t soundNumber, const char* newName)
	: sv(model), sc(scene), fr(frame), index(soundNumber), name(newName) {
}

CommandRenameSound::~CommandRenameSound() {
	delete[] name;
}

void CommandRenameSound::setName(const char* newName) {
	int length = strlen(newName) + 1;
	char* nn = new char[length];
	strncpy(nn, newName, length);
	delete[] name;
	name = nn;
}

Command* CommandRenameSound::execute() {
	name = sv.setSoundName(sc, fr, index, name);
	return this;
}

CommandRenameSoundFactory::CommandRenameSoundFactory(AnimationImpl& model) :sv(model) {
}

CommandRenameSoundFactory::~CommandRenameSoundFactory() {
}

Command* CommandRenameSoundFactory::create(Parameters& ps, ErrorHandler&) {
	int sceneCount = sv.sceneCount();
	if (sceneCount == 0)
		return 0;
	int32_t sc = ps.getInteger(0, sceneCount - 1);
	int frameCount = sv.frameCount(sc);
	if (frameCount == 0)
		return 0;
	int32_t fr = ps.getInteger(0, frameCount - 1);
	int soundCount = sv.getScene(sc)->getFrame(fr)->soundCount();
	if (soundCount == 0)
		return 0;
	int32_t index = ps.getInteger(0, soundCount - 1);
	std::unique_ptr<CommandRenameSound> r(new CommandRenameSound(sv, sc, fr, index, 0));
	std::string name;
	ps.getString(name, "sound ?*");
	r->setName(name.c_str());
	return r.release();
}
