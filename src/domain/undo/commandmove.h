/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMMANDMOVE_H
#define COMMANDMOVE_H

#include "command.h"

#include <cstdint>

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
	~CommandMove() override;
	Command* execute() override;
};

class CommandMoveFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandMoveFactory(AnimationImpl& model);
	~CommandMoveFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
