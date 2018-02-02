/***************************************************************************
 *   Copyright (C) 2017 by Linuxstopmotion contributors;                   *
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

#include "uiexception.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

void UiException::setStringParam(const char* s) {
	size_t length = strlen(s);
	char* buffer =  new char[length + 1];
	strncpy(buffer, s, length + 1);
	stringParam = buffer;
}


UiException::UiException(Warning w) : err(UiException::IsWarning),
		warn(w), stringParam(0), out(0) {
}

UiException::UiException(Warning w, const char* param)
		: err(UiException::IsWarning), warn(w), stringParam(0), out(0) {
	setStringParam(param);
}

UiException::UiException(Error e) : err(e), warn(UiException::IsError),
		stringParam(0), out(0) {
}

UiException::UiException(Error e, const char* param)
		: err(e), warn(UiException::IsError), stringParam(0), out(0) {
	setStringParam(param);
}

UiException::~UiException() throw() {
	delete[] out;
	delete[] stringParam;
}

UiException::Warning UiException::warning() const {
	return warn;
}

UiException::Error UiException::error() const {
	return err;
}

const char* formatError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  int n = vsnprintf (0, 0, format, args);
  va_end(args);
  char* p = new char[n];
  va_start(args, format);
  vsnprintf(p, n, format, args);
  va_end(args);
  return p;
}

const char* UiException::what() const throw () {
	if (out)
		return out;
	switch (err) {
	case IsWarning:
		switch (warn) {
		case couldNotOpenFile:
			out = formatError("Could not open file %s", stringParam);
			return out;
		case invalidAudioFormat:
			return "Did not understand the format of that audio file";
		case unsupportedImageType:
			return "That type of audio file is not supported";
		case failedToCopyFilesToWorkspace:
			out = formatError("Failed to copy the following files to the workspace: %s", stringParam);
			return out;
		case failedToInitializeAudioDriver:
			return "Failed To Initialize Audio Driver";
		case failedToWriteToPreferencesFile:
			out = formatError("Preferences file could not be written to: %s", stringParam);
			return out;
		default:
			break;
		}
		break;
	case failedToGetExclusiveLock:
		return "Failed to get an exclusive lock on command.log. Perhaps Stopmotion is already running.";
	case preferencesFileUnreadable:
		out = formatError("Preferences file cannot be read: %s", stringParam);
		return out;
	case preferencesFileMalformed:
		out = formatError("Preferences file is malformed: %s", stringParam);
		return out;
	case ArbitraryError:
		return stringParam;
	default:
		break;
	}
	return "Internal error: Threw a type of exception that was not handled.";
}

const char* UiException::string() const {
	return stringParam;
}
