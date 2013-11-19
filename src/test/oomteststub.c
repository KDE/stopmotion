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

// Don't really need this line unless, for some reason, this functionality is
// placed into a C++ file.
#include "oomtestutil.h"

typedef void setMallocsUntilFailure_t(int);
typedef unsigned long mallocsSoFar_t(void);
typedef void init_t(void);

static setMallocsUntilFailure_t* smuf;
static mallocsSoFar_t* msf;

int loadOomTestUtil() {
	// Using dlopen might cause a malloc, which would not work when we have not
	// yet wired up the real malloc by calling Init, so we have to use
	// RTLD_DEFAULT.
	// RTLD_DEFAULT searches all the libraries in the order that they were
	// loaded in order to find the requested symbol. RTLD_NEXT begins the
	// search with the library after the one we are calling from.
	// RTLD_NEXT and RTLD_DEFAULT are only available with the GNU dl library;
	// standard C dl libraries do not have this functionality.
	if (smuf && msf)
		return 1;  // already initialized
	init_t* init = (init_t*)dlsym(RTLD_DEFAULT, "init");
	smuf = (setMallocsUntilFailure_t*)dlsym(RTLD_DEFAULT,
			"realSetMallocsUntilFailure");
	msf = (mallocsSoFar_t*)dlsym(RTLD_DEFAULT, "realMallocsSoFar");
	if (!init || !smuf)
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
