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

#include <QtTest/QtTest>

namespace {
	const char* TestPath1 = "1";
	const char* TestPath2 = "2";
	const char* TestPath3 = "3";
	const char* TestPath4 = "4";
}

const char* TestLoader::lastFreed;
const char* TestLoader::lastLoaded;

TestCache::TestCache() : cache(0) {
	cache = new LoadCache<TestLoader>(3);
}

TestCache::~TestCache() {
	delete cache;
}

void TestCache::GettingTwiceReturnsSameInstance() {
	cache->clear();
	TestLoader::lastLoaded = 0;
	const char* r = cache->get(TestPath1);
	QVERIFY2(TestLoader::lastLoaded == TestPath1, "Did not load");
	QVERIFY2(r == TestPath1, "Did not get the correct value");
	TestLoader::lastLoaded = 0;
	r = cache->get(TestPath1);
	QVERIFY2(TestLoader::lastLoaded == 0, "Spurious load");
	QVERIFY2(r == TestPath1, "Did not get the correct value");
}
