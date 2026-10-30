/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef OOMTESTUTIL_H_
#define OOMTESTUTIL_H_

// sadly, can't simply forward-declare FILE
#include <cstdio>

/**
 * Interface class for mock file systems, and the facade for the real file
 * system.
 */
class MockableFileSystem {
public:
	virtual ~MockableFileSystem();
	virtual void setDelegate(MockableFileSystem* mfs) = 0;
	virtual FILE* fopen(const char* filename, const char* mode) = 0;
	virtual FILE* freopen(const char* filename, const char* mode, FILE* fh) = 0;
	virtual int fclose(FILE*) = 0;
	virtual int fflush(FILE*) = 0;
	virtual size_t fread (void *out, size_t blockSize,
			     size_t blockCount, FILE *fh) = 0;
	virtual size_t fwrite (const void *in, size_t blockSize,
			      size_t blockCount, FILE *fh) = 0;
	virtual int access (const char *name, int type) = 0;
	virtual int ferror(FILE*) = 0;
	virtual int unlink(const char *name) = 0;
	virtual char* getenv(const char *name) = 0;
};

extern "C" {

/**
 * Installs the SetMallocsUntilFailure function.
 * Returns 1 on success, 0 on failure.
 * @ref SetMallocsUntilFailure will not work unless this function has been
 * called and has returned 1.
 */
int loadOomTestUtil();

/**
 * Sets the number of successful memory allocations until one will fail.
 * Will not work unless @ref LoadOomTestUtil has been called and returned 1.
 */
void setMallocsUntilFailure(int successes);

/**
 * Sets that there will be no artificial memory allocation failures.
 */
void cancelAnyMallocFailure();

/**
 * Returns 1 if artificial memory allocation failures work, 0 otherwise.
 * They do not work if a tool such as AddressSanitizer or Valgrind replaces
 * @c malloc.
 */
int mallocFailuresWork();

/**
 * Returns a counter of the number of (successful or unsuccessful) calls to
 * @c malloc so far. Will not work unless @ref LoadOomTestUtil has been called
 * and returned 1.
 */
long mallocsSoFar();

/**
 * Wraps the mock file system object.
 * @param mfs The new object with which to wrap the file system.
 * {@c setDelegate} will be called on {@a mfs} with an object representing the
 * current versions of these functions which can be used as a delegate. If 0 is
 * passed the file system will be returned to its natural unwrapped state.
 */
void wrapFileSystem(MockableFileSystem* mfs);

}

#endif
