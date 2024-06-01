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

TestCache::TestCache() : cache(0) {
	cache = new LoadCache<TestLoader>(3);
}

TestCache::~TestCache() {
	delete cache;
}

void TestCache::gettingTwiceReturnsSameInstance() {
	cache->clear();
	TestLoader::lastLoaded = 0;
	const char* r = cache->get(TestPath1);
	QVERIFY2(TestLoader::lastLoaded == TestPath1, "Did not load");
	QVERIFY2(r == TestPath1, "Did not get the correct value");
	TestLoader::lastLoaded = 0;
	r = cache->get(TestPath1);
	QVERIFY2(TestLoader::lastLoaded == 0,
			"Loaded item that should have been cache-resident");
	QVERIFY2(r == TestPath1, "Did not get the correct value");
}

void TestCache::leastRecentlyUsedIsFreed() {
	cache->clear();
	cache->get(TestPath1);
	cache->get(TestPath2);
	cache->get(TestPath3);
	TestLoader::lastFreed = 0;
	cache->get(TestPath4);
	QVERIFY2(TestLoader::lastFreed == TestPath1,
			"Least Recently Used not freed");
	TestLoader::lastFreed = 0;
	cache->get(TestPath2);
	QVERIFY2(TestLoader::lastFreed == 0,
			"Freed when requested item should have been cache-resident");
	TestLoader::lastFreed = 0;
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
	TestLoader::lastFreed = 0;
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
	TestLoader::lastFreed = 0;
	TestLoader::freeCount = 0;
	cache->drop(TestPath2);
	QCOMPARE(TestLoader::freeCount, 1);
	QVERIFY2(TestLoader::lastFreed == TestPath2,
			"drop() did not free the correct item");
	TestLoader::lastFreed = 0;
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
	TestLoader::lastLoaded = 0;
	const char* r = cache->get(TestPath2);
	QVERIFY2(TestLoader::lastLoaded == TestPath2,
			"Did not load previously dropped item");
	QVERIFY2(r == TestPath2, "Did not get the correct value");
}
