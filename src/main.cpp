/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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
#include "config.h"
#include "src/domain/domainfacade.h"

#ifdef QTGUI
#include "presentation/frontends/qtfrontend/qtfrontend.h"
#endif

#include "presentation/frontends/nonguifrontend/nonguifrontend.h"
#include "src/foundation/preferencestool.h"
#include "src/domain/animation/workspacefile.h"
#include "uiexception.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stddef.h>
#include <errno.h>
#include <exception>
#include <iostream>

class CouldNotInitializeRecoveryFiles : public std::exception {
	int error;
	char buffer[1024];
public:
	CouldNotInitializeRecoveryFiles(int err) : error(err) {
		snprintf(buffer, sizeof(buffer),
				"Could not initialize recovery files (%s)."
				" You need permission to read and write in a ~/.stopmotion"
				" directory.", strerror(err));
	}
    const char* what() const throw() {
    	return buffer;
    }
};

class RecoveryIncompleteException : public std::exception {
	char buffer[1024];
public:
	RecoveryIncompleteException(const char* filename) {
		snprintf(buffer, sizeof(buffer),
				"Failed to recover previous project from command log file %s."
				" Perhaps the file is corrupt or you don't have permission"
				" to read it.", filename);
	}
    const char* what() const throw() {
    	return buffer;
    }
};

/**
 * Recover the project. An exception will be throw if recovery fails.
 */
void recover(DomainFacade *facadePtr) {
	struct stat st;
	WorkspaceFile commandLog(WorkspaceFile::commandLogFile);
	bool commandLogExists = 0 <= stat(commandLog.path(), &st);
	WorkspaceFile currentDat(WorkspaceFile::currentModelFile);
	if (stat(currentDat.path(), &st) < 0) {
		WorkspaceFile newDat(WorkspaceFile::newModelFile);
		if (stat(newDat.path(), &st) < 0) {
			if (!commandLogExists) {
				// None of the files are present; clear and start afresh
				WorkspaceFile::clear();
			}
		} else {
			// Program must have crashed in the middle of saving.
			if (unlink(commandLog.path()) < 0 && errno != ENOENT) {
				throw CouldNotInitializeRecoveryFiles(errno);
			}
			commandLogExists = false;
			if (!facadePtr->loadProject(newDat.path(), 0)) {
				throw RecoveryIncompleteException(newDat.path());
			}
		}
	} else {
		Preference projectFile("projectFile");
		if (!facadePtr->loadProject(currentDat.path(), projectFile.get())) {
			throw RecoveryIncompleteException(currentDat.path());
		}
	}
	if (commandLogExists) {
		facadePtr->replayCommandLog(commandLog.path());
	}
}

int main(int argc, char **argv) {
	int ret = 1;
	DomainFacade *facadePtr;
	// if program is started with --nonGUI
	if ( argc > 1 && strcmp(argv[1], "--nonGUI") == 0 ) {
		NonGUIFrontend nonGUIFrontend(facadePtr);
		facadePtr = DomainFacade::getFacade();

		try {
			recover(facadePtr);
			facadePtr->registerFrontend(&nonGUIFrontend);
			ret = nonGUIFrontend.run(argc, argv);
		} catch (std::exception& e) {
			std::cerr << e.what();
		}
	}
	else {
#ifdef QTGUI
		QtFrontend qtFrontend(argc, argv);
		facadePtr = DomainFacade::getFacade();

		qtFrontend.processEvents();
		try {
			facadePtr->registerFrontend(&qtFrontend);
			recover(facadePtr);
			qtFrontend.setUndoRedoEnabled();
			facadePtr->initializeCommandLoggerFile();
			if (argc > 1 && access(argv[1], R_OK) == 0) {
				qtFrontend.openProject(argv[1]);
				facadePtr->setMostRecentProject();
			}
			ret = qtFrontend.run(argc, argv);
		} catch (UiException& e) {
			qtFrontend.handleException(e);
		} catch (std::exception& e) {
			UiException uie(UiException::ArbitraryError, e.what());
			qtFrontend.handleException(uie);
		}
#endif
	}

	delete facadePtr;
	facadePtr = NULL;
	return ret;
}
