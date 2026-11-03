/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "imagegrabber.h"

ImageGrabber::ImageGrabber(const char* filePath)
		: path(filePath) {
}

ImageGrabber::~ImageGrabber() {
}

const char* ImageGrabber::filePath() const {
	return path.c_str();
}
