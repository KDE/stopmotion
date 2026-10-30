/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "logger.h"

#include <cstdarg>
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
	QString qsm = QString::vasprintf(msg, ap);
	va_end(ap);
	QDebug qdebug(QtWarningMsg);
	qdebug << qsm;
}

void Logger::logFatal(const char * msg, ...) {
	va_list ap;
	va_start(ap, msg);
	QString qsm = QString::vasprintf(msg, ap);
	va_end(ap);
	QDebug qdebug(QtFatalMsg);
	qdebug << qsm;
}
