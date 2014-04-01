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
#include "qtfrontend.h"

#include "src/application/externalcommand.h"
#include "src/presentation/frontends/qtfrontend/mainwindowgui.h"
#include "src/foundation/preferencestool.h"
#include "src/foundation/logger.h"
#include "src/technical/util.h"
#include "src/domain/animation/workspacefile.h"

#include <QProgressDialog>
#include <QProgressBar>
#include <QTimer>
#include <QApplication>
#include <QLabel>
#include <QtGui>

#include <cstring>
#include <unistd.h>
#include <sstream>
#include <stdio.h>
#include <assert.h>

QtFrontend::QtFrontend(int &argc, char **argv)
{
	stApp = new QApplication(argc, argv);
#if QT_VERSION == 0x040400
        stApp->setAttribute(Qt::AA_NativeWindows);
#endif

	// Need to call this here to get the locale for the language
	// which is used by the translator created in mainWindowGUI
	initializePreferences();

	mw = new MainWindowGUI(stApp);
	mw->setWindowTitle("Stopmotion");
	mw->showMaximized();

	progressDialog = 0;
	progressBar = 0;
	infoText = 0;
	timer = 0;
}


QtFrontend::~QtFrontend()
{
	delete mw;
	mw = 0;
	delete stApp;
	stApp = 0;
}


int QtFrontend::run(int, char **)
{
	stApp->connect( stApp, SIGNAL(lastWindowClosed()), stApp, SLOT(quit()) );
	return stApp->exec();
}


void QtFrontend::showProgress(ProgressMessage message, int numOperations) {
	QString msg = tr("Please wait...");
	switch (message) {
	case connectingCamera:
		msg = tr("Connecting camera...");
		break;
	case importingFramesFromDisk:
		msg = tr("Importing frames from disk");
		break;
	case exporting:
		msg = tr("Exporting...");
		break;
	case restoringProject:
		msg = tr("Restoring project...");
		break;
	case savingScenesToDisk:
		msg = tr("Saving scenes to disk...");
		break;
	}
	if (numOperations > 0) {
		progressDialog = new QProgressDialog(msg, tr("Cancel"), 0,
				numOperations, mw);
		progressDialog->show();
	} else {
		progressBar = new QProgressBar;
		progressBar->setFixedWidth(150);
		infoText = new QLabel(msg);
		mw->statusBar()->addWidget(infoText);
		mw->statusBar()->addWidget(progressBar);
		timer = new QTimer();
		connect( timer, SIGNAL( timeout() ), this, SLOT( updateProgressBar() ) );
        timer->start(10);
	}
}


void QtFrontend::hideProgress() {
	if (progressDialog) {
		progressDialog->hide();
		delete progressDialog;
		progressDialog = NULL;
	} else if (progressBar) {
		timer->stop();
		progressBar->hide();
		mw->statusBar()->removeWidget(progressBar);
		mw->statusBar()->removeWidget(infoText);
		delete progressBar;
		progressBar = NULL;
		delete infoText;
		infoText = NULL;
		delete timer;
		timer = NULL;
	}
}


void QtFrontend::updateProgress(int numOperationsDone) {
	if (progressDialog) {
		progressDialog->setValue(numOperationsDone);
	}
}


void QtFrontend::setProgressInfo(const char *infoText) {
	if (progressDialog) {
		progressDialog->setLabelText(infoText);
	}
}


bool QtFrontend::isOperationAborted() {
	if (progressDialog) {
		return progressDialog->wasCanceled();
	}
	return false;
}


void QtFrontend::processEvents() {
	stApp->processEvents();
}


void QtFrontend::updateProgressBar() {
    int p = progressBar->value();
    progressBar->setValue(++p);
}


void QtFrontend::reportError(const char *message, int id)
{
	id = id != 0 && id != 1 ? 0 : id;

	if (id == 0) {
		QMessageBox::warning(mw, tr("Warning"), message, QMessageBox::Ok,
				QMessageBox::NoButton, QMessageBox::NoButton);
	}
	else {
		QMessageBox::critical(mw, tr("Fatal"), message, QMessageBox::Ok,
				QMessageBox::NoButton, QMessageBox::NoButton);
	}
}


void QtFrontend::initializePreferences()
{
	Logger::get().logDebug("Loading preferencestool");

	PreferencesTool *prefs = PreferencesTool::get();
	WorkspaceFile preferencesFile(WorkspaceFile::preferencesFile);
	WorkspaceFile oldPrefsFile(WorkspaceFile::preferencesFileOld);

	// Has to check this before calling setPreferencesFile(...) because
	// the function creates the file if it doesn't exist.
	int prefsFileExists = access(preferencesFile.path(), R_OK);
	if (prefsFileExists != -1) {
		Util::copyFile(oldPrefsFile.path(), preferencesFile.path());
	}

	// If file doesn't exist or has wrong version number
	if ( !prefs->setPreferencesFile(preferencesFile.path(), "0.8") ) {
		// File doesn't exist
		if (prefsFileExists == -1) {
			setDefaultPreferences(prefs);
		}
		// Has wrong version number
		else {
			int useNewPrefsFile = askQuestion(Frontend::useNewerPreferences);
			// Use new preferences
			if (useNewPrefsFile == 0) { // 0 = yes
				setDefaultPreferences(prefs);
			}
			// Use old preferences
			else {
				rename(oldPrefsFile.path(), preferencesFile.path());
				prefs->setPreferencesFile(preferencesFile.path(), prefs->getOldVersion());

				// Update version
				prefs->setVersion("0.8");

				// Do necessary updates on the old prefs file:
				updateOldPreferences(prefs);
			}
		}
	}
}


void QtFrontend::setDefaultPreferences(PreferencesTool *prefs)
{
	assert(prefs != NULL);
	Logger::get().logDebug("Setting default preferences");

	// Default import options ------------------------------------------------
	prefs->setPreference("numberofimports", 5);
	prefs->setPreference("activedevice", 1);

	// Default import option 1
	prefs->setPreference("importname0", tr("vgrabbj").toUtf8().constData());
	prefs->setPreference("importdescription0",
			tr("The simplest setting. Fairly slow").toUtf8().constData());
	prefs->setPreference("importprepoll0",
			"vgrabbj -f $IMAGEFILE -d $VIDEODEVICE -b -D 0 -i vga");
	prefs->setPreference("importstartdaemon0", "");
	prefs->setPreference("importstopdaemon0", "");

	// Default import option 2
	prefs->setPreference("importname1", tr("vgrabbj VGA daemon").toUtf8().constData());
	prefs->setPreference("importdescription1",
			tr("Starts vgrabbj as a daemon. Pretty fast.").toUtf8().constData());
	prefs->setPreference("importprepoll1", "");
	prefs->setPreference("importstartdaemon1",
			"vgrabbj -f $IMAGEFILE -d $VIDEODEVICE -b -D 0 -i vga -L250");
	prefs->setPreference("importstopdaemon1",
			"kill -9 $(pidof vgrabbj)");

	// Default import option 3
	prefs->setPreference("importname2", tr("uvccapture").toUtf8().constData());
	prefs->setPreference("importdescription2",
			tr("Grabbing from V4L2 devices").toUtf8().constData());
	prefs->setPreference("importprepoll2",
			"uvccapture -d$VIDEODEVICE -x640 -y480 -o$IMAGEFILE");
	prefs->setPreference("importstartdaemon2", "");
	prefs->setPreference("importstopdaemon2", "");

	// Default import option 4
	prefs->setPreference("importname3", tr("videodog singleshot").toUtf8().constData());
	prefs->setPreference("importdescription3",
			tr("Videodog.").toUtf8().constData());
	prefs->setPreference("importprepoll3",
			"videodog -x 640 -y 480 -w 3 -d $VIDEODEVICE -j -f $IMAGEFILE");
	prefs->setPreference("importstartdaemon3", "");
	prefs->setPreference("importstopdaemon3", "");

	// Default import option 5
	prefs->setPreference("importname4", tr("dvgrab").toUtf8().constData());
	prefs->setPreference("importdescription4",
			tr("Grabbing from DV-cam.").toUtf8().constData());
	prefs->setPreference("importprepoll4", "");
	prefs->setPreference("importstartdaemon4",
			"dvgrab --format jpeg --jpeg-overwrite --jpeg-temp dvtemp.jpeg "
			"--every 25 $IMAGEFILE &");
	prefs->setPreference("importstopdaemon4",
			"kill -9 $(pidof dvgrab)");
	// -----------------------------------------------------------------------

	// Default export options ------------------------------------------------
	prefs->setPreference("numEncoders", 4);
	prefs->setPreference("activeEncoder", 3);

	// Default export option 1
	prefs->setPreference("encoderName0", "mencoder");
	prefs->setPreference("encoderDescription0",
			tr("Exports from jpeg images to mpeg1 video").toUtf8().constData());
	prefs->setPreference("startEncoder0",
			"mencoder \"mf://$IMAGEPATH/*.jpg\" -mf w=640:h=480:fps=12:type=jpg "
			"-ovc lavc -lavcopts vcodec=mpeg1video -oac copy -o \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder0", "");

	// Default export option 2
	prefs->setPreference("encoderName1", "mencoder");
	prefs->setPreference("encoderDescription1",
			tr("Exports from jpeg images to mpeg2 video").toUtf8().constData());
	prefs->setPreference("startEncoder1",
			"mencoder \"mf://$IMAGEPATH/*.jpg\" -mf w=640:h=480:fps=12:type=jpg "
			"-ovc lavc -lavcopts vcodec=mpeg2video -oac copy -o \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder1", "");

	// Default export option 3
	prefs->setPreference("encoderName2", "mencoder");
	prefs->setPreference("encoderDescription2",
			tr("Exports from jpeg images to mpeg4 video").toUtf8().constData());
	prefs->setPreference("startEncoder2",
			"mencoder -ovc lavc -lavcopts vcodec=msmpeg4v2:vpass=1:$opt -mf type=jpg:fps=12 "
			"-o \"$VIDEOFILE\" \"mf://$IMAGEPATH/*.jpg\"");
	prefs->setPreference("stopEncoder2", "");

	// Default export option 4
	prefs->setPreference("encoderName3", "avconv");
	prefs->setPreference("encoderDescription3",
			tr("Exports from jpeg images to mpeg4 video").toUtf8().constData());
	prefs->setPreference("startEncoder3",
			"avconv -r 12 -b 1800 -i \"$IMAGEPATH/%06d.jpg\" \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder3", "");
	//-------------------------------------------------------------------------
}


void QtFrontend::updateOldPreferences(PreferencesTool *prefs)
{
	// Replace all occurences of '(DEFAULTPATH)' with '$IMAGEFILE'  (version 0.3 and 0.4)
	// Replace all occurences of '/dev/xxx' with $VIDEODEVICE (version < 0.7)
	int numImports = prefs->getPreference("numberofimports", 1);
	for (int i = 0; i < numImports; ++i) {
		std::string start( prefs->getPreference(QString("importstartdeamon%1")
				.arg(i).toUtf8().constData(), "") );
		int index = start.find("(DEFAULTPATH)");
		if (index != -1) {
			start.replace(index, strlen("(DEFAULTPATH)"),
					std::string("$IMAGEFILE"));
		}
		QString s(start.c_str());
		s.replace( QRegExp("/dev/(v4l/){0,1}video[0-9]{0,1}"),
				QString("$VIDEODEVICE") );
		prefs->setPreference( QString("importstartdeamon%1")
				.arg(i).toUtf8().constData(), s.toUtf8().constData());

		std::string prepoll( prefs->getPreference(QString("importprepoll%1")
				.arg(i).toUtf8().constData(), "") );
		index = prepoll.find("(DEFAULTPATH)");
		if (index != -1) {
			prepoll.replace(index, strlen("(DEFAULTPATH)"),
					std::string("$IMAGEFILE"));
		}
		QString ss(prepoll.c_str());
		ss.replace( QRegExp("/dev/(v4l/){0,1}video[0-9]{0,1}"), QString("$VIDEODEVICE") );
		prefs->setPreference( QString("importprepoll%1").arg(i).toUtf8().constData(), ss.toUtf8().constData());
	}
}


int QtFrontend::askQuestion(Question question) {
	QString text;
	switch (question) {
	case useNewerPreferences:
		text = tr(
				"A newer version of the preferences file with few more default\n"
				"values exists. Do you want to use this one? (Your old preferences\n "
				"will be saved in ~/.stopmotion/preferences.xml.OLD)");
		break;
	}
	int ret = QMessageBox::question(0,
			tr("Question"), text,
			QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);
	return ret == QMessageBox::Yes? 0 : 1;
}


int QtFrontend::runExternalCommand(const char *command)
{
	ExternalCommand *ec = new ExternalCommand;
	ec->show();
	ec->run( QString::fromLocal8Bit(command) );
	return 0;
}


void QtFrontend::setUndoRedoEnabled() {
	mw->activateMenuOptions();
}

void QtFrontend::openProject(const char* file) {
	mw->openProject(file);
}
