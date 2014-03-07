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
#include "src/domain/animation/workspacefile.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <stdio.h>
#include <libgen.h>

bool recover(DomainFacade *facadePtr);

int main(int argc, char **argv) {
	int ret = 0;
	bool hasCorrectPermissions = true;

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
		bool hasRecovered = recover(facadePtr);
		if (!hasRecovered) {
			// No recovery. Create the workspace afresh.
			WorkspaceFile::clear();
		} else {
			qtFrontend.setUndoRedoEnabled();
		}
		if (!facadePtr->initializeCommandLoggerFile()) {
			// report failure to initialize recovery files
			qtFrontend.reportError(
					"Could not initialize recovery files."
					" Recovery will not be available!", 1);
		}
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

/**
 * Recover the project.
 * @return {@c true} if successful, {@c false} if some file failed to load.
 */
bool recover(DomainFacade *facadePtr) {
	struct stat st;
	WorkspaceFile commandLog(WorkspaceFile::commandLogFile);
	bool readCommandLog = 0 <= stat(commandLog.path(), &st);
	bool loadFailed = false;
	WorkspaceFile currentDat(WorkspaceFile::currentModelFile);
	if (stat(currentDat.path(), &st) < 0) {
		WorkspaceFile newDat(WorkspaceFile::newModelFile);
		if (stat(newDat.path(), &st) < 0) {
			if (!readCommandLog) {
				// None of the files are present; clear and start afresh
				WorkspaceFile::clear();
			}
		} else {
			// recover from new.dat without considering command.log
			readCommandLog = false;
			if (!facadePtr->loadProject(newDat.path())) {
				return false;
			}
		}
	} else {
		if (!facadePtr->loadProject(currentDat.path())) {
			return false;
		}
	}
	if (readCommandLog) {
		facadePtr->replayCommandLog(commandLog.path());
	}
	return true;
}
