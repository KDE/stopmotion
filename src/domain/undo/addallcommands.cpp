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

#include "addallcommands.h"

#include <memory>
#include <utility>

#include "commandadd.h"
#include "commandremove.h"
#include "commandmove.h"
#include "commandaddsound.h"
#include "commandremovesound.h"
#include "commandrenamesound.h"
#include "commandaddscene.h"
#include "commandremovescene.h"
#include "commandmovescene.h"
#include "commandsetimage.h"
#include "src/domain/undo/command.h"
#include "src/domain/undo/executor.h"


class AnimationImpl;

const char* Commands::addFrames = "add-frame";
const char* Commands::removeFrames = "delete-frame";
const char* Commands::moveFrames = "move-frame";
const char* Commands::setImage = "set-image";
const char* Commands::addSound = "add-sound";
const char* Commands::removeSound = "delete-sound";
const char* Commands::renameSound = "rename-sound";
const char* Commands::addScene = "new-scene";
const char* Commands::removeScene = "delete-scene";
const char* Commands::moveScene = "move-scene";

template<typename COMMAND_FACTORY> void addCommand(Executor& ex, AnimationImpl& model,
		const char* name, bool constructive) {
	std::unique_ptr<CommandFactory> c(new COMMAND_FACTORY(model));
	ex.addCommand(name, std::move(c), constructive);
}

Executor* makeAnimationCommandExecutor(AnimationImpl& model) {
	std::unique_ptr<Executor> ex(makeExecutor());
	addCommand<CommandAddFactory>(*ex, model, Commands::addFrames, true);
	addCommand<CommandRemoveFactory>(*ex, model, Commands::removeFrames, false);
	addCommand<CommandMoveFactory>(*ex, model, Commands::moveFrames, false);
	addCommand<CommandSetImageFactory>(*ex, model, Commands::setImage, false);
	addCommand<CommandAddSoundFactory>(*ex, model, Commands::addSound, true);
	addCommand<UndoRemoveSoundFactory>(*ex, model, Commands::removeSound, false);
	addCommand<CommandRenameSoundFactory>(*ex, model, Commands::renameSound, false);
	addCommand<CommandAddSceneFactory>(*ex, model, Commands::addScene, true);
	addCommand<UndoRemoveSceneFactory>(*ex, model, Commands::removeScene, false);
	addCommand<CommandMoveSceneFactory>(*ex, model, Commands::moveScene, false);
	return ex.release();
}

