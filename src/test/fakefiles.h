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

#ifndef FAKEFILES_H_
#define FAKEFILES_H_

#include "oomtestutil.h"

#include <stdio.h>

struct OggVorbis_File;

class RealOggEmptyJpg : public MockableFileSystem {
	MockableFileSystem* delegate;
	FILE* fakeJpg;
	FILE* fakePng;
	int fakeReads;
public:
	RealOggEmptyJpg();
	~RealOggEmptyJpg();
	bool hasExtension(const char* filename, const char* extension);
	bool isSound(const char* filename);
	bool isJpg(const char* filename);
	bool isPng(const char* filename);
	void setDelegate(MockableFileSystem* mfs);
	FILE* fopen(const char* filename, const char* mode);
	FILE* freopen(const char* filename, const char* mode, FILE* fh);
	int fclose(FILE* fh);
	int fflush(FILE* fh);
	size_t fread(void *out, size_t blockSize, size_t blockCount, FILE *fh);
	size_t fwrite (const void *in, size_t blockSize, size_t blockCount, FILE *fh);
	int access (const char *name, int type);
	int ferror(FILE*);
	int unlink(const char *name);
	int ov_test(FILE *, OggVorbis_File *, const char *, long);
	int ov_clear(OggVorbis_File *);
	int ov_open(FILE *,OggVorbis_File *,const char *, long);
	long ov_read(OggVorbis_File *,char *,int, int, int, int, int *);
	char *getenv(const char *name);
};

#endif
