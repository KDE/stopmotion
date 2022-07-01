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

#ifndef HASH_H_
#define HASH_H_

#include <stdint.h>
#include <stdio.h>
#include <string>

/**
 * Very simple hashing function by Professor Daniel J Bernstein.
 * @par
 * Use this to calculate hashes for @ref ModelTestHelper::HashModel.
 * @note
 * If your model has some sort of hierarchical structure, don't just add all
 * the leaf parts to the hash as this will fail to hash the structure. Instead,
 * hash each part and combine these parts into the whole's hash. For each
 * part's hash, hash each sub-part and combine the hashes together into the
 * part's hash. Using hashes recursively in this way will allow the structure
 * as well as its contents to be hashed.
 */
class Hash {
	uint64_t h;
public:
	Hash();
	Hash(const Hash&);
	Hash& operator=(const Hash&);
	void add(uint64_t n);
	void addS(int64_t n);
	void add(const char* string);
	void add(Hash h)
	/**
	 * Add the contents of a file to the hash.
	 * @param fh The file handle to the file (which must be open for reading).
	 * The seek position will be reset afterwards if there is no error reading.
	 */;
	void add(FILE* fh);
	bool equals(const Hash& other) const;
	/**
	 * Appends the 16-character hex representation of the value of this hash to
	 * the string.
	 * @param out The string to be appended.
	 */
	void appendTo(std::string& out);
};

bool operator==(const Hash& a, const Hash& b);
bool operator!=(const Hash& a, const Hash& b);

#endif /* HASH_H_ */
