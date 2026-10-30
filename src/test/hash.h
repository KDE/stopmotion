/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef HASH_H_
#define HASH_H_

#include <cstdint>
#include <cstdio>
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
