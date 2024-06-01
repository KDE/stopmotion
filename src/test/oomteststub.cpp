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

#include "oomtestutil.h"

namespace {
typedef void setMallocsUntilFailure_t(int);
typedef long mallocsSoFar_t(void);
typedef void init_t(void);
typedef void wrapFileSystem_t(MockableFileSystem*);

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

long mallocsSoFar() {
	if (msf)
		return msf();
	return 0;
}

void wrapFileSystem(MockableFileSystem* mfs) {
	if (wfs)
		wfs(mfs);
}
