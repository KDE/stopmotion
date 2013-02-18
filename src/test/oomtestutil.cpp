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

#include <malloc.h>

typedef void freehook_t (void*, const void*);
typedef void* mallochook_t (size_t, const void*);

namespace {
	long MallocsUntilFailure = 0;

	mallochook_t CountedMalloc;

	void Init() {
		__malloc_hook = CountedMalloc;
	}
	void Reset() {
		__malloc_hook = 0;
	}
	void* CountedMalloc(size_t bytes, const void*) {
		if (0 < MallocsUntilFailure &&
				0 == __sync_sub_and_fetch(&MallocsUntilFailure, 1))
			return 0;
		Reset();
		void* r = malloc(bytes);
		Init();
		return r;
	}
}

void SetMallocsUntilFailure(int successes) {
	Init();
	MallocsUntilFailure = successes + 1;
}
