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
#include "logger.h"

#include <stdarg.h>
#include <qdebug.h>
#include <qstring.h>
#include <QtCore/qglobal.h>

Logger Logger::get() {
	return logger;
}

#ifndef NO_DEBUG
void Logger::logDebug(const char * msg, ...) {
	va_list ap;
	va_start(ap, msg);
	QString qsm = QString::vasprintf(msg, ap);
	va_end(ap);
	QDebug qdebug(QtDebugMsg);
	qdebug << qsm;
}
#else
void Logger::logDebug(const char *, ...) {
}
#endif

void Logger::logWarning(const char * msg, ...) {
	va_list ap;
	va_start(ap, msg);
	QString qsm;
	qsm.vasprintf(msg, ap);
	va_end(ap);
	QDebug qdebug(QtWarningMsg);
	qdebug << qsm;
}

void Logger::logFatal(const char * msg, ...) {
	va_list ap;
	va_start(ap, msg);
	QString qsm;
	qsm.vasprintf(msg, ap);
	va_end(ap);
	QDebug qdebug(QtFatalMsg);
	qdebug << qsm;
}
