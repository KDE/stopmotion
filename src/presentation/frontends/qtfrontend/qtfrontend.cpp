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
#include "qtfrontend.h"

#include "src/application/externalcommand.h"

#include <QtGui>

#include <cstring>
#include <unistd.h>


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
	mw->resize(751, 593);
	mw->move(20, 20);
	mw->show();
	
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


void QtFrontend::showProgress(const char* operation, unsigned int numOperations)
{
	if (numOperations > 0) {
		progressDialog = new QProgressDialog( operation, tr("Cancel"), 0, numOperations,mw);
		progressDialog->show();
	}
	else {
		progressBar = new QProgressBar;
		progressBar->setFixedWidth(150);
		infoText = new QLabel(operation);
		mw->statusBar()->addWidget(infoText);
		mw->statusBar()->addWidget(progressBar);

		timer = new QTimer();
		connect( timer, SIGNAL( timeout() ), this, SLOT( updateProgressBar() ) );
        timer->start(10);
	}
}


void QtFrontend::hideProgress()
{
	if (progressDialog) { 
		progressDialog->hide();
		delete progressDialog;
		progressDialog = NULL;
	}
	else if (progressBar) {
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


void QtFrontend::updateProgress(int numOperationsDone)
{
	if (progressDialog) {
		progressDialog->setValue(numOperationsDone);
	}
}


void QtFrontend::setProgressInfo(const char *infoText)
{
	if (progressDialog) {
		progressDialog->setLabelText(infoText);
	}
}


bool QtFrontend::isOperationAborted()
{
	if (progressDialog) {
		return progressDialog->wasCanceled();
	}
	return false;
}


void QtFrontend::processEvents()
{
	stApp->processEvents();
}


void QtFrontend::updateProgressBar()
{
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
	QString preferencesFile = getenv("HOME");
	preferencesFile += "/.stopmotion/preferences.xml";
	QString oldPrefsFile = preferencesFile + ".OLD";
	
	// Has to check this before calling setPreferencesFile(...) because
	// the function creates the file if it doesn't exist.
	int prefsFileExists = access(preferencesFile.toLatin1().constData(), R_OK);
	if (prefsFileExists != -1) {
		QString tmp = "/bin/cp " + preferencesFile + " " + oldPrefsFile;
		system(tmp.toLatin1().constData());
	}

	// If file doesn't exist or has wrong version number
	if ( !prefs->setPreferencesFile(preferencesFile.toLatin1().constData(), "0.8") ) {
		// File doesn't exist
		if (prefsFileExists == -1) {
			setDefaultPreferences(prefs);
		}
		// Has wrong version number
		else {
			QString question(tr(	
					"A newer version of the preferences file with few more default\n"
					"values exists. Do you want to use this one? (Your old preferences\n "
					"will be saved in ~/.stopmotion/preferences.xml.OLD)"));
			int useNewPrefsFile = askQuestion(question.toLatin1().constData());
			// Use new preferences
			if (useNewPrefsFile == 0) { // 0 = yes
				setDefaultPreferences(prefs);
			}
			// Use old preferences
			else {
				rename(oldPrefsFile.toLatin1().constData(), preferencesFile.toLatin1().constData());
				prefs->setPreferencesFile(preferencesFile.toLatin1().constData(), prefs->getOldVersion());
				
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
	prefs->setPreference("importname0", tr("vgrabbj VGA singleshot").toLatin1().constData());
	prefs->setPreference("importdescription0", 
			tr("The simplest setting. Fairly slow").toLatin1().constData());
	prefs->setPreference("importprepoll0",
			"vgrabbj -f $IMAGEFILE -d $VIDEODEVICE -b -D 0 -i vga");
	prefs->setPreference("importstopdeamon0", "");

	// Default import option 2
	prefs->setPreference("importname1", tr("vgrabbj VGA deamon").toLatin1().constData());
	prefs->setPreference("importdescription1", 
			tr("Starts vgrabbj as a deamon. Pretty fast.").toLatin1().constData());
	prefs->setPreference("importstartdeamon1", 
			"vgrabbj -f $IMAGEFILE -d $VIDEODEVICE -b -D 0 -i vga -L250");
	prefs->setPreference("importstopdeamon1", 
			"kill -9 `ps ax | grep vgrabbj | grep -v grep | cut -b 0-5`");
	
	// Default import option 3
	prefs->setPreference("importname2", tr("dvgrab").toLatin1().constData());
	prefs->setPreference("importdescription2", 
			tr("Grabbing from DV-cam. (EXPERIMENTAL)").toLatin1().constData());
	prefs->setPreference("importstartdeamon2", 
			"dvgrab --format jpeg --jpeg-overwrite --jpeg-deinterlace --jpeg-width 640 " 
			"--jpeg-height 480 --frames 25 $IMAGEFILE");
	prefs->setPreference("importstopdeamon2", 
			"kill -9 `ps ax | grep dvgrab | grep -v grep | cut -b 0-5`");
	
	// Default import option 4
	prefs->setPreference("importname3", tr("videodog singleshot").toLatin1().constData());
	prefs->setPreference("importdescription3", 
			tr("Videodog.").toLatin1().constData());
	prefs->setPreference("importprepoll3",
			"videodog -x 640 -y 480 -w 3 -d $VIDEODEVICE -j -f $IMAGEFILE");
	prefs->setPreference("importstopdeamon3", "");
	
	// Default import option 5
	prefs->setPreference("importname4", tr("dvgrab").toLatin1().constData());
	prefs->setPreference("importdescription4", 
			tr("Grabbing from DV-cam.").toLatin1().constData());
	prefs->setPreference("importstartdeamon4", 
			"dvgrab --format jpeg --jpeg-overwrite --jpeg-deinterlace --jpeg-width 640 "
			"--jpeg-height 480 --every 25 $IMAGEFILE &");
	prefs->setPreference("importstopdeamon4", 
			"kill -9 `ps ax | grep dvgrab | grep -v grep | cut -b 0-5`");
	// -----------------------------------------------------------------------

	// Default export options ------------------------------------------------
	prefs->setPreference("numEncoders", 4);
	prefs->setPreference("activeEncoder", 3);

	// Default export option 1
	prefs->setPreference("encoderName0", "mencoder");
	prefs->setPreference("encoderDescription0", 
			tr("Exports from jpeg images to mpeg1 video").toLatin1().constData());
	prefs->setPreference("startEncoder0",
			"mencoder \"mf://$IMAGEPATH/*.jpg\" -mf w=640:h=480:fps=12:type=jpg "
			"-ovc lavc -lavcopts vcodec=mpeg1video -oac copy -o \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder0", "");

	// Default export option 2
	prefs->setPreference("encoderName1", "mencoder");
	prefs->setPreference("encoderDescription1", 
			tr("Exports from jpeg images to mpeg2 video").toLatin1().constData());
	prefs->setPreference("startEncoder1",
			"mencoder \"mf://$IMAGEPATH/*.jpg\" -mf w=640:h=480:fps=4:type=jpg "
			"-ovc lavc -lavcopts vcodec=mpeg2video -oac copy -o \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder1", "");

	// Default export option 3
	prefs->setPreference("encoderName2", "mencoder");
	prefs->setPreference("encoderDescription2", 
			tr("Exports from jpeg images to mpeg4 video").toLatin1().constData());
	prefs->setPreference("startEncoder2",
			"mencoder -ovc lavc -lavcopts vcodec=msmpeg4v2:vpass=1:$opt -mf type=jpg:fps=8 "
			"-o \"$VIDEOFILE\" \"mf://$IMAGEPATH/*.jpg\"");
	prefs->setPreference("stopEncoder2", "");
	
	// Default export option 4
	prefs->setPreference("encoderName3", "ffmpeg");
	prefs->setPreference("encoderDescription3", 
			tr("Exports from jpeg images to mpeg4 video").toLatin1().constData());
	prefs->setPreference("startEncoder3",
			"ffmpeg -r 10 -b 1800 -i \"$IMAGEPATH/%06d.jpg\" \"$VIDEOFILE\"");
	prefs->setPreference("stopEncoder3", "");
	//-------------------------------------------------------------------------
}


void QtFrontend::updateOldPreferences(PreferencesTool *prefs)
{
	// Replace all occurences of '(DEFAULTPATH)' with '$IMAGEFILE'  (version 0.3 and 0.4)
	// Replace all occurences of '/dev/xxx' with $VIDEODEVICE (version < 0.7)
	int numImports = prefs->getPreference("numberofimports", 1);
	for (int i = 0; i < numImports; ++i) {
		string start( prefs->getPreference(QString("importstartdeamon%1").arg(i).toLatin1().constData(), "") );
		int index = start.find("(DEFAULTPATH)");
		if (index != -1) {
			start.replace(index, strlen("(DEFAULTPATH)"), string("$IMAGEFILE"));
		}
		QString s(start.c_str());
		s.replace( QRegExp("/dev/(v4l/){0,1}video[0-9]{0,1}"), QString("$VIDEODEVICE") );
		prefs->setPreference( QString("importstartdeamon%1").arg(i).toLatin1().constData(), s.toLatin1().constData());

		string prepoll( prefs->getPreference(QString("importprepoll%1").arg(i).toLatin1().constData(), "") );
		index = prepoll.find("(DEFAULTPATH)");
		if (index != -1) {
			prepoll.replace(index, strlen("(DEFAULTPATH)"), string("$IMAGEFILE"));
		}
		QString ss(prepoll.c_str());
		ss.replace( QRegExp("/dev/(v4l/){0,1}video[0-9]{0,1}"), QString("$VIDEODEVICE") );
		prefs->setPreference( QString("importprepoll%1").arg(i).toLatin1().constData(), ss.toLatin1().constData());
	}
}


int QtFrontend::askQuestion(const char *question)
{
	int ret = QMessageBox::question(0,
			tr("Question"),
			QString(question),
			QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);
	if (ret == QMessageBox::Yes) {
		return 0;
	}
	return 1;
}
	

int QtFrontend::runExternalCommand(const char *command)
{	
	ExternalCommand *ec = new ExternalCommand;
	ec->show();
	ec->run( QString(command) );
	return 0;
}
