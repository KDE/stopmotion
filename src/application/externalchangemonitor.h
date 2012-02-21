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
#ifndef EXTERNALCHANGEMONITOR_H
#define EXTERNALCHANGEMONITOR_H

#include <QObject>
#include <QStringList>

class QSocketNotifier;

/**
 * Class for listening after changes to the disc representation of the
 * animationmodel.
 *
 * The class uses Inotify-tools to listen for changes
 * in the project directories.
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
	ExternalChangeMonitor(QObject *parent = 0);

	/**
	 * Calls stopMonitoring and cleans up.
	 */
	~ExternalChangeMonitor();

	/**
	 * Register the directory for monitoring.
	 * @param directory the directory to listen for changes in.
	 */
	void addDirectory(const QString &directory);

	/**
	 * Creates a inotify connection and listens for changes in the project directories.
	 */
	void startMonitoring();

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
	/** For polling the file connection through the qt event loop */
	QSocketNotifier *socketNotifier;
	bool isMonitoring;
        QStringList directories;

private slots:
	/**
	 * Callback function for when the QSocketNotifier recieves an Inotify event.
	 */
	void readInotifyEvents(int socket);
};

#endif
