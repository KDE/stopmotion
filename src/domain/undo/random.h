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

#ifndef RANDOM_H_
#define RANDOM_H_

#include <stdint.h>
#include <string>

class RandomImpl;

/**
 * A randomness source for test-case generation.
 */
class RandomSource {
	RandomImpl* impl;
	int index;
public:
	RandomSource();
	/**
	 * Copies a randomness source; from now on both sources will produce the
	 * same results, given the same calls.
	 */
	RandomSource(const RandomSource& other);
	/**
	 * Copies a randomness source; from now on both sources will produce the
	 * same results, given the same calls.
	 */
	RandomSource& operator=(const RandomSource&);
	/**
	 * Destroys a randomness source. Any copies still keep functioning.
	 */
	~RandomSource();
	/**
	 * Random integer between 0 and RAND_MAX.
	 */
	int get();
	/**
	 * Gets a random integer. The distribution is uniform between min and max
	 * inclusive.
	 */
	int32_t getUniform(int32_t min, int32_t max);
	/**
	 * Gets a random integer. The distribution is uniform between 0 and max
	 * inclusive.
	 */
	int32_t getUniform(int32_t max);
	/**
	 * Gets a random nonnegative integer. For each possible return value n,
	 * given that the return value is at least n, the probability that it is
	 * greater than n is @code{.cpp} p/100 @endcode. In other words, pick a
	 * high value of {@c p} to get mostly large results, a low value to get
	 * mostly small results.
	 * @param 100 times the probability of incrementing the result at each
	 * stage. Must be between 1 and 99.
	 * @return A random number from 1 upwards.
	 */
	int32_t getLogInt(int32_t p);
	/**
	 * Gets a random string from the characters in the null-terminated
	 * string provided.
	 * @param [out] out The resulting random string.
	 * @param [in] characters A null-terminated string of characters from
	 * which to chose (uniformly).
	 * @param [in] allowNulls true to allow nulls also in the string, false to
	 * disallow.
	 */
	void getString(std::string& out, const char* characters,
			bool allowNulls = false);
	/**
	 * Gets a random string of alphanumeric characters.
	 */
	void getAlphanumeric(std::string& out);
};

#endif /* RANDOM_H_ */
