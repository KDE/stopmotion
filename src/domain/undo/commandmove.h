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

#ifndef COMMANDMOVE_H
#define COMMANDMOVE_H

#include "command.h"

#include <stdint.h>

class AnimationImpl;
class ErrorHandler;

class CommandMove : public Command {
	AnimationImpl& sv;
	int32_t fromSc;
	int32_t fromFr;
	int32_t frameCount;
	int32_t toSc;
	int32_t toFr;
public:
	CommandMove(AnimationImpl& model, int fromScene, int fromFrame, int count,
			int toScene, int toFrame);
	~CommandMove();
	Command* execute();
};

class CommandMoveFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandMoveFactory(AnimationImpl& model);
	~CommandMoveFactory();
	Command* create(Parameters& ps, ErrorHandler& e);
};

#endif
