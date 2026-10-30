/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMMANDSETIMAGE_H_
#define COMMANDSETIMAGE_H_

#include "command.h"
#include "src/domain/animation/workspacefile.h"

class AnimationImpl;
class ErrorHandler;

class CommandSetImage : public Command {
	friend class CommandDuplicateImage;
	AnimationImpl& sv;
	int32_t sc;
	int32_t fr;
	WorkspaceFile image;
public:
	CommandSetImage(AnimationImpl& model, int32_t scene, int32_t frame,
			WorkspaceFile& w);
	~CommandSetImage() override;
	Command* execute() override;
};

class CommandSetImageFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandSetImageFactory(AnimationImpl& model);
	~CommandSetImageFactory() override;
	Command* create(Parameters& ps, ErrorHandler& e) override;
};

#endif
