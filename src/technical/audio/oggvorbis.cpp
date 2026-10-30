/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "oggvorbis.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <new>

#include <vorbis/codec.h>

#include "errorhandler.h"
#include "workspacefile.h"
#include "src/foundation/logger.h"
#include "src/foundation/uiexception.h"


class FileCloser {
	FILE* h;
public:
	FileCloser(const FileCloser&) = delete;
	FileCloser& operator=(const FileCloser&) = delete;
	FileCloser(FILE* fh) : h(fh) {
	}
	FILE* release() {
		FILE* r = h;
		h = nullptr;
		return r;
	}
	~FileCloser() {
		if (h) {
			fclose(h);
		}
	}
};

OggVorbis::OggVorbis() {
	oggFile = nullptr;
}


OggVorbis::~OggVorbis() {
	close();
}


void OggVorbis::setFilename(WorkspaceFile& file, ErrorHandler& e) {
	assert(file.path() != nullptr);

	// Opens the file and tests for vorbis-ness
	FILE *f = fopen(file.path(), "r");
	FileCloser fcloser(f);
	if (f) {
		close();
		oggFile = static_cast<OggVorbis_File*>(malloc(sizeof(OggVorbis_File)));
		if (oggFile == nullptr) {
			throw std::bad_alloc();
		}

		if ( ov_test(f, oggFile, nullptr, 0) < 0 ) {
			Logger::get().logDebug("Not a valid oggfile");
			free(oggFile);
			oggFile = nullptr;
			e.error( UiException(UiException::invalidAudioFormat) );
			return;
		}
		// For some reason, ov_test does not necessarily take
		// ownership of our file handle. We must only release
		// ownership if oggFile has taken it.
		if (oggFile->datasource == f)
			fcloser.release();
		this->filename.swap(file);
	} else {
		int err = errno;
		if (err == ENOMEM) {
			throw std::bad_alloc();
		}
		Logger::get().logDebug("Cannot open file '%s' for reading", file.path());
		e.error( UiException(UiException::couldNotOpenFile) );
	}
}


int OggVorbis::open() {
	FILE *f = fopen(filename.path(), "r");
	FileCloser fcloser(f);
	if (f) {
		close();
		oggFile = static_cast<OggVorbis_File*>(malloc(sizeof(OggVorbis_File)));
		if (oggFile == nullptr) {
			throw std::bad_alloc();
		}
		if ( ov_open(f, oggFile, nullptr, 0) == 0 ) {
			fcloser.release();
			return 0;
		}
		free(oggFile);
		oggFile = nullptr;
	}
	return -1;
}


void OggVorbis::reset() {
	if (oggFile) {
		ov_raw_seek(oggFile, 0);
	}
}


int OggVorbis::close() {
	if (oggFile) {
		ov_clear(oggFile);
		free(oggFile);
		oggFile = nullptr;
		return 0;
	}
	return -1;
}


int OggVorbis::fillBuffer(char *audioBuffer, int numBytes) {
	assert(audioBuffer != nullptr);
	if (oggFile) {
		int dummy;
		int ret = ov_read(oggFile, audioBuffer, numBytes,
				littleEndian, wordsAre16Bit, wordsAreSigned, &dummy);
		if (ret < 0) {
			Logger logger = Logger::get();
			switch (ret) {
			case OV_HOLE:
				logger.logWarning("Hole in the vorbis audio data");
				break;
			case OV_EBADLINK:
				logger.logWarning("Invalid link or stream section in vorbis audio data");
				break;
			case OV_EINVAL:
				logger.logWarning("Corrupt initial headers for vorbis audio data");
				break;
			default:
				logger.logWarning("Unknown error in vorbis audio data");
				break;
			}
			return 0;
		}
		return ret;
	}
	return 0;
}


int OggVorbis::add16bit(int16_t* audioBuffer, int) {
	assert(audioBuffer);
	return 0;
}


const char* OggVorbis::getSoundPath() const {
	return filename.path();
}

const char* OggVorbis::getBasename() const {
	return filename.basename();
}

