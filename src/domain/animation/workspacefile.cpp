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
#include <sstream>
#include <memory.h>

WorkspaceFile::WorkspaceFile(const char* filename)
		: fullPath(0), namePart(0) {
	std::stringstream path;
	path << getenv("HOME");
	path << "/.stopmotion/tmp/";
	int nameStartIndex = path.tellp();
	path << filename;
	unsigned int size = path.tellp() + 1;
	fullPath = new char[size];
	strncpy(fullPath, path.str().c_str(), size);
	namePart = fullPath + nameStartIndex;
}

WorkspaceFile::WorkspaceFile(TemporaryWorkspaceFile t)
		: fullPath(0), namePart(0) {
	*this = t;
}

WorkspaceFile& WorkspaceFile::operator=(TemporaryWorkspaceFile t) {
	unsigned int size = strlen(t.path) + 1;
	delete[] fullPath;
	fullPath = new char[size];
	strncpy(fullPath, t.path, size);
	namePart = fullPath + (t.namePart - t.path);
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
		std::stringstream p;
		p << getenv("HOME");
		p << "/.stopmotion/tmp/";
		int indexOfName = p.tellp();
		p.fill(0);
		p.width(8);
		p <<  nextImageNumber();
		const char* extension = strrchr(path,'.');
		p << extension;
		int size = p.tellp() + 1;
		path = new char[size];
		strncpy(path, p.str().c_str(), size);
		namePart = path + indexOfName;
		Util::copyFile(path, filename);
		toBeDeleted = true;
	}
}

TemporaryWorkspaceFile::~TemporaryWorkspaceFile() {
	if (toBeDeleted) {
		unlink(path);
	}
	delete[] path;
	path = 0;
	namePart = 0;
}
