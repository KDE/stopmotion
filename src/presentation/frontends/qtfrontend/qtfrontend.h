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
#ifndef QTFRONTEND_H
#define QTFRONTEND_H

#include "src/config.h"
#include "src/presentation/frontends/frontend.h"
#include "mainwindowgui.h"
#include "src/foundation/preferencestool.h"

#include <qprogressdialog.h>
#include <qprogressbar.h>
#include <qtimer.h>
#include <qobject.h>
#include <qapplication.h>


/**
 * Frontend for using the program through a GUI developed with the QT library.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
*/
class QtFrontend : public QObject, public Frontend
{
	Q_OBJECT
public:
	
	/**
	 * Initializing the frontend.
	 * @param argc the argc argument from the environment through main
	 * @param argv argv the argv arguments from the environment through main
	 */
	QtFrontend(int argc, char **argv);
	
	/**
	 * Deallocates allocated memory and sets the pointers to NULL.
	 */
	virtual ~QtFrontend();
	
	/**
	 * The run function for starting the application.
	 * @param argc the argc argument from the environment through main.
	 * @param argv the argv arguments from the environment through main.
	 * @return the return status on exit
	 */
	int run(int argc, char **argv);
	
	/**
	 * Function for displaying progress on timeconsuming operations.
	 * @param infoText the text to display to the user
	 * @param numOperations the number of calculated operations to do
	 */
	void showProgress(const char *infoText, unsigned int numOperations = 0);
	
	/**
	 * Function for hiding the progress info.
	 */
	void hideProgress();

	/**
	 * Function for updating the progress.
	 * @param numOperationsDone the number of operations done
	 */
	void updateProgress(int numOperationsDone);
	
	/**
	 * Function for changing the information to display to the user
	 * @param infoText the text to display to the user
	 */
	void setProgressInfo(const char *infoText);
	
	/**
	 * Function for checking if the user has aborted the operation 
	 * (eg pressed cancel)
	 * @return true if the the operation is aborted, false otherwise
	 */
	bool isOperationAborted();
	
	/**
	 * Function for processing Qt events. This is useful on timeconsuming
	 * operations which aren't running in separate processes or threads.
	 */
	void processEvents();
	
	/**
	 * Function for reporting an error to the user. It has two categories
	 * of errors; warning and critical.
	 * @param message the error message to display to the user
	 * @param id kind of error; 0 for warning, 1 for critical
	 */
	void reportError(const char *message, int id);
	
	/**
	 * Function for asking the user a yes/no question.
	 * @param question the question to ask
	 * @return 0 if the user answer yes, 1 if no
	 */
	int askQuestion(const char *question);
	
protected slots:
	void updateProgressBar();
	
private:
	QApplication *stApp;
	MainWindowGUI *mw;
	QProgressDialog *progressDialog;
	QProgressBar *progressBar;
	QLabel *infoText;
	QTimer *timer;
	
	void initializePreferences();
	void setDefaultPreferences(PreferencesTool *prefs);
};

#endif
