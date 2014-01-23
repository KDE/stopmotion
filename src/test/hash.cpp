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
