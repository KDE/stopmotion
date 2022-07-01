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

#ifndef TESTHOME_H_
#define TESTHOME_H_

#include <stdio.h>

#include "oomtestutil.h"

/**
 * A filesystem/environment wrapper that makes only one change to the wrapped
 * filesystem/environment: it creates a temporary directory and pretends that
 * the HOME environment variable points to it.
 * @par
 * This is very useful for tests that exercise real Stopmotion functionality
 * (that assumes the presence of HOME) because some automatic test runners do
 * not have HOME set.
 */
class TestHome : public MockableFileSystem {
	MockableFileSystem* delegate;
	char* fakeHome;
public:
	TestHome();
	~TestHome();
	void setDelegate(MockableFileSystem* mfs);
	FILE* fopen(const char* filename, const char* mode);
	FILE* freopen(const char* filename, const char* mode, FILE* fh);
	int fclose(FILE* fh);
	int fflush(FILE* fh);
	size_t fread (void *out, size_t blockSize,
			     size_t blockCount, FILE *fh);
	size_t fwrite (const void *in, size_t blockSize,
			      size_t blockCount, FILE *fh);
	int access (const char *name, int type);
	int ferror(FILE* fh);
	int unlink(const char *name);
	char *getenv(const char *name);
};

#endif
