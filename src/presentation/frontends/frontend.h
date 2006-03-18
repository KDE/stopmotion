/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
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


/**
 * The frontend interface to be used by the implemented frontend.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
*/
class Frontend
{
public:
	virtual ~Frontend() {}
	
	/**
	 * Abstract function for starting the application through the frontend.
	 * @param argc the argc argument from the environment through main.
	 * @param argv the argv arguments from the environment through main.
	 * @return the return status on exit
	 */
	virtual int run(int argc, char **argv) = 0;
	
	/**
	 * Abstract function for displaying progress on timeconsuming operations.
	 * @param infoText the text to display to the user
	 * @param numOperations the number of calculated operations to do
	 */
	virtual void showProgress(const char *infoText, unsigned int numOperations = 0) = 0;
	
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
	 * Abstract function for changing the information to display to the user
	 * @param infoText the text to display to the user
	 */
	virtual void setProgressInfo(const char *infoText) = 0;
	
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
	 * Abstract function for reporting an error to the user. It has two categories
	 * of errors; warning and critical.
	 * @param message the error message to display to the user
	 * @param id kind of error; 0 for warning, 1 for critical
	 */
	virtual void reportError(const char *message, int id) = 0;
	
	/**
	 * Abstract function for asking the user a yes/no question.
	 * @param question the question to ask
	 * @return 0 if the user answer yes, 1 if no
	 */
	virtual int askQuestion(const char *question) = 0;
};

#endif
