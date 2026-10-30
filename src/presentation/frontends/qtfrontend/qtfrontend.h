/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
	~QtFrontend() override;

	/**
	 * The run function for starting the application.
	 * @param argc the argc argument from the environment through main.
	 * @param argv the argv arguments from the environment through main.
	 * @return the return status on exit
	 */
	int run(int argc, char **argv) override;

	void showProgress(ProgressMessage message, int numOperations) override;
	void hideProgress() override;
	void updateProgress(int numOperationsDone) override;
	void setProgressInfo(const char *infoText);
	bool isOperationAborted() override;
	void processEvents() override;
	bool askQuestion(Question question) override;
	int runExternalCommand(const char *command) override;
	void handleException(UiException&) override;
	void reportWarning(const char *message) override;

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
