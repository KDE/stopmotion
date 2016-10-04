/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#include <dlfcn.h>
#include <assert.h>
#include <cerrno>
#include <new>

#include "oomtestutil.h"

// make malloc_t a type alias for the type of the malloc function. Now we can
// declare a pointer to a malloc function with something like:
// malloc_t* mallocFnPtr;
typedef void* malloc_t (size_t);

extern "C" {
malloc_t malloc;
void init();
void realSetMallocsUntilFailure(int);
long realMallocsSoFar();
void realWrapFileSystem(MockableFileSystem* mfs);
FILE* fopen(const char* filename, const char* mode);
FILE* freopen(const char* filename, const char* mode, FILE* fh);
int fclose(FILE* fh);
int fflush(FILE* fh);
size_t fread(void *out, size_t s, size_t n, FILE *fh);
size_t fwrite(const void *in, size_t s, size_t n, FILE *fh);
int access(const char *name, int type);
int ferror(FILE* fh);
int unlink(const char *name);
int ov_test(FILE *, OggVorbis_File *, const char *, long);
int ov_clear(OggVorbis_File *vf);
int ov_open(FILE *f,OggVorbis_File *vf,const char *initial, long ibytes);
long ov_read(OggVorbis_File *vf,char *buffer,int length,
		int bigendianp,int word,int sgned,int *bitstream);
char *getenv(const char *name);
}

// Which future malloc should return 0 instead of attempting to allocate memory
long mallocsUntilFailure;
long mallocCount;

// Pointer to the original libc malloc function, set up by Init().
malloc_t* realMalloc;

// Pointer to the facade of the file system, set up by Init().
MockableFileSystem* realFs;

// Pointer to the mock file system if requested, or the facade to the real file
// system if not.
MockableFileSystem* requiredFs;

MockableFileSystem::~MockableFileSystem() {
}

class RealFileSystem : public MockableFileSystem {
	typedef FILE* fopen_t(const char*, const char*);
	typedef FILE* freopen_t(const char*, const char*, FILE*);
	typedef int fclose_t(FILE*);
	typedef int fflush_t(FILE*);
	typedef size_t fread_t(void *, size_t, size_t, FILE*);
	typedef size_t fwrite_t(const void*, size_t, size_t, FILE*);
	typedef int access_t(const char*, int);
	typedef int ferror_t(FILE*);
	typedef int unlink_t(const char *);
	typedef int ov_test_t(FILE *, OggVorbis_File *, const char *, long);
	typedef int ov_clear_t(OggVorbis_File *);
	typedef int ov_open_t(FILE *, OggVorbis_File *, const char *, long);
	typedef long ov_read_t(OggVorbis_File *, char *, int, int, int, int ,int *);
	typedef char *getenv_t(const char *name);
	fopen_t* rfopen;
	freopen_t* rfreopen;
	fclose_t* rfclose;
	fflush_t* rfflush;
	fread_t* rfread;
	fwrite_t* rfwrite;
	access_t* raccess;
	ferror_t* rferror;
	unlink_t* runlink;
	ov_test_t* rov_test;
	ov_clear_t* rov_clear;
	ov_open_t* rov_open;
	ov_read_t* rov_read;
	getenv_t* rgetenv;
public:
	RealFileSystem() : rfopen(0), rfreopen(0), rfclose(0), rfflush(0),
			rfread(0), rfwrite(0), raccess(0), rferror(0), rov_test(0),
			rov_clear(0), rov_open(0), rov_read(0), rgetenv(0) {
		rfopen = (fopen_t*)dlsym(RTLD_NEXT, "fopen");
		assert(rfopen);
		rfreopen = (freopen_t*)dlsym(RTLD_NEXT, "freopen");
		assert(rfreopen);
		rfclose = (fclose_t*)dlsym(RTLD_NEXT, "fclose");
		assert(rfclose);
		rfflush = (fflush_t*)dlsym(RTLD_NEXT, "fflush");
		assert(rfflush);
		rfread = (fread_t*)dlsym(RTLD_NEXT, "fread");
		assert(rfread);
		rfwrite = (fwrite_t*)dlsym(RTLD_NEXT, "fwrite");
		assert(rfwrite);
		raccess = (access_t*)dlsym(RTLD_NEXT, "access");
		assert(raccess);
		rferror = (ferror_t*)dlsym(RTLD_NEXT, "ferror");
		assert(rferror);
		runlink = (unlink_t*)dlsym(RTLD_NEXT, "unlink");
		assert(runlink);
		rov_test = (ov_test_t*)dlsym(RTLD_NEXT, "ov_test");
		assert(rov_test);
		rov_clear = (ov_clear_t*)dlsym(RTLD_NEXT, "ov_clear");
		assert(rov_clear);
		rov_open = (ov_open_t*)dlsym(RTLD_NEXT, "ov_open");
		assert(rov_open);
		rov_read = (ov_read_t*)dlsym(RTLD_NEXT, "ov_read");
		assert(rov_read);
		rgetenv = (getenv_t*)dlsym(RTLD_NEXT, "getenv");
		assert(rgetenv);
	}
	~RealFileSystem() {
	}
	void setDelegate(MockableFileSystem*) {
	}
	FILE* fopen(const char* filename, const char* mode) {
		return rfopen(filename, mode);
	}
	FILE* freopen(const char* filename, const char* mode, FILE* fh) {
		return rfreopen(filename, mode, fh);
	}
	int fclose(FILE* fh) {
		return rfclose(fh);
	}
	int fflush(FILE* fh) {
		return rfflush(fh);
	}
	size_t fread(void *out, size_t s, size_t n, FILE *fh) {
		return rfread(out, s, n, fh);
	}
	size_t fwrite(const void *in, size_t s, size_t n, FILE *fh) {
		return rfwrite(in, s, n, fh);
	}
	int access(const char *name, int type) {
		return raccess(name, type);
	}
	int ferror(FILE* fh) {
		return rferror(fh);
	}
	int unlink(const char *name) {
		return runlink(name);
	}
	int ov_test(FILE *f, OggVorbis_File *vf, const char *initial,
			long ibytes) {
		return rov_test(f, vf, initial, ibytes);
	}
	int ov_clear(OggVorbis_File *vf) {
		return rov_clear(vf);
	}
	int ov_open(FILE *f,OggVorbis_File *vf,const char *initial,
			long ibytes) {
		return rov_open(f, vf, initial, ibytes);
	}
	long ov_read(OggVorbis_File *vf,char *buffer,int length,
			int bigendianp,int word,int sgned,int *bitstream) {
		return rov_read(vf, buffer, length, bigendianp, word, sgned,
				bitstream);
	}
	char *getenv(const char *name) {
		return rgetenv(name);
	}
};

// Initialization function sets up the pointer to the original malloc function.
void init() {
	if (!realMalloc) {
		realMalloc = (malloc_t*)dlsym(RTLD_NEXT, "malloc");
		assert(realMalloc);
	}
	if (!realFs) {
		realFs = new RealFileSystem();
		assert(realFs);
		requiredFs = realFs;
	}
}

// Our malloc does its own processing, then calls the libc malloc, if
// applicable.
void* malloc(size_t bytes) {
	__sync_add_and_fetch(&mallocCount, 1);
	if (0 < mallocsUntilFailure &&
			0 == __sync_sub_and_fetch(&mallocsUntilFailure, 1))
		return 0;
	if (!realMalloc)
		init();
	return realMalloc(bytes);
}

void realSetMallocsUntilFailure(int successes) {
	mallocsUntilFailure = successes + 1;
}

long realMallocsSoFar() {
	return mallocCount;
}

void realWrapFileSystem(MockableFileSystem* mfs) {
	if (mfs) {
		mfs->setDelegate(requiredFs);
		requiredFs = mfs;
	} else {
		requiredFs = realFs;
	}
}

FILE* fopen(const char* filename, const char* mode) {
	return requiredFs->fopen(filename, mode);
}

FILE* freopen(const char* filename, const char* mode, FILE* fh) {
	return requiredFs->freopen(filename, mode, fh);
}

int fclose(FILE* fh) {
	return requiredFs->fclose(fh);
}

int fflush(FILE* fh) {
	return requiredFs->fflush(fh);
}

size_t fread(void *out, size_t s, size_t n, FILE *fh) {
	return requiredFs->fread(out, s, n, fh);
}

size_t fwrite(const void *in, size_t s, size_t n, FILE *fh) {
	return requiredFs->fwrite(in, s, n, fh);
}

int access(const char *name, int type) {
	return requiredFs->access(name, type);
}

int ferror(FILE* fh) {
	return requiredFs->ferror(fh);
}

int unlink(const char *name) {
	return requiredFs->unlink(name);
}

int ov_test(FILE *f, OggVorbis_File *vf, const char *initial, long ibytes) {
	return requiredFs->ov_test(f, vf, initial, ibytes);
}

int ov_clear(OggVorbis_File *vf) {
	return requiredFs->ov_clear(vf);
}

int ov_open(FILE *f,OggVorbis_File *vf,const char *initial, long ibytes) {
	return requiredFs->ov_open(f, vf, initial, ibytes);
}

long ov_read(OggVorbis_File *vf,char *buffer,int length,
		int bigendianp,int word,int sgned,int *bitstream) {
	return requiredFs->ov_read(vf, buffer, length, bigendianp, word, sgned,
			bitstream);
}

char *getenv(const char *name) {
	if (!requiredFs)
		init();
	return requiredFs->getenv(name);
}
