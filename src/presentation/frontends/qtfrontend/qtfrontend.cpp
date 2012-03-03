/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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
#include "src/foundation/preferencestool.h"

#include <qmessagebox.h>
#include <string.h>
#include <iostream>


QtFrontend::QtFrontend(int argc, char **argv)
{
	stApp = new QApplication(argc, argv);
	
	initializePreferences();
	
	//Loads the correct translator based on the language in the preferences file.
	QString t = PreferencesTool::get()->getPreference("language", "en");
	Logger::get().logDebug("Loading translator: ");
	Logger::get().logDebug(t.ascii());
	QTranslator translator( 0 );
	translator.load( "translations/stopmotion_" + t, "." );
	stApp->installTranslator( &translator );
	
	//If the preferred language is english (en) no translator should be
	//installed.
	if(t == "en") {
		translator.clear();
	}
	
	mw = new MainWindowGUI(stApp);
	mw->setCaption("Stopmotion");
	mw->resize(751, 593);
	mw->move(20, 20);
	mw->show();
	progressDialog = NULL;
	progressBar = NULL;
	infoText = NULL;
	timer = NULL;
}


QtFrontend::~QtFrontend()
{
	delete stApp;
	stApp = NULL;
	// doesn't need to delete mw since the WDestructiveClose flag is set.
}


int QtFrontend::run(int, char **)
{
	stApp->connect( stApp, SIGNAL(lastWindowClosed()), stApp, SLOT(quit()) );
	return stApp->exec();
}


void QtFrontend::showProgress(const char* operation, unsigned int numOperations)
{
	if (numOperations > 0) {
		progressDialog = new QProgressDialog( operation, tr("Cancel"), numOperations,
				mw, tr("Progress"), TRUE );
		progressDialog->show();
	}
	else {
		progressBar = new QProgressBar(0, mw);
		progressBar->setFixedWidth(150);
		infoText = new QLabel(operation, mw);
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
		progressDialog->setProgress(numOperationsDone);
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
    int p = progressBar->progress();
    progressBar->setProgress(++p);
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
	
	//Initializes the preferences file and sets default preferences
	//if the file hasn't been made before.
	QString preferencesFile = getenv("HOME");
	preferencesFile += "/.stopmotion/preferences.xml";
		
	PreferencesTool *prefs = PreferencesTool::get();
	//If the file doesn't excist or has the wrong version default preferenses 
	//are made.
	if( !prefs->setPreferencesFile(preferencesFile.ascii(), "0.3") ) {
		Logger::get().logDebug("Loading default preferences");
		
		
		//The default import options
		prefs->setPreference("numberofimports", 2);
		
		prefs->setPreference("importname0", tr("vgrabbj VGA singleshot").ascii());
		prefs->setPreference("importdescription0", 
				tr("The simplest setting. Repeatedly asks vgrabbj "
				"for a picture. Fairly slow").ascii());
		prefs->setPreference("importprepoll0",
				"vgrabbj -f (DEFAULTPATH) -d /dev/video0 -b -D 0 -i vga");
				
		prefs->setPreference("importname1", tr("vgrabbj VGA deamon").ascii());
		prefs->setPreference("importdescription1", 
				tr("Starts vgrabbj as a deamon. Pretty fast.").ascii());
		
		prefs->setPreference("importstartdeamon1", 
				"vgrabbj -f (DEFAULTPATH) -d /dev/video0 -b -D 0 -i vga -L250");
		prefs->setPreference("importstopdeamon1", 
				"kill -9 `ps ax | grep vgrabbj | grep -v grep | cut -b 0-5`");
		
		prefs->setPreference("activedevice", 1);
	}
}


int QtFrontend::askQuestion(const char *question)
{
	int ret = QMessageBox::question(mw,
			tr("Question"),
			question,
			tr("&Yes"), tr("&No"), // button 0, button 1, ...
			QString::null, 0, 1 );
	return ret;
}
