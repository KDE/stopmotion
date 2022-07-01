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
#ifndef COMMANDADD_H
#define COMMANDADD_H

#include "command.h"

#include <stdint.h>
#include <string>
#include <vector>

class ErrorHandler;
class FileNameVisitor;
class Frame;
class AnimationImpl;
class TemporaryWorkspaceFile;

class CommandAdd : public Command {
public:
	/**
	 * @param count The number of frames to reserve; {@ref addFrame} can
	 * subsequently be called this many times without throwing an exception.
	 */
	CommandAdd(AnimationImpl& model, int toScene, int toFrame, int count);
	~CommandAdd();
	/**
	 * Adds a frame to the add command.
	 * @param frame Ownership is passed.
	 */
	void addFrame(Frame* frame);
	Command* execute();
	void accept(FileNameVisitor& v) const;
private:
	AnimationImpl& sv;
	std::vector<Frame*> frames;
	int scene;
	int frame;
};

class CommandAddFactory : public CommandFactory {
	AnimationImpl& sv;
public:
	CommandAddFactory(AnimationImpl& model);
	~CommandAddFactory();
	Command* create(::Parameters& ps, ErrorHandler& e);
	class Parameters : public ::Parameters {
		int32_t sc;
		int32_t fr;
		int32_t frameCount;
		TemporaryWorkspaceFile* twfs;
		int32_t twfCount;
		int32_t parameterCount;
	public:
		Parameters(int scene, int frame, int count);
		~Parameters();
		/**
		 * Add a frame.
		 * @filename The filename of the image. Ownership is not passed.
		 * @return The full path of the workspace file created.
		 */
		const char* addFrame(const char* filename);
		int32_t getInteger(int32_t min, int32_t max);
		int32_t getHowMany();
		void getString(std::string& out, const char* pattern);
		/**
		 * If this function is not called before destruction, copied files
		 * held by this function are deleted.
		 */
		void retainFiles();
	};
};

#endif
