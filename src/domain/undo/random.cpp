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

#include "random.h"

#include <stdlib.h>
#include <string.h>

// This is not thread safe, but could easily be made so
class RandomImpl {
	~RandomImpl() {
		if (nextChunk)
			nextChunk->DelRef();
		nextChunk = 0;
	}
	RandomImpl() : refCount(1), nextIndex(0), nextChunk(0) {
	}
	RandomImpl(const RandomImpl&); // unimplemented
	RandomImpl& operator=(const RandomImpl&); // unimplemented
	enum {
		chunkSize = 10
	};
	int chunk[chunkSize];
	int refCount;
	int nextIndex;
	RandomImpl* nextChunk;
public:
	static RandomImpl* Create() {
		return new RandomImpl;
	}
	void AddRef() {
		++refCount;
	}
	void DelRef() {
		--refCount;
		if (refCount == 0)
			delete this;
	}
	static int Get(RandomImpl*& pImpl, int& index) {
		if (index == chunkSize) {
			if (!pImpl->nextChunk) {
				pImpl->nextChunk = Create();
			}
			RandomImpl* nc = pImpl->nextChunk;
			nc->AddRef();
			pImpl->DelRef();
			pImpl = nc;
			index = 0;
		}
		if (pImpl->nextIndex == index) {
			pImpl->chunk[index] = rand();
			++pImpl->nextIndex;
		}
		int val = pImpl->chunk[index];
		++index;
		return val;
	}
};

RandomSource::RandomSource() : impl(RandomImpl::Create()), index(0) {
}

RandomSource::RandomSource(const RandomSource& other)
		: impl(other.impl), index(other.index) {
	impl->AddRef();
}

RandomSource::~RandomSource() {
	impl->DelRef();
}

int RandomSource::Get() {
	return RandomImpl::Get(impl, index);
}

int32_t RandomSource::GetUniform(int32_t min, int32_t max) {
	int64_t r = Get();
	return r * (max + 1 - min) / ((int64_t) RAND_MAX + 1) + min;
}

int32_t RandomSource::GetUniform(int32_t max) {
	return GetUniform(0, max);
}

void RandomSource::GetString(std::string& out,
		const char* characters, bool allowNulls) {
	int n = strlen(characters) + allowNulls? 1 : 0;
	out.clear();
	while (true) {
		int r = GetUniform(n);
		if (n == r)
			return;
		out.append(1, characters[r]);
	}
}

void RandomSource::GetAlphanumeric(std::string& out) {
	GetString(out,
			"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
			false);
}

RandomSource* CreateRandomSource() {
	return new RandomSource;
}