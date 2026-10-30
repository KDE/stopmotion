/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "hash.h"

#include <ios>

Hash::Hash() {
	h = 5381;
}

void Hash::add(uint64_t n) {
	h = h * 33 + n;
}

void Hash::addS(int64_t n) {
	add(static_cast<uint64_t>(n));
}

void Hash::add(const char* string) {
	Hash s;
	while (*string) {
		s.add(static_cast<uint64_t>(*string));
		++string;
	}
	add(s);
}

void Hash::add(Hash h) {
	add(h.h);
}

void Hash::add(FILE* fh) {
	Hash s;
	long fpos = ftell(fh);
	fseek(fh, 0, SEEK_SET);
	char buffer[256];
	size_t r = 0;
	while (0 < (r = fread(buffer, 1, sizeof(buffer), fh))) {
		for (size_t i = 0; i != r; ++i)
			s.add(buffer[i]);
	}
	if (r == 0) {
		if (ferror(fh))
			throw std::ios_base::failure("error reading file for hash");
	}
	fseek(fh, fpos, SEEK_SET);
	add(s);
}

bool Hash::equals(const Hash& other) const {
	return h == other.h;
}

void Hash::appendTo(std::string& out) {
	out.reserve(out.length() + 16);
	for (int i = 60; i != 0; i -= 4) {
		int digit = (h >> i) & 0xF;
		if (digit < 10)
			out.append(1, '0' + digit);
		else
			out.append(1, 'A' - 10 + digit);
	}
}

bool operator==(const Hash& a, const Hash& b) {
	return a.equals(b);
}

bool operator!=(const Hash& a, const Hash& b) {
	return !a.equals(b);
}

Hash::Hash(const Hash& other) : h(other.h) {
}

Hash& Hash::operator=(const Hash& other) {
	h = other.h;
	return *this;
}
