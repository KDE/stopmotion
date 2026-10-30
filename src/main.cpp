/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <cstddef>
#include <cerrno>
#include <exception>
#include <iostream>

class CouldNotInitializeRecoveryFiles : public std::exception {
	char buffer[1024];
public:
	CouldNotInitializeRecoveryFiles(int err) {
		snprintf(buffer, sizeof(buffer),
				"Could not initialize recovery files (%s)."
				" You need permission to read and write in a ~/.stopmotion"
				" directory.", strerror(err));
	}
    const char* what() const noexcept override {
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
    const char* what() const noexcept override {
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
			if (!facadePtr->loadProject(newDat.path(), nullptr)) {
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
		facadePtr = DomainFacade::getFacade();
		NonGUIFrontend nonGUIFrontend(facadePtr);

		try {
			nonGUIFrontend.initializePreferences();
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
	facadePtr = nullptr;
	return ret;
}
