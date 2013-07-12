/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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

typedef void SetMallocsUntilFailure_t(int);
typedef void Init_t(void);

static SetMallocsUntilFailure_t* smuf;

int LoadOomTestUtil() {
	// Using dlopen might cause a malloc, which would not work when we have not
	// yet wired up the real malloc by calling Init, so we have to use
	// RTLD_DEFAULT.
	// RTLD_DEFAULT searches all the libraries in the order that they were
	// loaded in order to find the requested symbol. RTLD_NEXT begins the
	// search with the library after the one we are calling from.
	// RTLD_NEXT and RTLD_DEFAULT are only available with the GNU dl library;
	// standard C dl libraries do not have this functionality.
	if (smuf)
		return 1;  // already initialized
	Init_t* init = (Init_t*)dlsym(RTLD_DEFAULT, "Init");
	smuf = (SetMallocsUntilFailure_t*)dlsym(RTLD_DEFAULT,
			"RealSetMallocsUntilFailure");
	if (!init || !smuf)
		return 0;
	init();
	return 1;
}

void SetMallocsUntilFailure(int successes) {
	if (smuf)
		smuf(successes);
}