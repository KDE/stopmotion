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

#include "workspacefile.h"

#include "src/technical/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include <memory.h>
#include <dirent.h>

namespace {

uint32_t fileNum;
uint32_t soundNumber;

uint32_t nextFileNumber() {
	return ++fileNum;
}

class WorkspacePath {
};

WorkspacePath workspacePath;

std::ostream& operator<<(std::ostream& s, WorkspacePath) {
	s << getenv("HOME");
	s << "/.stopmotion/tmp/";
	return s;
}

/**
 * Gets a fresh filename in the workspace that doesn't clash with any other
 * file.
 * @param [out] path Will get a @code{.cpp} new char[] @endcode containing
 * the full path to the new file.
 * @param [out] namePart Will get a pointer into @a path that points to the
 * basename part of the path.
 * @param [in] extension Characters that must come at the end of the filename,
 * for example ".jpg".
 */
void getFreshFilename(char*& path, const char*& namePart,
		const char* extension) {
	std::string pathOut;
	std::stringstream p;
	int indexOfName = 0;
	int size = 0;
	do {
		p.str("");
		p << workspacePath;
		indexOfName = p.str().length();
		p.fill('0');
		p.width(8);
		p << nextFileNumber();
		if (extension)
			p << extension;
		pathOut = p.str();
		size = pathOut.length() + 1;
		// keep going until we find a filename that doesn't already exist.
	} while (0 == access(pathOut.c_str(), F_OK));
	path = new char[size];
	strncpy(path, pathOut.c_str(), size);
	namePart = path + indexOfName;
}

void ensurePathExists(const char* path) {
	if (0 == access(path, F_OK))
		return;
	std::string copy(path);
	copy.c_str();  // ensure terminating '\0'
	char* parent = &copy[0];
	char* end = strrchr(parent, '/');
	if (end && end[1] == '\0') {
		// there is a trailing '/', so let's remove it and try again.
		*end = '\0';
		end = strrchr(parent, '/');
	}
	if (end) {
		*end = '\0';
		ensurePathExists(parent);
		if (mkdir(path, 0755) < 0)
			throw WorkspaceDirectoryCreationException();
	} else {
		throw WorkspaceDirectoryCreationException();
	}
}

void deleteAllFilesIn(const char* path) {
	DIR* dir = opendir(path);
	if (!dir)
		throw WorkspaceDirectoryCreationException();
	struct dirent* ent = 0;
	std::string file;
	while (0 != (ent = readdir(dir))) {
		switch (ent->d_type) {
		case DT_DIR:
			// ignore directories for now
			break;
		default:
			// delete all other types of directory entry
			file = path;
			file.append(1, '/');
			file.append(ent->d_name);
			unlink(file.c_str());
			break;
		}
	}
	closedir(dir);
}

/**
 * Returns a freshly-allocated {@c char[]} of the workspace file with basename
 * {@a basenameIn}.
 * @param basenameOut Pointer to the suffix of the buffer that matches
 * {@a basenameIn}.
 * @param basenameIn The name of the file.
 * @return The newly-allocated buffer.
 */
char* getWorkspaceFilename(const char *&basenameOut, const char* basenameIn) {
	std::stringstream p;
	p.str("");
	p << workspacePath;
	int indexOfName = p.str().length();
	p << basenameIn;
	std::string out = p.str();
	const char* cp = out.c_str();
	int size = out.length() + 1;
	char* fullPath = new char[size];
	strncpy(fullPath, cp, size);
	basenameOut = fullPath + indexOfName;
	return fullPath;
}

}

void WorkspaceFile::clear() {
	std::stringstream pathStr;
	pathStr << workspacePath;
	std::string path = pathStr.str();
	ensurePathExists(path.c_str());
	deleteAllFilesIn(path.c_str());
	fileNum = 0;
	soundNumber = 0;
}

uint32_t WorkspaceFile::getSoundNumber() {
	return soundNumber;
}

void WorkspaceFile::nextSoundNumber() {
	++soundNumber;
}

WorkspaceFile::WorkspaceFile()
		: fullPath(0), namePart(0) {
}

WorkspaceFile& WorkspaceFile::operator=(const WorkspaceFile& other) {
	if (other.fullPath) {
		int nameStart = other.namePart - other.fullPath;
		size_t length = 1 + nameStart + strlen(other.namePart);
		char* buffer = new char[length];
		delete[] fullPath;
		fullPath = buffer;
		strncpy(fullPath, other.fullPath, length);
		namePart = fullPath + nameStart;
	}
	return *this;
}

WorkspaceFile::WorkspaceFile(const WorkspaceFile& t)
		: fullPath(0), namePart(0) {
	*this = t;
}

WorkspaceFile::WorkspaceFile(const char* name)
		: fullPath(0), namePart(0) {
	fullPath = getWorkspaceFilename(namePart, name);
}

WorkspaceFile::WorkspaceFile(NewModelFile)
		: fullPath(0), namePart(0) {
	fullPath = getWorkspaceFilename(namePart, "new.dat");
}

WorkspaceFile::WorkspaceFile(CurrentModelFile)
		: fullPath(0), namePart(0) {
	fullPath = getWorkspaceFilename(namePart, "current.dat");
}

WorkspaceFile::WorkspaceFile(CommandLogFile)
		: fullPath(0), namePart(0) {
	fullPath = getWorkspaceFilename(namePart, "command.log");
}

WorkspaceFile::~WorkspaceFile() {
	delete[] fullPath;
}

const char* WorkspaceFile::basename() const {
	return namePart;
}

const char* WorkspaceFile::path() const {
	return fullPath;
}

void WorkspaceFile::swap(WorkspaceFile& w) {
	char* t = w.fullPath;
	w.fullPath = fullPath;
	fullPath = t;
	const char* t0 = w.namePart;
	w.namePart = namePart;
	namePart = t0;
}

void TemporaryWorkspaceFile::copyToWorkspace(const char* filename) {
	const char* extension = strrchr(filename,'.');
	getFreshFilename(fullPath, namePart, extension);
	toBeDeleted = false;
	if (!Util::copyFile(fullPath, filename)) {
		throw CopyFailedException();
	}
	toBeDeleted = true;
}

TemporaryWorkspaceFile::TemporaryWorkspaceFile(const char* filename)
		: fullPath(0), namePart(0), toBeDeleted(false) {
	// not a totally fullproof test...
	if (strstr(filename, "/.stopmotion/tmp/") != NULL) {
		// Already a workspace file; no need to copy it again
		int size = strlen(filename) + 1;
		fullPath = new char[size];
		strncpy(fullPath, filename, size);
		namePart = strrchr(fullPath,'/') + 1;
	} else {
		copyToWorkspace(filename);
	}
}

TemporaryWorkspaceFile::TemporaryWorkspaceFile(const char* filename,
		ForceCopy)
		: fullPath(0), namePart(0), toBeDeleted(false) {
	copyToWorkspace(filename);
}

TemporaryWorkspaceFile::~TemporaryWorkspaceFile() {
	if (toBeDeleted) {
		unlink(fullPath);
	}
	delete[] fullPath;
	fullPath = 0;
	namePart = 0;
}

CopyFailedException::CopyFailedException() {
}

const char* CopyFailedException::what() const _GLIBCXX_USE_NOEXCEPT {
	return "Failed to copy file to workspace directory (~/.stopmotion/tmp).";
}

const char* WorkspaceDirectoryCreationException::what() const
		_GLIBCXX_USE_NOEXCEPT {
	return "Failed to create workspace directory (~/.stopmotion/tmp).";
}
