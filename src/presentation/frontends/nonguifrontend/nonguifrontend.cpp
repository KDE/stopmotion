/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "nonguifrontend.h"

#include "domainfacade.h"
#include "frontends/frontend.h"
#include "uiexception.h"
#include "src/technical/stringiterator.h"
#include "src/foundation/preferencestool.h"
#include "src/domain/animation/workspacefile.h"

#include <cstdio>
#include <getopt.h>
#include <cstdlib>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cerrno>
#include <string>
#include <cstring>
#include <sstream>


NonGUIFrontend::NonGUIFrontend(DomainFacade *facadePtr)
		: facadePtr(facadePtr) {
}


NonGUIFrontend::~ NonGUIFrontend() {
}


void NonGUIFrontend::initializePreferences() {
	PreferencesTool *prefs = PreferencesTool::get();
	WorkspaceFile preferencesFile(WorkspaceFile::preferencesFile);
	WorkspaceFile oldPrefsFile(WorkspaceFile::preferencesFileOld);

	WorkspaceFile::ensureStopmotionDirectoriesExist();

	if (prefs->load(preferencesFile.path())) {
		prefs->setSavePath(preferencesFile.path(), false);
		return;
	}
	// There is no user to ask whether a corrupt preferences file may be
	// overwritten, so refuse to continue instead.
	if (0 == access(preferencesFile.path(), F_OK)) {
		throw UiException(UiException::preferencesFileMalformed,
				preferencesFile.path());
	}
	if (!prefs->load(oldPrefsFile.path()))
		prefs->setDefaultPreferences(STOPMOTION_VERSION);
	prefs->setSavePath(preferencesFile.path(), true);
	prefs->flush();
}


int NonGUIFrontend::run(int argc, char **argv) {
	printf("NonGUIFrontend starts running ...\n");
	return parseArguments(argc, argv);
}


void NonGUIFrontend::showProgress(ProgressMessage message,
		int /*numOperations*/) {
	// sets the stdout to be unbuffered
	setvbuf(stdout, nullptr, _IONBF,0);
	const char* infoText = "Please wait";
	switch (message) {
	case connectingCamera:
		infoText = "Connecting camera";
		break;
	case exporting:
		infoText = "Exporting";
		break;
	case importingFramesFromDisk:
		infoText = "Importing frames from disk";
		break;
	case restoringProject:
		infoText = "Restoring project";
		break;
	case savingScenesToDisk:
		infoText = "Saving scenes to disk";
		break;
	}
	printf("%s...", infoText);
}


void NonGUIFrontend::hideProgress() {
	printf("\nOperation finished!\n");
	// sets the stdout the be buffered
	setvbuf (stdout, nullptr , _IOFBF , 1024);
}


void NonGUIFrontend::updateProgress(int) {
	printf(".");
}


bool NonGUIFrontend::isOperationAborted() {
	return false;
}

void NonGUIFrontend::processEvents() {
}

int NonGUIFrontend::parseArguments(int argc, char **argv) {
	while (1) {
		static struct option longOptions[] =
			{
				{"nonGUI", 0, nullptr, 0},
				{"addFrames", 1, nullptr, 'a'},
				{"capture", 1, nullptr, 'c'},
				{"save", 1, nullptr, 's'},
				{"export", 0, nullptr, 'e'},
				{nullptr, 0, nullptr, 0}
			};

		/* getopt_long  stores the option index here. */
		int optionIndex = 0;
		int c = getopt_long(argc, argv, "a:c:s:e:", longOptions, &optionIndex);

		/* Detect the end of the options. */
		if (c == -1) {
			break;
		}

		switch (c) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				break;

			case 'a':
				//printf ("option --addFrames with argument %s\n", optarg);
				addFrames(optarg);
				break;

			case 'c':
				printf ("option --capture with argument %s\n", optarg);
				break;

			case 'e':
				printf ("option --export with argument %s\n", optarg);
				break;

			case 's':
				//printf ("option --save with argument %s\n", optarg);
				save(optarg);
				break;

			case '?':
				/* getopt_long  already printed an error message. */
				break;

			default:
				return 1;
		}
	}
	return 0;
}

class DirIterator : public StringIterator {
	std::string buffer;
	std::string::size_type dirLen;
	DIR *dp;
	struct dirent *ep;
public:
	DirIterator(const DirIterator&) = delete;
	DirIterator& operator=(const DirIterator&) = delete;
	DirIterator(const char* path)
			: buffer(path), dirLen(0), dp(opendir(path)) {
		dirLen = buffer.size();
		next();
	}
	~DirIterator() override {
		if (dp)
			closedir(dp);
	}
	int count() override {
		int c = 0;
		if (dp) {
			while (!atEnd()) {
				++c;
				next();
			}
			rewinddir(dp);
			next();
		}
		return c;
	}
	bool atEnd() const override {
		return !dp || !ep;
	}
	const char* get() const override {
		return buffer.c_str();
	}
	void next() override {
		if (dp) {
			ep = readdir(dp);
			while (ep && ep->d_type != DT_REG)
				ep = readdir(dp);
			if (ep) {
				buffer.resize(dirLen);
				buffer.append(ep->d_name);
			}
		}
	}
};

void NonGUIFrontend::addFrames(const char *directory) {
	std::string dir;
	getAbsolutePath(dir, directory, true);
	DirIterator di(dir.c_str());
	facadePtr->addFrames(0, 0, di);
}


void NonGUIFrontend::save(const char *projectFile) {
	std::string file;
	getAbsolutePath(file, projectFile, false);
	facadePtr->saveProject(file.c_str());
}


void NonGUIFrontend::getAbsolutePath(std::string& out, const char *path,
		bool isDirectory) {
	if (!path || path[0] == '\0') {
		out = "/";
		return;
	}
	out = path;
	// isn't an absolute path
	if (path[0] != '/') {
		// make it absolute; $PWD may be unset or stale, so ask the system.
		// If that fails the relative path still works.
		char* cwd = getcwd(nullptr, 0);
		if (cwd) {
			out = cwd;
			free(cwd);
			out.append("/");
			out.append(path);
		}
	}
	// ensure a directory ends in /
	if (isDirectory && out[out.size() - 1] != '/')
		out.append("/");
}


int NonGUIFrontend::checkFiles(const char *directory) {
	int numSuccessful = 0;
	DIR *dp = opendir(directory);

	if (dp) {
		struct dirent *ep;
		struct stat st;
		std::stringstream path;

		while ( (ep = readdir(dp)) ) {
			path.str("");
			path << directory << ep->d_name;
			std::string p(path.str());
			stat(p.c_str(), &st);
			// is a regular file, not a directory
			if ( S_ISREG(st.st_mode) != 0) {
				++numSuccessful;
			}
		}
		closedir(dp);
	}
	else {
		fprintf (stderr, "Couldn't open directory: %s; %s\n",directory, strerror (errno));
	}

	return numSuccessful;
}


void NonGUIFrontend::handleException(UiException& e) {
	printf("%s\n", e.what());
	if (e.warning() == UiException::IsError)
		throw CriticalError();
}


void NonGUIFrontend::reportWarning(const char *message) {
	printf("Warning: %s\n", message);
}


bool NonGUIFrontend::askQuestion(Question) {
	return false;
}


int NonGUIFrontend::runExternalCommand(const char *) {
	return 1;
}
