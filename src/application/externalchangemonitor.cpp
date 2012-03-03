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
#include "src/application/externalchangemonitor.h"

#include "src/domain/domainfacade.h"

#include <QImage>
#include <QStringList>

#include <iostream>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <SDL/SDL_image.h>

using namespace std;


/**
 * @todo uniform most of the getenv("home") calls so that it is done once
 * and pasted around.
 */
ExternalChangeMonitor::ExternalChangeMonitor(QObject *parent)
	: QObject(parent)
{
	socketNotifier = 0;
	famConn        = 0;
	workDirectory  = "";
	refresh        = false;
	running        = false;
	
	connect( &timer, SIGNAL(timeout()), this, SLOT(timeout()) );
	
	tmpDirectory = getenv("HOME");
	tmpDirectory += "/.stopmotion/tmp";
}


ExternalChangeMonitor::~ExternalChangeMonitor()
{
	if (socketNotifier != 0) {
		stopMonitoring();
	}
}


void ExternalChangeMonitor::changeWorkDirectory(const char* workDirectory)
{
	if (running) {
		Logger::get().logDebug("Registring working directory for changelistening: ");
		Logger::get().logDebug(workDirectory);
		
		stopMonitoring();
		
		famConn = new FAMConnection;
		if (FAMOpen2(famConn, "stopmotion") != 0)
		{
			delete famConn;
			famConn = 0;
			Logger::get().logWarning("Warning: Cannot connect to FAM");
			running = false;
			return;
		}
		
		if ( FAMMonitorDirectory( famConn, tmpDirectory.toStdString().c_str(), &famRequest, 0 ) < 0 ) {
			Logger::get().logWarning("Warning: Cannot monitor tmp directory: ");
			FAMClose( famConn );
			return;
		}
		
		QString dirName;
		if (strcmp(workDirectory, "") != 0) {
			QStringList lst = QString(workDirectory).split("/");
			dirName = lst.back();
			lst = QString(dirName).split(".sto");
			dirName = lst.front();
			dirName.prepend("/.stopmotion/packer/");
			dirName.prepend(getenv("HOME"));
			dirName.append("/images");
			
			if ( FAMMonitorDirectory( famConn, dirName.toStdString().c_str(), &famRequest, 0 ) < 0 ) {
				Logger::get().logWarning("Warning: Cannot monitor project directory: ");
				FAMClose( famConn );
				return;
			}
		}
		
		int famfd = FAMCONNECTION_GETFD(famConn);
		int flags = fcntl(famfd, F_GETFL);
		fcntl(famfd, F_SETFL, flags | O_NONBLOCK);
		
		socketNotifier = new QSocketNotifier(famfd, QSocketNotifier::Read, 0);
		connect(socketNotifier, SIGNAL(activated(int)), this, SLOT(readFam()));
		
		this->workDirectory = dirName;
	}
}


void ExternalChangeMonitor::startMonitoring()
{
	if (running) {
		stopMonitoring();
	}
	
	famConn = new FAMConnection;
	if (FAMOpen2(famConn, "stopmotion") != 0) {
		delete famConn;
		famConn = 0;
		Logger::get().logWarning("Warning: Cannot connect to FAM");
		running = false;
		return;
	}
	
	running = true;
	
	int famfd = FAMCONNECTION_GETFD(famConn);
	int flags = fcntl(famfd, F_GETFL);
	fcntl(famfd, F_SETFL, flags | O_NONBLOCK);
	
	if ( FAMMonitorDirectory( famConn, tmpDirectory.toLatin1().constData(), &famRequest, 0 ) < 0 ) {
		Logger::get().logWarning("Warning: Cannot monitor tmp directory: ");
		FAMClose( famConn );
		return;
	}
	
	socketNotifier = new QSocketNotifier(famfd, QSocketNotifier::Read, 0);
	connect(socketNotifier, SIGNAL(activated(int)), this, SLOT(readFam()));
}


void ExternalChangeMonitor::stopMonitoring()
{
	delete socketNotifier;
	socketNotifier = 0;
	
	if (famConn) {
		FAMClose(famConn);
		delete famConn;
		famConn = 0;
	}

	running = false;
}


void ExternalChangeMonitor::readFam()
{
	while ( FAMPending(famConn) ) {
		FAMEvent event;
		if (FAMNextEvent(famConn, &event) != 1) {
			stopMonitoring();
			return;
		}
		
		if (event.code == FAMChanged) {
			if (refresh == false){
				timer.start(TIMER_INTERVAL);
				timer.setSingleShot(true);
				refresh = true;
			}
		
			if(timer.isActive()) {
				timer.stop();
				timer.start(TIMER_INTERVAL);
				timer.setSingleShot(true);
			}
			
			QString tmp = event.filename;
			tmp.prepend("/");
			
			if (tmp.left(4) == "/tmp") {
				tmp.prepend(tmpDirectory);
			}
			else {
				tmp.prepend(workDirectory);
			}
			
			fileName = tmp;
		}
	}
}


void ExternalChangeMonitor::suspendMonitor()
{
	if (running) {
		FAMSuspendMonitor(famConn, &famRequest);
	}
}


void ExternalChangeMonitor::resumeMonitor()
{
	if (running) {
		FAMResumeMonitor(famConn, &famRequest);
	}
}


void ExternalChangeMonitor::timeout()
{
	DomainFacade::getFacade()->animationChanged(fileName.toLatin1().constData());
	refresh = false;
}
