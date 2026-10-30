/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <dlfcn.h>
#include <cstdlib>

#include "oomtestutil.h"

namespace {
using setMallocsUntilFailure_t = void(int);
using mallocsSoFar_t = long();
using init_t = void();
using wrapFileSystem_t = void(MockableFileSystem*);

init_t* init;
setMallocsUntilFailure_t* smuf;
mallocsSoFar_t* msf;
wrapFileSystem_t* wfs;
}

MockableFileSystem::~MockableFileSystem() {
}

int loadOomTestUtil() {
	// Using dlopen might cause a malloc, which would not work when we have not
	// yet wired up the real malloc by calling Init, so we have to use
	// RTLD_DEFAULT.
	// RTLD_DEFAULT searches all the libraries in the order that they were
	// loaded in order to find the requested symbol. RTLD_NEXT begins the
	// search with the library after the one we are calling from.
	// RTLD_NEXT and RTLD_DEFAULT are only available with the GNU dl library;
	// standard C dl libraries do not have this functionality.
	if (!init)
		init = reinterpret_cast<init_t*>(dlsym(RTLD_DEFAULT, "init"));
	if (!smuf)
		smuf = reinterpret_cast<setMallocsUntilFailure_t*>(dlsym(RTLD_DEFAULT,
				"realSetMallocsUntilFailure"));
	if (!msf)
		msf = reinterpret_cast<mallocsSoFar_t*>(dlsym(RTLD_DEFAULT, "realMallocsSoFar"));
	if (!wfs)
		wfs = reinterpret_cast<wrapFileSystem_t*>(dlsym(RTLD_DEFAULT, "realWrapFileSystem"));
	if (!init || !smuf || !msf || !wfs)
		return 0;
	init();
	return 1;
}

void setMallocsUntilFailure(int successes) {
	if (smuf)
		smuf(successes);
}

void cancelAnyMallocFailure() {
	if (smuf)
		smuf(-1);
}

int mallocFailuresWork() {
	if (!smuf && !loadOomTestUtil())
		return 0;
	// Call malloc through a volatile pointer, otherwise the compiler may
	// optimize the allocation away, as its result is not used.
	void* (*volatile mallocFn)(size_t) = malloc;
	smuf(0);
	void* p = mallocFn(1);
	smuf(-1);
	free(p);
	return p == nullptr;
}

long mallocsSoFar() {
	if (msf)
		return msf();
	return 0;
}

void wrapFileSystem(MockableFileSystem* mfs) {
	if (wfs)
		wfs(mfs);
}
