/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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
#include <unistd.h>
#include <sstream>
#include <memory.h>

namespace {

uint32_t fileNum;

uint32_t nextFileNumber() {
	return ++fileNum;
}

/**
 * Gets a fresh filename in the workspace that doesn't clash with any other
 * file.
 * @param [out] path Will get a @code{.cpp} new char[] @endcode containing
 * the full path to the new file.
 * @param [out] namePart Will get a pointer into @c path that points to the
 * basename part of the path.
 * @param [in] extension Characters that must come at the end of the filename,
 * for example ".jpg".
 */
void getFreshFilename(char*& path, const char*& namePart,
		const char* extension) {
	std::stringstream p;
	int indexOfName = 0;
	do {
		p.str("");
		p << workspacePath;
		indexOfName = p.tellp();
		p.fill(0);
		p.width(8);
		p <<  nextFileNumber();
		p << extension;
		// keep going until we find a filename that doesn't already exist.
	} while (0 != access(p.str().c_str(), F_OK));
	int size = p.tellp() + 1;
	path = new char[size];
	strncpy(path, p.str().c_str(), size);
	namePart = path + indexOfName;
}

class WorkspacePath {
};

WorkspacePath workspacePath;

std::ostream& operator<<(std::ostream& s, WorkspacePath) {
	s << getenv("HOME");
	s << "/.stopmotion/tmp/";
	return s;
}

}

void WorkspaceFile::clear() {
	std::stringstream ps;
	ps << workspacePath;
	const char* path = ps.str().c_str();
	std::stringstream rm;
	rm << "rm -rf " << path;
	system(rm.str().c_str());
	mkdir(path, 0755);
	//TODO what about failure? Probably can only inform the user and close
	fileNum = 0;
}

WorkspaceFile::WorkspaceFile()
		: fullPath(0), namePart(0) {
}

WorkspaceFile::WorkspaceFile(const char* basename)
		: fullPath(0), namePart(0) {
	// prevent new files from clashing with this one.
	// This isn't really necessary, as we ensure that new files don't
	// overwrite old ones, but it will make sure that new files have
	// greater numbers than old ones, which might be nice.
	char* endDigits;
	uint32_t num = static_cast<uint32_t>(
			strtoul(basename, &endDigits, 10));
	if (fileNum < num) {
		fileNum = num;
	}
	std::stringstream p;
	p << workspacePath;
	int indexOfName = p.tellp();
	p << basename;
	int size = p.tellp() + 1;
	fullPath = new char[size];
	strncpy(fullPath, p.str().c_str(), size);
	namePart = fullPath + indexOfName;
}

WorkspaceFile::WorkspaceFile(const char* extension, FreshFilename)
		: fullPath(0), namePart(0) {
	getFreshFilename(fullPath, namePart, extension);
}

WorkspaceFile& WorkspaceFile::operator=(TemporaryWorkspaceFile t) {
	unsigned int size = strlen(t.path) + 1;
	delete[] fullPath;
	fullPath = t.path;
	namePart = t.namePart;
	t.path = 0;
	t.namePart = 0;
	t.toBeDeleted = false;
	return *this;
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

void TemporaryWorkspaceFile::copyToWorkspace(const char* filename) {
	const char* extension = strrchr(filename,'.');
	getFreshFilename(path, namePart, extension);
	Util::copyFile(path, filename);
	toBeDeleted = true;
}

TemporaryWorkspaceFile::TemporaryWorkspaceFile(const char* filename)
		: path(0), namePart(0), toBeDeleted(false) {
	// not a totally fullproof test...
	if (strstr(filename, "/.stopmotion/tmp/") != NULL) {
		// Already a workspace file; no need to copy it again
		int size = strlen(filename) + 1;
		path = new char[size];
		strncpy(path, filename, size);
		namePart = strrchr(path,'/') + 1;
	} else {
		copyToWorkspace(filename);
	}
}

TemporaryWorkspaceFile::TemporaryWorkspaceFile(const char* filename,
		ForceCopy) : path(0), namePart(0), toBeDeleted(false) {
	copyToWorkspace(filename);
}

TemporaryWorkspaceFile::~TemporaryWorkspaceFile() {
	if (toBeDeleted) {
		unlink(path);
	}
	delete[] path;
	path = 0;
	namePart = 0;
}
