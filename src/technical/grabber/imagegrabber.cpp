/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "imagegrabber.h"
#include <cstring>

ImageGrabber::ImageGrabber(const char* filePath)
		: path(nullptr) {
	size_t pathLen = strlen(filePath) + 1;
	path = new char[pathLen];
	strncpy(path, filePath, pathLen);
}

ImageGrabber::~ImageGrabber() {
	delete[] path;
}

const char* ImageGrabber::filePath() const {
	return path;
}
