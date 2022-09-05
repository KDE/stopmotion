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
#ifndef NONGUI_H
#define NONGUI_H

#include <string>

#include "src/config.h"
#include "src/presentation/frontends/frontend.h"

class DomainFacade;
class UiException;

/**
 * Frontend for using the program through command line options.
 * 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class NonGUIFrontend : public Frontend
{
public:
    /**
     * Constructs and initializes the object.
     * @param facadePtr pointer to the domain facade
     */
    NonGUIFrontend(DomainFacade *facadePtr);
	virtual ~NonGUIFrontend();
	
	/**
	 * The run function for starting the application.
	 * @param argc the argc argument from the environment through main.
	 * @param argv the argv arguments from the environment through main.
	 * @return the return status on exit
	 */
	int run(int argc, char **argv);
	
	void showProgress(ProgressMessage message, int numOperations);
	void hideProgress();
	void updateProgress(int numOperationsDone);
	void setProgressInfo(const char *infoText);
	bool isOperationAborted();
	
	/**
	 * Function for processing events. This is useful on timeconsuming
	 * operations which aren't running in separate processes or threads.
	 * This function is actually not needed in this frontend.
	 */
	void processEvents();
	bool askQuestion(Question question);
	int runExternalCommand(const char *command);
	void reportWarning(const char *message);
	void handleException(UiException&);
	
private:
	DomainFacade *facadePtr;
	
	int parseArguments(int argc, char **argv);
	void addFrames(const char *directory);
	void save(const char *directory);
	/**
	 * Sets {@a out} to be the absolute path of {@a path}, always with a
	 * trailing backslash.
	 * @param[out] out Receives the path returned.
	 * @param[in] path The path to make absolute.
	 */
	void getAbsolutePath(std::string& out, const char *path);
	int checkFiles(const char *directory);
};

#endif
