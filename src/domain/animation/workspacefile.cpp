/***************************************************************************
 *   Copyright (C) 2013-2017 by Linuxstopmotion contributors;              *
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
#include "src/foundation/uiexception.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <string>

namespace {

uint32_t fileNum;
uint32_t soundNumber;

uint32_t nextFileNumber() {
	return ++fileNum;
}

enum WorkspacePath { workspacePath };
enum WorkspacePathFrames { workspacePathFrames };

std::ostream& operator<<(std::ostream& s, WorkspacePath) {
	s << getenv("HOME");
	s << "/.stopmotion/";
	return s;
}

std::ostream& operator<<(std::ostream& s, WorkspacePathFrames) {
	s << getenv("HOME");
	s << "/.stopmotion/frames/";
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
		p << workspacePathFrames;
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

char upper(char c) {
	return 'a' <= c && c <= 'z'? c - ('a' - 'A') : c;
}

/** ASCII case-insensitive string compare. */
bool asciiEqualI(const char* a, const char* b) {
	for (; *a != '\0' && *b != '\0'; ++a, ++b) {
		char ca = upper(*a);
		char cb = upper(*b);
		if (ca != cb)
			return false;
	}
	return true;
}

}

WorkspaceFileType::~WorkspaceFileType() {
}

class ImageFileType : public WorkspaceFileType {
public:
	static ImageFileType* instance;
	const char* preferredExtension(const char*) const {
		return ".jpg";
	}
	bool isType(const char* path) const {
		const char* extension = strrchr(path,'.');
		return extension &&
				(asciiEqualI(extension, ".jpg") || asciiEqualI(extension, ".jpeg"));
	}
};

class SoundFileType : public WorkspaceFileType {
public:
	static SoundFileType* instance;
	const char* preferredExtension(const char*) const {
		return ".ogg";
	}
	bool isType(const char* path) const {
		const char* extension = strrchr(path,'.');
		return extension &&
			(asciiEqualI(extension, ".ogg")
			|| asciiEqualI(extension, ".test-sound"));
	}
};

ImageFileType* ImageFileType::instance;
SoundFileType* SoundFileType::instance;

const WorkspaceFileType& WorkspaceFileType::image() {
	if (!ImageFileType::instance)
		ImageFileType::instance = new ImageFileType();
	return *ImageFileType::instance;
}

const WorkspaceFileType& WorkspaceFileType::sound() {
	if (!SoundFileType::instance)
		SoundFileType::instance = new SoundFileType();
	return *SoundFileType::instance;
}

/**
 * Sets @c fullPath and namePart.
 * @param basename The name of the file.
 * @param inFrames True for a sound or image file.
 */
void WorkspaceFile::setFilename(const char* basename, bool inFrames) {
	namePart = 0;
	delete[] fullPath;
	fullPath = 0;
	std::stringstream p;
	if (p.fail())
		throw UiException(UiException::failedToCopyFilesToWorkspace);
	if (inFrames) {
		p << workspacePathFrames;
	} else {
		p << workspacePath;
	}
	int indexOfName = p.tellp();
	p << basename;
	int size = p.tellp();
	if (p.fail())
		throw UiException(UiException::failedToCopyFilesToWorkspace);
	fullPath = new char[size + 1];
	p.read(fullPath, size);
	if (p.fail())
		throw UiException(UiException::failedToCopyFilesToWorkspace);
	fullPath[size] = '\0';
	namePart = fullPath + indexOfName;
}

void WorkspaceFile::ensureStopmotionDirectoriesExist(AndClear clear) {
	{
		std::stringstream pathStr;
		pathStr << workspacePath;
		std::string path = pathStr.str();
		Util::ensurePathExists(path.c_str());
	}
	{
		std::stringstream pathStr;
		pathStr << workspacePathFrames;
		std::string path = pathStr.str();
		Util::ensurePathExists(path.c_str());
		if (clear == andClear) {
			Util::removeDirectoryContents(path.c_str());
		}
	}
}

void WorkspaceFile::clear() {
	ensureStopmotionDirectoriesExist(andClear);
	WorkspaceFile nm(newModelFile);
	unlink(nm.path());
	WorkspaceFile cm(currentModelFile);
	unlink(cm.path());
	WorkspaceFile cl(commandLogFile);
	unlink(cl.path());
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
	setFilename(name, true);
}

WorkspaceFile::WorkspaceFile(NewModelFile)
		: fullPath(0), namePart(0) {
	setFilename("new.dat");
}

WorkspaceFile::WorkspaceFile(CurrentModelFile)
		: fullPath(0), namePart(0) {
	setFilename("current.dat");
}

WorkspaceFile::WorkspaceFile(CommandLogFile)
		: fullPath(0), namePart(0) {
	setFilename("command.log");
}

WorkspaceFile::WorkspaceFile(CapturedImage)
	: fullPath(0), namePart(0) {
	setFilename("capturedfile.jpg");
}

WorkspaceFile::WorkspaceFile(PreferencesFile)
	: fullPath(0), namePart(0) {
	setFilename("preferences.xml");
}

WorkspaceFile::WorkspaceFile(PreferencesFileOld)
	: fullPath(0), namePart(0) {
	setFilename("preferences.xml.OLD");
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

void TemporaryWorkspaceFile::copyToWorkspace(const char* filename,
		const WorkspaceFileType& type) {
	if (!type.isType(filename)) {
		throw UiException(UiException::unsupportedImageType);
	}
	getFreshFilename(fullPath, namePart, type.preferredExtension(filename));
	toBeDeleted = false;
	if (!Util::copyFile(fullPath, filename)) {
		throw UiException(UiException::failedToCopyFilesToWorkspace);
	}
	toBeDeleted = true;
}

TemporaryWorkspaceFile::TemporaryWorkspaceFile(const char* filename,
		const WorkspaceFileType& type)
		: fullPath(0), namePart(0), toBeDeleted(false) {
	// not a totally foolproof test...
	if (strstr(filename, "/.stopmotion/frames/") != NULL) {
		// Already a workspace file; no need to copy it again
		int size = strlen(filename) + 1;
		fullPath = new char[size];
		strncpy(fullPath, filename, size);
		namePart = strrchr(fullPath,'/') + 1;
	} else {
		copyToWorkspace(filename, type);
	}
}

TemporaryWorkspaceFile::TemporaryWorkspaceFile(const char* filename,
		const WorkspaceFileType& type, ForceCopy)
		: fullPath(0), namePart(0), toBeDeleted(false) {
	copyToWorkspace(filename, type);
}

TemporaryWorkspaceFile::~TemporaryWorkspaceFile() {
	if (toBeDeleted) {
		unlink(fullPath);
	}
	delete[] fullPath;
	fullPath = 0;
	namePart = 0;
}

ExportDirectory::ExportDirectory() : p(0) {
	std::stringstream s;
	s << getenv("HOME");
	s << "/.stopmotion/export/";
	std::string pathOut = s.str();
	pathOut.c_str();  // force allocation of trailing '\0'
	p = new char[pathOut.length() + 1];
	strcpy(p, pathOut.c_str());
}

ExportDirectory::~ExportDirectory() {
	delete[] p;
}

void ExportDirectory::makeEmpty() {
	//TODO
}
