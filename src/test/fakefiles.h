/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FAKEFILES_H_
#define FAKEFILES_H_

#include "oomtestutil.h"

#include <cstdio>

struct OggVorbis_File;

class RealOggEmptyJpg : public MockableFileSystem {
	MockableFileSystem* delegate;
	FILE* fakeJpg;
	FILE* fakePng;
	int fakeReads;
public:
	RealOggEmptyJpg();
	~RealOggEmptyJpg() override;
	bool hasExtension(const char* filename, const char* extension);
	bool isSound(const char* filename);
	bool isJpg(const char* filename);
	bool isPng(const char* filename);
	void setDelegate(MockableFileSystem* mfs) override;
	FILE* fopen(const char* filename, const char* mode) override;
	FILE* freopen(const char* filename, const char* mode, FILE* fh) override;
	int fclose(FILE* fh) override;
	int fflush(FILE* fh) override;
	size_t fread(void *out, size_t blockSize, size_t blockCount, FILE *fh) override;
	size_t fwrite (const void *in, size_t blockSize, size_t blockCount, FILE *fh) override;
	int access (const char *name, int type) override;
	int ferror(FILE*) override;
	int unlink(const char *name) override;
	int ov_test(FILE *, OggVorbis_File *, const char *, long);
	int ov_clear(OggVorbis_File *);
	int ov_open(FILE *,OggVorbis_File *,const char *, long);
	long ov_read(OggVorbis_File *,char *,int, int, int, int, int *);
	char *getenv(const char *name) override;
};

#endif
