/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
	const char* what() const noexcept override;
};

class DirectoryCreationException : public std::exception {
	char buffer[1024];
public:
	DirectoryCreationException(const char* path);
	const char* what() const noexcept override;
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

