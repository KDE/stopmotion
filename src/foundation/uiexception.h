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

#ifndef UIEXCEPTION_H_
#define UIEXCEPTION_H_

#include <exception>

/**
 * Exception requiring an error message to be displayed to the user.
 */
class UiException : public std::exception {
public:
	/** Error, will stop the program. */
	enum Error {
		/** Not an error, but a warning. */
		IsWarning,
		/** Could not get exclusive lock on the command.log file. */
		failedToGetExclusiveLock,
		/** Preferences file cannot be read. */
		preferencesFileUnreadable,
		/** Preferences file cannot be loaded as XML. */
		preferencesFileMalformed,
		/** Error where the parameter is the entirety of the message. */
		ArbitraryError
	};
	/** Warning, program will continue. */
	enum Warning {
		/** Not a warning, but an error. */
		IsError,
		/** Attempt to add a file that is not a .jpeg */
		unsupportedImageType,
		/** Attempt to add a file that is not an .ogg */
		invalidAudioFormat,
		/** Attempt to add a corrupt .ogg */
		couldNotOpenFile,
		/** Sound or image could not be copied to ~/.stopmotion/images */
		failedToCopyFilesToWorkspace,
		/** Audio driver could not be initialized */
		failedToInitializeAudioDriver,
		/** Failed to write to preferences file */
		failedToWriteToPreferencesFile
	};
	explicit UiException(Warning);
	UiException(Warning, const char* param);
	explicit UiException(Error);
	UiException(Error, const char* param);
	~UiException() throw();
    const char* what() const throw();
	/**
	 * Returns the warning code, or @c IsError if it is an error.
	 */
	Warning warning() const;
	/**
	 * Returns the error code, or @c IsWarning if it is a warning.
	 */
	Error error() const;
	/**
	 * Returns the string parameter, or null if there is none.
	 */
	const char* string() const;
private:
	void setStringParam(const char*);
	Error err;
	Warning warn;
	mutable const char* stringParam;
	mutable const char* out;
};

#endif
