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

#include "src/domain/undo/executor.h"

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
#include "commandduplicateimage.h"

class AnimationImpl;

const char* Commands::addFrames = "add-frame";
const char* Commands::removeFrames = "delete-frame";
const char* Commands::moveFrames = "move-frame";
const char* Commands::setImage = "set-image";
const char* Commands::duplicateImage = "duplicate-image";
const char* Commands::addSound = "add-sound";
const char* Commands::removeSound = "delete-sound";
const char* Commands::renameSound = "rename-sound";
const char* Commands::addScene = "new-scene";
const char* Commands::removeScene = "delete-scene";
const char* Commands::moveScene = "move-scene";

Executor* makeAnimationCommandExecutor(AnimationImpl& model) {
	std::auto_ptr<Executor> ex(makeExecutor());
	std::auto_ptr<CommandFactory> add(new CommandAddFactory(model));
	ex->addCommand(Commands::addFrames, add, true);
	std::auto_ptr<CommandFactory> remove(new CommandRemoveFactory(model));
	ex->addCommand(Commands::removeFrames, remove, false);
	std::auto_ptr<CommandFactory> move(new CommandMoveFactory(model));
	ex->addCommand(Commands::moveFrames, move, false);
	std::auto_ptr<CommandFactory> setImage(new CommandSetImageFactory(model));
	ex->addCommand(Commands::setImage, setImage, false);
	std::auto_ptr<CommandFactory> duplicateImage(new CommandDuplicateImageFactory(model));
	ex->addCommand(Commands::duplicateImage, duplicateImage, false);
	std::auto_ptr<CommandFactory> addSound(new CommandAddSoundFactory(model));
	ex->addCommand(Commands::addSound, addSound, true);
	std::auto_ptr<CommandFactory> removeSound(new UndoRemoveSoundFactory(model));
	ex->addCommand(Commands::removeSound, removeSound, true);
	std::auto_ptr<CommandFactory> renameSound(new CommandRenameSoundFactory(model));
	ex->addCommand(Commands::renameSound, renameSound, false);
	std::auto_ptr<CommandFactory> addScene(new CommandAddSceneFactory(model));
	ex->addCommand(Commands::addScene, addScene, true);
	std::auto_ptr<CommandFactory> removeScene(new UndoRemoveSceneFactory(model));
	ex->addCommand(Commands::removeScene, removeScene, false);
	std::auto_ptr<CommandFactory> moveScene(new CommandMoveSceneFactory(model));
	ex->addCommand(Commands::moveScene, moveScene, false);
	return ex.release();
}

