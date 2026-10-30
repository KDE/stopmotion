/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "testhome.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "src/test/oomtestutil.h"

TestHome::TestHome() : delegate(nullptr), fakeHome(nullptr) {
	std::string tmpdir("/tmp/t_home_lsmXXXXXX");
	if (mkdtemp(tmpdir.data())) {
		std::string::size_type bufferSize = tmpdir.length() + 1;
		fakeHome = static_cast<char*>(malloc(bufferSize));
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
	free(fakeHome);
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

size_t TestHome::fread(void *out, size_t blockSize,
		size_t blockCount, FILE *fh) {
	return delegate->fread(out, blockSize, blockCount, fh);
}

size_t TestHome::fwrite(const void *in, size_t blockSize,
		size_t blockCount, FILE *fh) {
	return delegate->fwrite(in, blockSize, blockCount, fh);
}

int TestHome::access(const char *name, int type) {
	return delegate->access(name, type);
}

int TestHome::ferror(FILE* fh) {
	return delegate->ferror(fh);
}

int TestHome::unlink(const char *name) {
	return delegate->unlink(name);
}

char *TestHome::getenv(const char *name) {
	if (0 == strcmp(name, "HOME")) {
		return fakeHome;
	}
	return delegate->getenv(name);
}
