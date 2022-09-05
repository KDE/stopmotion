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
#ifndef FRONTEND_H
#define FRONTEND_H

#include <exception>

class UiException;

/**
 * The frontend interface to be used by the implemented frontend.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
*/
class Frontend {
public:
	enum ProgressMessage {
		connectingCamera,
		importingFramesFromDisk,
		exporting,
		restoringProject,
		savingScenesToDisk
	};
	enum Question {
		useNewerPreferences
	};

	virtual ~Frontend();
	
	/**
	 * Abstract function for starting the application through the frontend.
	 * @param argc the argc argument from the environment through main.
	 * @param argv the argv arguments from the environment through main.
	 * @return the return status on exit
	 */
	virtual int run(int argc, char **argv) = 0;
	
	/**
	 * Abstract function for displaying progress on timeconsuming operations.
	 * @param message Indicates the message to display to the user.
	 * @param numOperations The number of calculated operations to do.
	 */
	virtual void showProgress(ProgressMessage message, int numOperations = 0) = 0;
	
	/**
	 * Abstract function for hiding the progress info.
	 */
	virtual void hideProgress() = 0;
	
	/**
	 * Abstract function for updating the progress.
	 * @param numOperationsDone the number of operations done
	 */
	virtual void updateProgress(int numOperationsDone) = 0;
	
	/**
	 * Abstract function for checking if the user has aborted the operation 
	 * (eg pressed cancel)
	 * @return true if the the operation is aborted, false otherwise
	 */
	virtual bool isOperationAborted() = 0;
	
	/**
	 * Abstract function for processing GUI events. This is useful on timeconsuming
	 * operations which aren't running in separate processes or threads.
	 */
	virtual void processEvents() = 0;

	/**
	 * Displays an error to the user. This sort of error does not crash the program.
	 */
	virtual void reportWarning(const char *message) = 0;

	/**
	 * Displays an error to the user, ending the program if it is serious enough.
	 */
	virtual void handleException(UiException&) = 0;

	/**
	 * Abstract function for asking the user a yes/no question.
	 * @param question The question to ask
	 * @return true if the user answer yes, false if no
	 */
	virtual bool askQuestion(Question question) = 0;

	virtual int runExternalCommand(const char *command) = 0;

	class CriticalError : public std::exception {
	public:
		CriticalError();
		~CriticalError() throw();
		const char* what() const throw();
	};
};

#endif
