/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "../technical/audio/oggvorbis.h"

#include <cstddef>
#include <cstdint>

#include "src/domain/animation/workspacefile.h"

class ErrorHandler;

// A mock OggVorbis implementation that does nothing.

OggVorbis::OggVorbis() {
	oggFile = nullptr;
}


OggVorbis::~OggVorbis() {
	close();
}


void OggVorbis::setFilename(WorkspaceFile& file, ErrorHandler&) {
	filename = file;
}


int OggVorbis::open() {
	return 0;
}


void OggVorbis::reset() {
}


int OggVorbis::close() {
	return 0;
}


int OggVorbis::fillBuffer(char*, int numBytes) {
	return numBytes;
}


int OggVorbis::add16bit(int16_t*, int count) {
	return count;
}


const char* OggVorbis::getSoundPath() const {
	return filename.path();
}

const char* OggVorbis::getBasename() const {
	return filename.basename();
}

