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
#include "config.h"
#include "src/domain/domainfacade.h"

#ifdef QTGUI
#include "presentation/frontends/qtfrontend/qtfrontend.h"
#endif

#include "presentation/frontends/nonguifrontend/nonguifrontend.h"
#include "src/foundation/preferencestool.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <stdio.h>
#include <libgen.h>


struct AudioFile {
	unsigned int belongsTo;
	char filename[PATH_MAX];
};

bool init(Frontend *frontend, DomainFacade *facadePtr);
void cleanup();
bool isRecoveryMode();
void createDirectories();
void recover(DomainFacade *facadePtr);


int main(int argc, char **argv) 
{
	atexit(cleanup);
	int ret = 0;
	bool hasCorrectPermissions = true;
	
	// Check if ~./stopmotion directory exists, create it if not
	char tmp[PATH_MAX] = {0};
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) {
		mkdir(tmp, 0755);
	}
	else if (access(tmp, R_OK | W_OK | X_OK) == -1) {
		hasCorrectPermissions = false;
	}
	
	// returns a pointer to the facade (allocated with new)
	DomainFacade *facadePtr = DomainFacade::getFacade();
	
	// if program is started with --nonGUI
	if ( argc > 1 && strcmp(argv[1], "--nonGUI") == 0 ) {
		NonGUIFrontend nonGUIFrontend(facadePtr);
		if (hasCorrectPermissions == false) {
			nonGUIFrontend.reportError(
					"You do not have the necessary permissions to run Stopmotion.\n"
					"You need permission to read, write and execute on ~/.stopmotion", 1);
			delete facadePtr;
			facadePtr = NULL;
			return 1;
		}
		
		facadePtr->registerFrontend(&nonGUIFrontend);
		init(&nonGUIFrontend, facadePtr);
		ret = nonGUIFrontend.run(argc, argv);
	}
	else {
#ifdef QTGUI
		QtFrontend qtFrontend(argc, argv);
		qtFrontend.processEvents();
		if (!hasCorrectPermissions) {
			qtFrontend.reportError(
					"You do not have the necessary permissions to run Stopmotion.\n"
					"You need permission to read, write and execute on ~/.stopmotion", 1);
			delete facadePtr;
			facadePtr = NULL;
			return 1;
		}
		facadePtr->registerFrontend(&qtFrontend);
		bool hasRecovered = init(&qtFrontend, facadePtr);
		if ( hasRecovered == false && argc > 1 && access(argv[1], R_OK) == 0) {
			facadePtr->openProject(argv[1]);
			const char *proFile = facadePtr->getProjectFile();
			if ( proFile != NULL ) {
				PreferencesTool *pref = PreferencesTool::get();
				pref->setPreference("mostRecent", proFile);
			}
		}
		ret = qtFrontend.run(argc, argv);
#endif
	}
	
	delete facadePtr;
	facadePtr = NULL;
	return ret;
}


// creates directories if they already not exist.
bool init(Frontend *frontend, DomainFacade *facadePtr)
{
	if ( isRecoveryMode() ) {
		int ret = frontend->askQuestion(
				"Something caused Stopmotion to exit abnormally\n"
				"last time it was run. Do you want to recover?");
		// The user wants to recover
		if (ret == 0) {
			recover(facadePtr);
			return true;
		}
		else {
			cleanup();
			createDirectories();
		}
	}
	else {
		cleanup();
		createDirectories();
	}
	return false;
}


// cleanup the directories
void cleanup()
{
	char command[PATH_MAX] = {0};
	snprintf( command, sizeof(command), "rm -rf %s/.stopmotion/tmp/", getenv("HOME") );
	system(command);
	snprintf( command, sizeof(command), "rm -rf %s/.stopmotion/trash/", getenv("HOME") );
	system(command);
	snprintf( command, sizeof(command), "rm -rf %s/.stopmotion/packer/", getenv("HOME") );
	system(command);
}


bool isRecoveryMode()
{
	char tmp[PATH_MAX] = {0};
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/tmp/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/trash/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/packer/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	
	// Everything is intact and we have to run in recovery mode
	return true;
}


void createDirectories()
{
	char tmp[PATH_MAX] = {0};
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/tmp/", getenv("HOME") );
	mkdir(tmp, 0755);
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/trash/", getenv("HOME") );
	mkdir(tmp, 0755);
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/packer/", getenv("HOME") );
	mkdir(tmp, 0755);
}


void recover(DomainFacade *facadePtr)
{
	char tmp[PATH_MAX] = {0};
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/packer/", getenv("HOME") );
	
	DIR *dp = opendir(tmp);
	if (dp) {
		struct dirent *ep;
		struct stat st;

		while ( (ep = readdir(dp)) ) {
			stat(ep->d_name, &st);
			// is a directory and not a '.' or '..' directory
			if ( S_ISDIR(st.st_mode) != 0 && ep->d_name[0] != '.') {
				PreferencesTool *pref = PreferencesTool::get();
				const char *mostRecent = pref->getPreference("mostRecent", "");
				if ( strstr(mostRecent, ep->d_name) != NULL && 
							access(mostRecent, F_OK) == 0) {
					facadePtr->openProject(mostRecent);
				}
				if (strcmp(mostRecent, "") != 0) {
					xmlFree((xmlChar*)mostRecent);
				}
				break;
			}
		}
		closedir(dp);
	}
	
	snprintf( tmp, sizeof(tmp), "%s/.stopmotion/tmp", getenv("HOME") );
	dp = opendir(tmp);
	if (dp) {
		vector<char*> frames;
		vector<AudioFile> sounds;
		struct dirent *ep;
		struct stat st;

		while ( (ep = readdir(dp)) ) {
			char *fileName = new char[PATH_MAX];
			snprintf(fileName, sizeof(fileName), "%s/%s", tmp, ep->d_name);
			stat(fileName, &st);
			// Is a regular file, not a directory
			if ( S_ISREG(st.st_mode) != 0) {
				// Image file
				if ( strstr(fileName, "snd") == NULL ) {
					frames.push_back(fileName);
				}
				// Sound file
				else {
					char buf[PATH_MAX] = {0};
					char index[PATH_MAX] = {0};
					
					char *firstDelim = strchr(fileName, '_');
					strcpy(buf, firstDelim + 1);
					char *secondDelim = strchr(buf, '_');
					strncpy(index, buf, strlen(buf) - strlen(secondDelim));
					
					AudioFile af;
					af.belongsTo = (unsigned int)atoi(index);
					strcpy(af.filename, fileName);
					sounds.push_back(af);
					delete [] fileName;
					fileName = NULL;
				}
			}
			else {
				delete [] fileName;
				fileName = NULL;
			}
		}
		closedir(dp);
		
		if (frames.size() <= 0)
			return;
		
		vector<char*>(frames).swap(frames);
		facadePtr->addFrames(frames);

		unsigned int numElem = frames.size();
		for (unsigned int i = 0; i < numElem; ++i) {
			delete [] frames[i];
			frames[i] = NULL;
		}
		
		numElem = sounds.size();
		for (unsigned int j = 0; j < numElem; ++j) {
			facadePtr->addSound(sounds[j].belongsTo, sounds[j].filename);
		}
	}
}


