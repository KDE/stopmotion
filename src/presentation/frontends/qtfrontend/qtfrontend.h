/***************************************************************************
 *   Copyright (C) 2005-2017 by Linuxstopmotion contributors;              *
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
#ifndef QTFRONTEND_H
#define QTFRONTEND_H

#include "src/presentation/frontends/frontend.h"

#include <QObject>

class PreferencesTool;
class QApplication;
class MainWindowGUI;
class QProgressDialog;
class QProgressBar;
class QLabel;
class QTimer;
class UiException;

/**
 * Frontend for using the program through a GUI developed with the QT library.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
*/
class QtFrontend : public QObject, public Frontend {
	Q_OBJECT
public:

	/**
	 * Initializing the frontend.
	 * @param argc the argc argument from the environment through main
	 * @param argv argv the argv arguments from the environment through main
	 */
	QtFrontend(int &argc, char **argv);

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

	void showProgress(ProgressMessage message, int numOperations);
	void hideProgress();
	void updateProgress(int numOperationsDone);
	void setProgressInfo(const char *infoText);
	bool isOperationAborted();
	void processEvents();
	bool askQuestion(Question question);
	int runExternalCommand(const char *command);
	void handleException(UiException&);
	void reportWarning(const char *message);

	/**
	 * Set the Undo and Redo actions to be enabled or disabled according to the
	 * state of the command history.
	 */
	void setUndoRedoEnabled();

	void openProject(const char* file);
protected slots:
	void updateProgressBar();

private:
	QApplication *stApp;
	MainWindowGUI *mw;
	QProgressDialog *progressDialog;
	QProgressBar *progressBar;
	QLabel *infoText;
	QTimer *timer;
	static const char* VERSION;

	void initializePreferences();
	void setDefaultPreferences(PreferencesTool *prefs);
	void updateOldPreferences(PreferencesTool *prefs);
	bool loadPreferencesFrom(PreferencesTool* prefs, const char* path);
};

#endif
