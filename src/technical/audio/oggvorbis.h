/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef OGGVORBIS_H
#define OGGVORBIS_H

#include <cstdint>

#include <vorbis/vorbisfile.h>

#include "audioformat.h"
#include "src/domain/animation/workspacefile.h"

class ErrorHandler;

/**
 * Class for decoding of oggvorbis data to raw PCM data.
 *
 * @author Bjoern Erik Nilsen & Fredrik B. Kjoelstad
 */
class OggVorbis : public AudioFormat {
public:
	OggVorbis(const OggVorbis&) = delete;
	OggVorbis& operator=(const OggVorbis&) = delete;
	OggVorbis();
	~OggVorbis() override;

	/**
	 * Function for registering the given filename to  be an ogg file. This
	 * function checks that the file can be opened and that it is a valid
	 * ogg file.
	 * @param file the filename to register
	 */
	void setFilename(WorkspaceFile& file, ErrorHandler& e);

	int open() override;
	int close() override;
	void reset() override;
	int fillBuffer(char *audioBuffer, int numBytes) override;
	int add16bit(int16_t* audioBuffer, int count) override;
	const char* getSoundPath() const override;
	const char* getBasename() const override;

private:
	/** The ogg representation of the file registered in this class. */
	OggVorbis_File *oggFile;

	/** The filename registered in this class. Hopefully a valid ogg file. */
	WorkspaceFile filename;
	enum {
		littleEndian = 0,
		bigEndian = 1,
		wordsAreUnsigned = 0,
		wordsAreSigned = 1,
		wordsAre8Bit = 1,
		wordsAre16Bit = 2,
		wordsAre32Bit = 4
	};
};

#endif
