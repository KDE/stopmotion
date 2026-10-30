/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "uiexception.h"

#include <cstdio>

void UiException::setStringParam(const char* s) {
	hasStringParam = s != nullptr;
	stringParam = s? s : "";
}


UiException::UiException(Warning w) : err(UiException::IsWarning),
		warn(w), hasStringParam(false) {
}

UiException::UiException(Warning w, const char* param)
		: err(UiException::IsWarning), warn(w), hasStringParam(false) {
	setStringParam(param);
}

UiException::UiException(Error e) : err(e), warn(UiException::IsError),
		hasStringParam(false) {
}

UiException::UiException(Error e, const char* param)
		: err(e), warn(UiException::IsError), hasStringParam(false) {
	setStringParam(param);
}

UiException::~UiException() noexcept {
}

UiException::Warning UiException::warning() const {
	return warn;
}

UiException::Error UiException::error() const {
	return err;
}

namespace {
std::string formatError(const char* format, const std::string& param) {
	int n = snprintf(nullptr, 0, format, param.c_str());
	if (n < 0)
		return format;
	std::string p(n, '\0');
	snprintf(p.data(), n + 1, format, param.c_str());
	return p;
}
}

const char* UiException::what() const noexcept {
	if (!out.empty())
		return out.c_str();
	switch (err) {
	case IsWarning:
		switch (warn) {
		case couldNotOpenFile:
			out = formatError("Could not open file %s", stringParam);
			return out.c_str();
		case invalidAudioFormat:
			return "Did not understand the format of that audio file";
		case unsupportedImageType:
			return "That type of audio file is not supported";
		case failedToCopyFilesToWorkspace:
			out = formatError("Failed to copy the following files to the workspace: %s", stringParam);
			return out.c_str();
		case failedToInitializeAudioDriver:
			return "Failed To Initialize Audio Driver";
		case failedToWriteToPreferencesFile:
			out = formatError("Preferences file could not be written to: %s", stringParam);
			return out.c_str();
		default:
			break;
		}
		break;
	case failedToGetExclusiveLock:
		return "Failed to get an exclusive lock on command.log. Perhaps Stopmotion is already running.";
	case preferencesFileUnreadable:
		out = formatError("Preferences file cannot be read: %s", stringParam);
		return out.c_str();
	case preferencesFileMalformed:
		out = formatError("Preferences file is malformed: %s", stringParam);
		return out.c_str();
	case ArbitraryError:
		return stringParam.c_str();
	default:
		break;
	}
	return "Internal error: Threw a type of exception that was not handled.";
}

const char* UiException::string() const {
	return hasStringParam? stringParam.c_str() : nullptr;
}
