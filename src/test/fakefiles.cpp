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

#include "fakefiles.h"

#include <string.h>

#include "src/test/oomtestutil.h"

struct OggVorbis_File;

RealOggEmptyJpg::RealOggEmptyJpg() : delegate(0),
		fakeJpg(reinterpret_cast<FILE*>(1)),
		fakePng(reinterpret_cast<FILE*>(2)),
		fakeReads(0) {
}

RealOggEmptyJpg::~RealOggEmptyJpg() {
}

bool RealOggEmptyJpg::hasExtension(const char* filename, const char* extension) {
	const char* dot = strrchr(filename, '.');
	return dot && strcmp(dot, extension) == 0;
}

bool RealOggEmptyJpg::isSound(const char* filename) {
	return hasExtension(filename, ".test-sound");
}

bool RealOggEmptyJpg::isJpg(const char* filename) {
	return hasExtension(filename, ".jpg")
			|| hasExtension(filename, ".JPG")
			|| hasExtension(filename, ".jpeg");
}

bool RealOggEmptyJpg::isPng(const char* filename) {
	return hasExtension(filename, ".png");
}

void RealOggEmptyJpg::setDelegate(MockableFileSystem* mfs) {
	delegate = mfs;
}

FILE* RealOggEmptyJpg::fopen(const char* filename, const char* mode) {
	if (isJpg(filename)) {
		fakeReads = 0;
		return fakeJpg;
	} else if (isPng(filename)) {
		fakeReads = 0;
		return fakePng;
	} else if (isSound(filename)) {
		if (strstr(mode, "w") == 0)
			return delegate->fopen("resources/click.test-sound", mode);
		return fakePng;
	}
	return delegate->fopen(filename, mode);
}

FILE* RealOggEmptyJpg::freopen(const char* filename, const char* mode, FILE* fh) {
	if (fh == fakeJpg || fh == fakePng) {
		if (filename)
			return fopen(filename, mode);
		fakeReads = 0;
		return fh;
	}
	return delegate->freopen(filename, mode, fh);
}

int RealOggEmptyJpg::fclose(FILE* fh) {
	if (fh == fakeJpg || fh == fakePng)
		return 0;
	return delegate->fclose(fh);
}

int RealOggEmptyJpg::fflush(FILE* fh) {
	if (fh == fakeJpg || fh == fakePng)
		return 0;
	return delegate->fflush(fh);
}

size_t RealOggEmptyJpg::fread(void *out, size_t blockSize,
			 size_t blockCount, FILE *fh) {
	if (fh == fakeJpg || fh == fakePng) {
		if (0 < fakeReads)
			return 0;
		++fakeReads;
		return blockCount;
	}
	return delegate->fread(out, blockSize, blockCount, fh);
}

size_t RealOggEmptyJpg::fwrite(const void *in, size_t blockSize,
			  size_t blockCount, FILE *fh) {
	if (fh == fakeJpg || fh == fakePng)
		return blockCount;
	return delegate->fwrite(in, blockSize, blockCount, fh);
}

int RealOggEmptyJpg::access(const char *name, int /*type*/) {
	// always assume files within the workspace do not exist
	// (as access is only called to find empty slots to use in the
	// workspace) but files requested outside of the workspace exist
	return strstr(name, ".stopmotion/")? -1 : 0;
}

int RealOggEmptyJpg::ferror(FILE*) {
	return 0;
}

int RealOggEmptyJpg::unlink(const char *name) {
	static const char tmpPrefix[] = "/tmp/";
	// really delete any files in /tmp
	if (0 == strncmp(name, tmpPrefix, sizeof(tmpPrefix) - 1))
		delegate->unlink(name);
	return 0;
}

int RealOggEmptyJpg::ov_test(FILE *, OggVorbis_File *, const char *, long) {
	return 0;
}

int RealOggEmptyJpg::ov_clear(OggVorbis_File *) {
	return 0;
}

int RealOggEmptyJpg::ov_open(FILE *,OggVorbis_File *,const char *, long) {
	return 0;
}

long RealOggEmptyJpg::ov_read(OggVorbis_File *,char *,int, int, int, int, int *) {
	return 0;
}

int ov_raw_seek(OggVorbis_File *,long) {
	return 0;
}

char *RealOggEmptyJpg::getenv(const char *name) {
	return delegate->getenv(name);
}
