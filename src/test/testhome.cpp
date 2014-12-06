/***************************************************************************
 *   Copyright (C) 2014 by Linuxstopmotion contributors;                   *
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

#include "testhome.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

TestHome::TestHome() : delegate(0), fakeHome(0) {
	std::string tmpdir("/tmp/t_home_lsmXXXXXX");
	tmpdir.c_str(); // ensure trailing null is present
	if (mkdtemp(&tmpdir[0])) {
		std::string::size_type bufferSize = tmpdir.length() + 1;
		fakeHome = (char *) malloc(bufferSize);
		if (fakeHome) {
			strncpy(fakeHome, tmpdir.c_str(), bufferSize);
		} else {
			printf("Could not create temporary directory: Out Of Memory!\n");
		}
	} else {
		printf("Could not create temporary directory; error code: %d\n",
				errno);
	}
}

TestHome::~TestHome() {
}

void TestHome::setDelegate(MockableFileSystem* mfs) {
	delegate = mfs;
}

FILE* TestHome::fopen(const char* filename, const char* mode) {
	return delegate->fopen(filename, mode);
}

FILE* TestHome::freopen(const char* filename, const char* mode, FILE* fh) {
	return delegate->freopen(filename, mode, fh);
}

int TestHome::fclose(FILE* fh) {
	return delegate->fclose(fh);
}

int TestHome::fflush(FILE* fh) {
	return delegate->fflush(fh);
}

size_t TestHome::fread (void *out, size_t blockSize,
		     size_t blockCount, FILE *fh) {
	return delegate->fread(out, blockSize, blockCount, fh);
}

size_t TestHome::fwrite (const void *in, size_t blockSize,
		      size_t blockCount, FILE *fh) {
	return delegate->fwrite(in, blockSize, blockCount, fh);
}

int TestHome::access (const char *name, int type) {
	return delegate->access(name, type);
}

int TestHome::ferror(FILE* fh) {
	return delegate->ferror(fh);
}

int TestHome::unlink(const char *name) {
	return delegate->unlink(name);
}

int TestHome::ov_test(FILE *f, OggVorbis_File *vf, const char *initial, long ibytes) {
	return delegate->ov_test(f, vf, initial, ibytes);
}

int TestHome::ov_clear(OggVorbis_File *vf) {
	return delegate->ov_clear(vf);
}

int TestHome::ov_open(FILE *f, OggVorbis_File *vf, const char *initial, long ibytes) {
	return delegate->ov_open(f, vf, initial, ibytes);
}

long TestHome::ov_read(OggVorbis_File *vf,char *buffer, int length, int bigendianp,
		int word, int sgned, int *bitstream) {
	return delegate->ov_read(vf, buffer, length, bigendianp, word, sgned,
			bitstream);
}

char *TestHome::getenv(const char *name) {
	if (0 == strcmp(name, "HOME")) {
		return fakeHome;
	}
	return delegate->getenv(name);
}
