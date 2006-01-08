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
#ifndef EXTERNALCHANGEMONITOR_H
#define EXTERNALCHANGEMONITOR_H

#include "src/config.h"

#include <qobject.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <fam.h>


/**
 * Class for listening after changes to the disc representation of the 
 * animationmodel.
 *
 * The class use FAM (File Alteration Monitor) register for changes
 * in the project directories. 
 *
 * The use of the QSocketNotifier adds the monitoring of the FAM to the 
 * Qt main event loop and is documented at:
 * http://oss.sgi.com/projects/fam/qt_gtk.html#qt
 * FAM is documented many places most notably in the FAM man page.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ExternalChangeMonitor : public QObject
{
	Q_OBJECT
public:
	/**
	 * Initializes the ExternalChangeMonitor and registers the animationModel
	 * to notify of changes when the disk representation of the animation is 
	 * changed
	 */
	ExternalChangeMonitor();
	
	/**
	 * Calls stopMonitoring and cleans up.
	 */
	~ExternalChangeMonitor();
	
	/**
	 * Register the current workdirectory for monitorng.
	 * @param workDirectory the workDirectory to listen for changes in.
	 */
	void changeWorkDirectory(const char* workDirectory);
	
	/**
	 * Creates a FAM connection to the project directories and starts a thread
	 * which polls the FAM connection for changes in the tmp directory.
	 *
	 * To register the working project directory for monitoring also call 
	 * changeWorkDirectory
	 */
	void startMonitoring();
	
	/**
	 * Stops the thread polling the FAM connection and tears the connection down.
	 */
	void stopMonitoring();
	
	/**
	 * Suspends the monitoring until resumeMonitor() is called.
	 */
	void suspendMonitor();
	
	/**
	 * Resumes monitoring the directories.
	 */
	void resumeMonitor();
	
private:
	static const int TIMER_INTERVAL = 500;
	
	/** The connection to the fam deamon */
	FAMConnection *famConn;
	FAMRequest    famRequest;
	
	/** For polling the file connection through the qt event loop */
	QSocketNotifier *socketNotifier;
	
	QString workDirectory;
	QString tmpDirectory;
	
	QTimer timer;
	QString fileName;
	bool refresh;
	
	bool running;
	
private slots:
	/**
	 * Callback function for when the QSocketNotifier recieves a fam event.
	 */
	void readFam();
	
	void timeout();
};

#endif
