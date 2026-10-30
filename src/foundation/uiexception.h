/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef UIEXCEPTION_H_
#define UIEXCEPTION_H_

#include <exception>
#include <string>

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
	~UiException() noexcept override;
    const char* what() const noexcept override;
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
	bool hasStringParam;
	std::string stringParam;
	mutable std::string out;
};

#endif
