/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTHOME_H_
#define TESTHOME_H_

#include <cstdio>

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
	TestHome(const TestHome&) = delete;
	TestHome& operator=(const TestHome&) = delete;
	TestHome();
	~TestHome() override;
	void setDelegate(MockableFileSystem* mfs) override;
	FILE* fopen(const char* filename, const char* mode) override;
	FILE* freopen(const char* filename, const char* mode, FILE* fh) override;
	int fclose(FILE* fh) override;
	int fflush(FILE* fh) override;
	size_t fread (void *out, size_t blockSize,
			     size_t blockCount, FILE *fh) override;
	size_t fwrite (const void *in, size_t blockSize,
			      size_t blockCount, FILE *fh) override;
	int access (const char *name, int type) override;
	int ferror(FILE* fh) override;
	int unlink(const char *name) override;
	char *getenv(const char *name) override;
};

#endif
