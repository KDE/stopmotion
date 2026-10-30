/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "random.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

namespace {
const char alphanumericCharacters[] =
		"04 aqBZ\"'\\\xC3\x91\xC3\xA9";
}

// This is not thread safe, but could easily be made so
class RandomImpl {
	~RandomImpl() {
		if (nextChunk)
			nextChunk->delRef();
		nextChunk = nullptr;
	}
	RandomImpl() : refCount(1), nextIndex(0), nextChunk(nullptr) {
	}
	RandomImpl(const RandomImpl&) = delete;
	RandomImpl& operator=(const RandomImpl&) = delete;
	enum {
		chunkSize = 10
	};
	int chunk[chunkSize];
	int refCount;
	int nextIndex;
	RandomImpl* nextChunk;
public:
	static RandomImpl* create() {
		return new RandomImpl;
	}
	void addRef() {
		++refCount;
	}
	void delRef() {
		--refCount;
		if (refCount == 0)
			delete this;
	}
	static int get(RandomImpl*& pImpl, int& index) {
		if (index == chunkSize) {
			if (!pImpl->nextChunk) {
				pImpl->nextChunk = create();
			}
			RandomImpl* nc = pImpl->nextChunk;
			nc->addRef();
			pImpl->delRef();
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

RandomSource::RandomSource() : impl(RandomImpl::create()), index(0) {
}

RandomSource::RandomSource(const RandomSource& other)
		: impl(other.impl), index(other.index) {
	impl->addRef();
}

RandomSource::~RandomSource() {
	impl->delRef();
}

RandomSource& RandomSource::operator=(const RandomSource& other) {
	other.impl->addRef();
	impl->delRef();
	impl = other.impl;
	index = other.index;
	return*this;
}

int RandomSource::get() {
	return RandomImpl::get(impl, index);
}

int32_t RandomSource::getUniform(int32_t min, int32_t max) {
	int64_t r = get();
	return r * (max + 1 - min) / ((int64_t) RAND_MAX + 1) + min;
}

int32_t RandomSource::getUniform(int32_t max) {
	return getUniform(0, max);
}

int32_t RandomSource::getLogInt(int32_t p) {
	assert(0 < p && p < 100);
	int32_t r = 0;
	while (getUniform(0,99) < r) {
		++r;
	}
	return r;
}

void RandomSource::appendString(std::string& out,
		const char* characters, bool allowNulls) {
	int n = strlen(characters) + (allowNulls? 1 : 0);
	while (true) {
		int r = getUniform(n);
		if (n == r)
			return;
		out.append(1, characters[r]);
	}
}

void RandomSource::appendAlphanumeric(std::string& out) {
	appendString(out, alphanumericCharacters, false);
}

char RandomSource::getCharacter() {
	int u = getUniform(sizeof(alphanumericCharacters) - 2);
	return alphanumericCharacters[u];
}
