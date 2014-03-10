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
#include "externalchangemonitor.h"
#include "src/domain/domainfacade.h"
#include "src/foundation/logger.h"

#include <QDebug>
#include <QTime>
#include <QSocketNotifier>

#include <inotifytools/inotifytools.h>
#include <inotifytools/inotify.h>
#include <unistd.h>

typedef std::set<std::string>::iterator dirIterator;

/**
 * @todo uniform most of the getenv("home") calls so that it is done once
 * and pasted around.
 */
ExternalChangeMonitor::ExternalChangeMonitor(QObject *parent)
    : QObject(parent), socketNotifier(0), isMonitoring(false)
{
    string tmpDirectory = getenv("HOME");
    tmpDirectory += "/.stopmotion";
    addDirectory(tmpDirectory);
}

ExternalChangeMonitor::~ExternalChangeMonitor()
{
    stopMonitoring();
}

void ExternalChangeMonitor::addDirectory(const string &directory)
{
    if (directory.empty() || directories.find(directory) != directories.end())
        return;

    Logger::get().logDebug("Registering directory for changelistening:");
    Logger::get().logDebug(directory.c_str());

    directories.insert(directory);

    if (isMonitoring) {
        stopMonitoring();
        startMonitoring();
    }
}

void ExternalChangeMonitor::startMonitoring()
{
    if (isMonitoring)
        stopMonitoring();

    const int inotifyFd = inotify_init();
    if (inotifyFd != -1)
        close(inotifyFd);

    if (!inotifytools_initialize()) {
        Logger::get().logWarning(strerror(inotifytools_error()));
        return;
    }

    for (dirIterator d = directories.begin(); d != directories.end(); ++d) {
        if (!inotifytools_watch_recursively(d->c_str(), IN_CLOSE_WRITE | IN_DELETE_SELF)) {
            Logger::get().logWarning("Failed to start monitoring:");
            Logger::get().logWarning(strerror(inotifytools_error()));
            inotifytools_cleanup();
            return;
        }
    }

    isMonitoring = true;
    socketNotifier = new QSocketNotifier(inotifyFd, QSocketNotifier::Read, this);
    connect(socketNotifier, SIGNAL(activated(int)), this, SLOT(readInotifyEvents(int)));
}

void ExternalChangeMonitor::stopMonitoring()
{
    delete socketNotifier;
    socketNotifier = 0;

    inotifytools_cleanup();
    isMonitoring = false;
}

void ExternalChangeMonitor::readInotifyEvents(int socket)
{
    Q_UNUSED(socket);

    QTime time;
    time.start();

    struct inotify_event *event = inotifytools_next_event(1);
    if (!event)
        return;

    do {
        string filename = inotifytools_filename_from_wd(event->wd);
        if (event->mask & IN_DELETE_SELF) {
            directories.erase(directories.find(filename));
        } else if (event->len > 0) { // IN_CLOSE_WRITE
            filename += event->name;
            DomainFacade::getFacade()->animationChanged(filename.c_str());
        }
    } while (time.msec() < 250 && (event = inotifytools_next_event(1)));
}

void ExternalChangeMonitor::suspendMonitor()
{
    if (socketNotifier)
        socketNotifier->setEnabled(false);
}

void ExternalChangeMonitor::resumeMonitor()
{
    if (socketNotifier)
        socketNotifier->setEnabled(true);
    if (!isMonitoring)
        startMonitoring();
}

