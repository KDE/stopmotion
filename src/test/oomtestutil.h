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

#ifndef OOMTESTUTIL_H_
#define OOMTESTUTIL_H_

// Need to make sure that these function names are not mangled in case we
// include this file from a C++ source file.
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Installs the SetMallocsUntilFailure function.
 * Returns 1 on success, 0 on failure.
 * @ref SetMallocsUntilFailure will not work unless this function has been
 * called and has returned 1.
 */
int LoadOomTestUtil();

/**
 * Sets the number of successful memory allocations until one will fail.
 * Will not work unless @ref LoadOomTestUtil has been called and returned 1.
 */
void SetMallocsUntilFailure(int successes);

/**
 * Sets that there will be no artificial memory allocation failures.
 */
void CancelAnyMallocFailure();

/**
 * Returns a counter of the number of (successful or unsuccessful) calls to
 * @c malloc so far. Will not work unless @ref LoadOomTestUtil has been called
 * and returned 1.
 */
unsigned long MallocsSoFar();

#ifdef __cplusplus
}
#endif

#endif
