/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMMANDREMOVESOUND_H_
#define COMMANDREMOVESOUND_H_

#include <cstdint>

#include "command.h"

class AnimationImpl;
class ErrorHandler;

class CommandRemoveSound : public Command {
	AnimationImpl& sv;
	int32_t sc;
	int32_t fr;
	int32_t index;
public:
	CommandRemoveSound(AnimationImpl& model, int32_t scene, int32_t frame,
			int32_t soundNumber);
	~CommandRemoveSound() override;
	Command* execute() override;
};

class UndoRemoveSoundFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	UndoRemoveSoundFactory(AnimationImpl& model);
	~UndoRemoveSoundFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
