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
#include <assert.h>

// Don't really need this line unless, for some reason, this functionality is
// placed into a C++ file.
#include "oomtestutil.h"

// make malloc_t a type alias for the type of the malloc function. Now we can
// declare a pointer to a malloc function with something like:
// malloc_t* mallocFnPtr;
typedef void* malloc_t (size_t);

// in a C++ file you would also need:
//extern "C" {
//malloc_t malloc;
//}

// Which future malloc should return 0 instead of attempting to allocate memory
long MallocsUntilFailure = 0;

// Pointer to the original libc malloc function, set up by Init().
// If in a C++ file, it would be better to leave out the = 0; uninitialized
// globals are always set to 0 anyway in C++, and this happens before any
// constructors happen, whereas (I believe) this assignment to zero might
// happen after some constructors have run. It makes no difference in C,
// though.
malloc_t* RealMalloc = 0;

// Initialization function sets up the pointer to the original malloc function.
void Init() {
	RealMalloc = (malloc_t*)dlsym(RTLD_NEXT, "malloc");
	assert(RealMalloc);
}

// Our malloc does its own processing, then calls the libc malloc, if
// applicable.
void* malloc(size_t bytes) {
	if (0 < MallocsUntilFailure &&
			0 == __sync_sub_and_fetch(&MallocsUntilFailure, 1))
		return 0;
	if (!RealMalloc)
		Init();
	return RealMalloc(bytes);
}

void RealSetMallocsUntilFailure(int successes) {
	MallocsUntilFailure = successes + 1;
}
