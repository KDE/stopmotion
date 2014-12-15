/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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
#ifndef UTIL_H
#define UTIL_H

#include <exception>
#include <string>
#include <vector>

struct GrabberDevice {
	std::string device;
	std::string name;
	std::string type;
};

class FileLinkException : public std::exception {
	char msg[100];
public:
	FileLinkException(const char* message);
	const char* what() const throw();
};

class DirectoryCreationException : public std::exception {
	char buffer[1024];
public:
	DirectoryCreationException(const char* path);
	const char* what() const throw();
};

class Util {
public:
	/**
	 * Finds the end of the first segment of a bash command-line argument.
	 * This is usually the first space, but can be later in the presence of
	 * quotes or backslashes. Called on a full command line, it will  find the
	 * end of the command, which will be a space, tab or the end of the line.
	 * @param in The command line to check. Ownership is not passed.
	 * @return The rest of the string after the first segment.
	 */
	static const char* endOfArgument(const char* in);
	static bool checkCommand(std::string* pathOut, const char* command);
	static const std::vector<GrabberDevice> getGrabberDevices();
	static bool copyFile(const char *destFileName, const char *srcFileName);
	static void linkOrCopyFile(const char *newName, const char* oldName);
	static bool removeDirectoryContents(const char* path);
	static void ensurePathExists(const char* path);
};

#endif

