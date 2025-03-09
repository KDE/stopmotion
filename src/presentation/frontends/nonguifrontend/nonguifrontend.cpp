/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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
#include "nonguifrontend.h"

#include "domainfacade.h"
#include "frontends/frontend.h"
#include "uiexception.h"
#include "src/technical/stringiterator.h"

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <sstream>


NonGUIFrontend::NonGUIFrontend(DomainFacade *facadePtr)
		: facadePtr(facadePtr) {
}


NonGUIFrontend::~ NonGUIFrontend() {
}


int NonGUIFrontend::run(int argc, char **argv) {
	printf("NonGUIFrontend starts running ...\n");
	return parseArguments(argc, argv);
}


void NonGUIFrontend::showProgress(ProgressMessage message,
		int /*numOperations*/) {
	// sets the stdout to be unbuffered
	setvbuf(stdout, NULL, _IONBF,0);
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
	setvbuf (stdout, NULL , _IOFBF , 1024);
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
				{"nonGUI", 0, 0, 0},
				{"addFrames", 1, 0, 'a'},
				{"capture", 1, 0, 'c'},
				{"save", 1, 0, 's'},
				{"export", 0, 0, 'e'},
				{0, 0, 0, 0}
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
			while (atEnd()) {
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
		return &buffer[0];
	}
	void next() override {
		if (dp) {
			ep = readdir(dp);
			while (ep && ep->d_type != DT_REG)
				ep = readdir(dp);
			if (ep) {
				buffer.resize(dirLen);
				buffer.append(ep->d_name);
				buffer.c_str();
			}
		}
	}
};

void NonGUIFrontend::addFrames(const char *directory) {
	std::string dir;
	getAbsolutePath(dir, directory);
	DirIterator di(dir.c_str());
	facadePtr->addFrames(0, 0, di);
}


void NonGUIFrontend::save(const char *directory) {
	// returns a absolute path which is allocated with new[]
	std::string dir;
	getAbsolutePath(dir, directory);
	facadePtr->saveProject(dir.c_str());
}


void NonGUIFrontend::getAbsolutePath(std::string& out, const char *path) {
	if (!path || path[0] == '\0') {
		out = "/";
		return;
	}
	// isn't an absolute path
	if (path[0] != '/') {
		// make it absolute
		out = getenv("PWD");
		out.append("/");
		out.append(path);
	} else {
		out = path;
	}
	// ensure it ends in /
	if (out[out.size() - 1] != '/')
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
