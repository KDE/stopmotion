/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tcache.h"

#include "src/presentation/loadcache.h"

#include <QTest>

namespace {
	const char* TestPath1 = "1";
	const char* TestPath2 = "2";
	const char* TestPath3 = "3";
	const char* TestPath4 = "4";
}

const char* TestLoader::lastFreed;
const char* TestLoader::lastLoaded;
int TestLoader::freeCount;

TestCache::TestCache() : cache(nullptr) {
	cache = new LoadCache<TestLoader>(3);
}

TestCache::~TestCache() {
	delete cache;
}

void TestCache::gettingTwiceReturnsSameInstance() {
	cache->clear();
	TestLoader::lastLoaded = nullptr;
	const char* r = cache->get(TestPath1);
	QVERIFY2(TestLoader::lastLoaded == TestPath1, "Did not load");
	QVERIFY2(r == TestPath1, "Did not get the correct value");
	TestLoader::lastLoaded = nullptr;
	r = cache->get(TestPath1);
	QVERIFY2(TestLoader::lastLoaded == nullptr,
			"Loaded item that should have been cache-resident");
	QVERIFY2(r == TestPath1, "Did not get the correct value");
}

void TestCache::leastRecentlyUsedIsFreed() {
	cache->clear();
	cache->get(TestPath1);
	cache->get(TestPath2);
	cache->get(TestPath3);
	TestLoader::lastFreed = nullptr;
	cache->get(TestPath4);
	QVERIFY2(TestLoader::lastFreed == TestPath1,
			"Least Recently Used not freed");
	TestLoader::lastFreed = nullptr;
	cache->get(TestPath2);
	QVERIFY2(TestLoader::lastFreed == nullptr,
			"Freed when requested item should have been cache-resident");
	TestLoader::lastFreed = nullptr;
	cache->get(TestPath1);
	QVERIFY2(TestLoader::lastFreed == TestPath3,
			"Least Recently Used not freed");
}

void TestCache::clearFrees() {
	cache->clear();
	cache->get(TestPath1);
	cache->get(TestPath2);
	cache->get(TestPath3);
	TestLoader::freeCount = 0;
	cache->clear();
	QCOMPARE(TestLoader::freeCount, 3);
	cache->get(TestPath2);
	TestLoader::lastFreed = nullptr;
	TestLoader::freeCount = 0;
	cache->clear();
	QCOMPARE(TestLoader::freeCount, 1);
	QVERIFY2(TestLoader::lastFreed == TestPath2,
			"clear() did not free resident item");
}

void TestCache::dropFrees() {
	cache->clear();
	cache->get(TestPath1);
	cache->get(TestPath2);
	cache->get(TestPath3);
	TestLoader::lastFreed = nullptr;
	TestLoader::freeCount = 0;
	cache->drop(TestPath2);
	QCOMPARE(TestLoader::freeCount, 1);
	QVERIFY2(TestLoader::lastFreed == TestPath2,
			"drop() did not free the correct item");
	TestLoader::lastFreed = nullptr;
	TestLoader::freeCount = 0;
	cache->drop(TestPath2);
	QCOMPARE(TestLoader::freeCount, 0);
}

void TestCache::droppedItemMustBeReloaded() {
	cache->clear();
	cache->get(TestPath1);
	cache->get(TestPath2);
	cache->get(TestPath3);
	cache->drop(TestPath2);
	TestLoader::lastLoaded = nullptr;
	const char* r = cache->get(TestPath2);
	QVERIFY2(TestLoader::lastLoaded == TestPath2,
			"Did not load previously dropped item");
	QVERIFY2(r == TestPath2, "Did not get the correct value");
}
