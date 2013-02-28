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

// need to avoid name mangling for malloc if using C++
extern "C" {
void* malloc(size_t);
void SetMallocsUntilFailure(int successes);
void Init();
}

typedef void* malloc_t (size_t);

long MallocsUntilFailure = 0;

malloc_t* RealMalloc;

void Init() {
	RealMalloc = (malloc_t*)dlsym(RTLD_NEXT, "malloc");
	assert(RealMalloc);
}

void* malloc(size_t bytes) {
	if (0 < MallocsUntilFailure &&
			0 == __sync_sub_and_fetch(&MallocsUntilFailure, 1))
		return 0;
	void* r = RealMalloc(bytes);
	return r;
}

void SetMallocsUntilFailure(int successes) {
	if (!RealMalloc)
		Init();
	MallocsUntilFailure = successes + 1;
}
