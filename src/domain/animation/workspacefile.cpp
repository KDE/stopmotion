/*
 * SPDX-FileCopyrightText: 2013-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "workspacefile.h"

#include "src/technical/util.h"
#include "src/foundation/uiexception.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
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
 * @param [out] path Will get the full path to the new file.
 * @param [out] nameStart Will get the index of the basename within @a path.
 * @param [in] extension Characters that must come at the end of the filename,
 * for example ".jpg".
 */
void getFreshFilename(std::string& path, std::string::size_type& nameStart,
		const char* extension) {
	std::stringstream p;
	do {
		p.str("");
		p << workspacePathFrames;
		nameStart = p.str().length();
		p.fill('0');
		p.width(8);
		p << nextFileNumber();
		if (extension)
			p << extension;
		path = p.str();
		// keep going until we find a filename that doesn't already exist.
	} while (0 == access(path.c_str(), F_OK));
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
	const char* preferredExtension(const char*) const override {
		return ".jpg";
	}
	bool isType(const char* path) const override {
		const char* extension = strrchr(path,'.');
		return extension &&
				(asciiEqualI(extension, ".jpg") || asciiEqualI(extension, ".jpeg"));
	}
};

class SoundFileType : public WorkspaceFileType {
public:
	static SoundFileType* instance;
	const char* preferredExtension(const char*) const override {
		return ".ogg";
	}
	bool isType(const char* path) const override {
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
 * Sets @c fullPath and @c nameStart.
 * @param basename The name of the file.
 * @param inFrames True for a sound or image file.
 */
void WorkspaceFile::setFilename(const char* basename, bool inFrames) {
	std::stringstream p;
	if (inFrames) {
		p << workspacePathFrames;
	} else {
		p << workspacePath;
	}
	nameStart = p.tellp();
	p << basename;
	if (p.fail())
		throw UiException(UiException::failedToCopyFilesToWorkspace);
	fullPath = p.str();
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

WorkspaceFile::WorkspaceFile() {
}

WorkspaceFile::WorkspaceFile(const char* name) {
	setFilename(name, true);
}

WorkspaceFile::WorkspaceFile(NewModelFile) {
	setFilename("new.dat");
}

WorkspaceFile::WorkspaceFile(CurrentModelFile) {
	setFilename("current.dat");
}

WorkspaceFile::WorkspaceFile(CommandLogFile) {
	setFilename("command.log");
}

WorkspaceFile::WorkspaceFile(CapturedImage) {
	setFilename("capturedfile.jpg");
}

WorkspaceFile::WorkspaceFile(PreferencesFile) {
	setFilename("preferences.xml");
}

WorkspaceFile::WorkspaceFile(PreferencesFileOld) {
	setFilename("preferences.xml.OLD");
}

WorkspaceFile::~WorkspaceFile() {
}

const char* WorkspaceFile::basename() const {
	return fullPath.empty() ? nullptr : fullPath.c_str() + nameStart;
}

const char* WorkspaceFile::path() const {
	return fullPath.empty() ? nullptr : fullPath.c_str();
}

void WorkspaceFile::swap(WorkspaceFile& w) {
	fullPath.swap(w.fullPath);
	std::swap(nameStart, w.nameStart);
}

void TemporaryWorkspaceFile::copyToWorkspace(const char* filename,
		const WorkspaceFileType& type) {
	if (!type.isType(filename)) {
		throw UiException(UiException::unsupportedImageType);
	}
	getFreshFilename(fullPath, nameStart, type.preferredExtension(filename));
	toBeDeleted = false;
	if (!Util::copyFile(fullPath.c_str(), filename)) {
		fullPath.clear();
		throw UiException(UiException::failedToCopyFilesToWorkspace, filename);
	}
	toBeDeleted = true;
}

TemporaryWorkspaceFile::TemporaryWorkspaceFile(const char* filename,
		const WorkspaceFileType& type)
		: toBeDeleted(false) {
	// not a totally foolproof test...
	if (strstr(filename, "/.stopmotion/frames/") != nullptr) {
		// Already a workspace file; no need to copy it again
		fullPath = filename;
		nameStart = fullPath.rfind('/') + 1;
	} else {
		copyToWorkspace(filename, type);
	}
}

TemporaryWorkspaceFile::TemporaryWorkspaceFile(const char* filename,
		const WorkspaceFileType& type, ForceCopy)
		: toBeDeleted(false) {
	copyToWorkspace(filename, type);
}

TemporaryWorkspaceFile::~TemporaryWorkspaceFile() {
	if (toBeDeleted) {
		unlink(fullPath.c_str());
	}
}

ExportDirectory::ExportDirectory() {
	std::stringstream s;
	s << getenv("HOME");
	s << "/.stopmotion/export/";
	p = s.str();
}

ExportDirectory::~ExportDirectory() {
}

void ExportDirectory::makeEmpty() {
	//TODO
}
