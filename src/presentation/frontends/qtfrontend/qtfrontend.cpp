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
#include "qtfrontend.h"

#include "frontends/frontend.h"
#include "src/application/externalcommand.h"
#include "src/presentation/frontends/qtfrontend/mainwindowgui.h"
#include "src/foundation/preferencestool.h"
#include "src/foundation/logger.h"
#include "src/foundation/uiexception.h"
#include "src/domain/animation/workspacefile.h"
#include "src/domain/domainfacade.h"

#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QProgressBar>
#include <QRegularExpression>
#include <QStatusBar>
#include <QTimer>

#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string>

const char* QtFrontend::VERSION = "0.8";

QtFrontend::QtFrontend(int &argc, char **argv) {
	stApp = new QApplication(argc, argv);

	mw = new MainWindowGUI(stApp);
	mw->setWindowTitle("Stopmotion");
	mw->showMaximized();

	try {
		initializePreferences();
		PreferencesTool::get()->flush();
	} catch (UiException& ex) {
		DomainFacade::getFacade()->getFrontend()->handleException(ex);
	}

	mw->ConstructUI();

	progressDialog = 0;
	progressBar = 0;
	infoText = 0;
	timer = 0;
}


QtFrontend::~QtFrontend() {
	delete mw;
	mw = 0;
	delete stApp;
	stApp = 0;
}


int QtFrontend::run(int, char **) {
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

long fileSize(const char* filePath) {
	FILE* fh = fopen(filePath, "r");
	if (!fh)
		return -1;
	long result = -1;
	if (0 == fseek(fh, 0, SEEK_END)) {
		result = ftell(fh);
	}
	fclose(fh);
	return result;
}

bool QtFrontend::loadPreferencesFrom(PreferencesTool* prefs, const char* path) {
	if (prefs->load(path))
		return true;
	if (0 == access(path, R_OK)) {
		// file exists and is readable
		if (fileSize(path) < 40) {
			// not worth keeping a file this small
			return false;
		}
		int ret = QMessageBox::question(0,
				tr("Lose corrupt file"),
				tr("The file %1 seems to be corrupt, it's contents will be lost if you continue. Do you want to continue?").arg(path),
				QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);
		if (ret == QMessageBox::No)
			throw CriticalError();
	}
	return false;
}

void QtFrontend::initializePreferences() {
	Logger::get().logDebug("Loading preferencestool");

	PreferencesTool *prefs = PreferencesTool::get();
	WorkspaceFile preferencesFile(WorkspaceFile::preferencesFile);
	WorkspaceFile oldPrefsFile(WorkspaceFile::preferencesFileOld);

	WorkspaceFile::ensureStopmotionDirectoriesExist();

	bool dirty = false;
	bool oldIsDirty = true;
	if (!loadPreferencesFrom(prefs, preferencesFile.path())) {
		dirty = true;
		Logger::get().logWarning("Did not load new prefs");
		if (loadPreferencesFrom(prefs, oldPrefsFile.path())) {
			oldIsDirty = false;
		} else {
			Logger::get().logWarning("Did not load old prefs");
			prefs->setDefaultPreferences(VERSION);
			setDefaultPreferences(prefs);
		}
	}
	if (!prefs->isVersion(VERSION)) {
		bool useNewPrefsFile = askQuestion(Frontend::useNewerPreferences);
		if (useNewPrefsFile) {
			// copy to old
			prefs->setSavePath(preferencesFile.path(), oldIsDirty);
			prefs->flush();
			// Create new default preferences
			setDefaultPreferences(prefs);
		} else {
			// Use and update old preferences
			prefs->setVersion(VERSION);
			updateOldPreferences(prefs);
		}
		dirty = true;
	}
	prefs->setSavePath(preferencesFile.path(), dirty);
	prefs->flush();
}


void QtFrontend::setDefaultPreferences(PreferencesTool *prefs) {
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
			"vgrabbj -f $IMAGEFILE -d $VIDEODEVICE -b -D 0 -i vga -L250000");
	prefs->setPreference("importstopdaemon1",
			"kill $(pidof vgrabbj)");

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
			"dvgrab --format jpeg --jpeg-overwrite --jpeg-temp $(tempfile) "
			"--every 25 $IMAGEFILE &");
	prefs->setPreference("importstopdaemon4",
			"kill -2 $(pidof dvgrab)");
	// -----------------------------------------------------------------------

	// Default export options ------------------------------------------------
	prefs->setPreference("numEncoders", 5);
	prefs->setPreference("activeEncoder", 3);

	// Default export option 1
	prefs->setPreference("encoderName0", "mencoder");
	prefs->setPreference("encoderDescription0",
			tr("Exports from jpeg images to mpeg1 video").toUtf8().constData());
	prefs->setPreference("startEncoder0",
			"mencoder \"mf://$IMAGEPATH/*.jpg\" -mf w=640:h=480:fps=$FRAMERATE:type=jpg "
			"-ovc lavc -lavcopts vcodec=mpeg1video -oac copy -o \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder0", "");

	// Default export option 2
	prefs->setPreference("encoderName1", "mencoder");
	prefs->setPreference("encoderDescription1",
			tr("Exports from jpeg images to mpeg2 video").toUtf8().constData());
	prefs->setPreference("startEncoder1",
			"mencoder \"mf://$IMAGEPATH/*.jpg\" -mf w=640:h=480:fps=$FRAMERATE:type=jpg "
			"-ovc lavc -lavcopts vcodec=mpeg2video -oac copy -o \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder1", "");

	// Default export option 3
	prefs->setPreference("encoderName2", "mencoder");
	prefs->setPreference("encoderDescription2",
			tr("Exports from jpeg images to mpeg4 video").toUtf8().constData());
	prefs->setPreference("startEncoder2",
			"mencoder -ovc lavc -lavcopts vcodec=msmpeg4v2:vpass=1:$opt -mf type=jpg:fps=$FRAMERATE "
			"-o \"$VIDEOFILE\" \"mf://$IMAGEPATH/*.jpg\"");
	prefs->setPreference("stopEncoder2", "");

	// Default export option 4
	prefs->setPreference("encoderName3", "avconv");
	prefs->setPreference("encoderDescription3",
			tr("Exports from jpeg images to mpeg4 video").toUtf8().constData());
	prefs->setPreference("startEncoder3",
			"avconv -y -r $FRAMERATE -i \"$IMAGEPATH/%06d.jpg\" -b 6000 \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder3", "");

	// Default export option 5 -- ffmpeg
	prefs->setPreference("encoderName4", "ffmpeg");
	prefs->setPreference("encoderDescription4",
			tr("Exports from jpeg images to mpeg4 video").toUtf8().constData());
	prefs->setPreference("startEncoder4",
			"ffmpeg -y -framerate $FRAMERATE -i \"$IMAGEPATH/%06d.jpg\" -codec:v mpeg4 -b:v 6k \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder4", "");
}

void QtFrontend::handleException(UiException& e) {
	bool unhandled = false;
	switch (e.error()) {
	case UiException::IsWarning:
		switch (e.warning()) {
		case UiException::unsupportedImageType:
			QMessageBox::warning(0, tr("Unsupported image file type"),
					tr("Only JPeg image files can be added to the animation"));
			return;
		case UiException::invalidAudioFormat:
			QMessageBox::warning(0, tr("The selected audio file could not be loaded"),
					tr("Perhaps it is corrupt."));
			return;
		case UiException::couldNotOpenFile:
			QMessageBox::warning(0,
					tr("Cannot open the selected file for reading"),
					tr("The file %1 could not be opened").arg(e.string()));
			return;
		case UiException::failedToCopyFilesToWorkspace:
			QMessageBox::warning(0,
					tr("Could not copy file to workspace"),
					tr("Failed to copy the following files to the workspace (~/.stopmotion): %1").arg(e.string()));
			return;
		case UiException::failedToInitializeAudioDriver:
			QMessageBox::warning(0,
					tr("Failed to initialize audio driver"),
					tr("Sound will not work until this is corrected"));
			break;
		case UiException::failedToWriteToPreferencesFile:
			QMessageBox::warning(0,
					tr("Failed to write preferences"),
					tr("Could not write preferences to file: %1").arg(e.string()));
			break;
		default:
			unhandled = true;
			break;
		}
		break;
	case UiException::failedToGetExclusiveLock:
		QMessageBox::critical(0,
				tr("Stopmotion cannot be started."),
				tr("Failed to get exclusive lock on command.log. Perhaps Stopmotion is already running."));
		break;
	case UiException::preferencesFileUnreadable:
		QMessageBox::critical(0,
				tr("Preferences file cannot be read"),
				tr("Preferences file %1 is unreadable. Please correct this and try again.").arg(e.string()));
		break;
	case UiException::preferencesFileMalformed:
		QMessageBox::critical(0,
				tr("Preferences file cannot be read"),
				tr("Preferences file %1 is not a valid XML preferences file. Please correct this or delete the file and try again.").arg(e.string()));
		break;
	case UiException::ArbitraryError:
		QMessageBox::critical(0, tr("Fatal"), e.what());
		break;
	default:
		unhandled = true;
		break;
	}
	if (unhandled) {
		QMessageBox::critical(0,
				tr("Stopmotion threw an exception it could not handle."),
				tr("Please raise a bug report."));
		throw CriticalError();
	}
}

void QtFrontend::reportWarning(const char *message) {
	QMessageBox::warning(0, tr("Warning"), message);
}

void QtFrontend::updateOldPreferences(PreferencesTool *prefs) {
	// Replace all occurrences of '(DEFAULTPATH)' with '$IMAGEFILE'  (version 0.3 and 0.4)
	// Replace all occurrences of '/dev/xxx' with $VIDEODEVICE (version < 0.7)
	int numImports = prefs->getPreference("numberofimports", 1);
	for (int i = 0; i < numImports; ++i) {
		Preference startPref(QString("importstartdaemon%1")
				.arg(i).toUtf8().constData(), "");
		std::string start(startPref.get());
		int index = start.find("(DEFAULTPATH)");
		if (index != -1) {
			start.replace(index, (int) strlen("(DEFAULTPATH)"),
					std::string("$IMAGEFILE"));
		}
		QString s(start.c_str());
		s.replace( QRegularExpression("/dev/(v4l/){0,1}video[0-9]{0,1}"),
				QString("$VIDEODEVICE") );
		prefs->setPreference( QString("importstartdaemon%1")
				.arg(i).toUtf8().constData(), s.toUtf8().constData());

		Preference prepollPref(QString("importprepoll%1")
				.arg(i).toUtf8().constData(), "");
		std::string prepoll(prepollPref.get());
		index = prepoll.find("(DEFAULTPATH)");
		if (index != -1) {
			prepoll.replace(index, (int) strlen("(DEFAULTPATH)"),
					std::string("$IMAGEFILE"));
		}
		QString ss(prepoll.c_str());
		ss.replace( QRegularExpression("/dev/(v4l/){0,1}video[0-9]{0,1}"), QString("$VIDEODEVICE") );
		prefs->setPreference( QString("importprepoll%1").arg(i).toUtf8().constData(), ss.toUtf8().constData());
	}
}


bool QtFrontend::askQuestion(Question question) {
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
	return ret == QMessageBox::Yes;
}


int QtFrontend::runExternalCommand(const char *command) {
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
