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

#ifndef OOMTESTUTIL_H_
#define OOMTESTUTIL_H_

// sadly, can't simply forward-declare FILE
#include <stdio.h>

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
