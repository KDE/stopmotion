/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TCACHE_H_
#define TCACHE_H_

#include <QObject>

struct TestLoader {
	using value_t = const char;
	static value_t* lastFreed;
	static value_t* lastLoaded;
	static int freeCount;
	static value_t* load(const char* p) {
		lastLoaded = p;
		return p;
	}
	static void free(value_t* p) {
		lastFreed = p;
		++freeCount;
	}
};

template<typename T> class LoadCache;

class TestCache : public QObject {
	Q_OBJECT
	LoadCache<TestLoader>* cache;
public:
	TestCache();
	~TestCache() override;
private slots:
	void gettingTwiceReturnsSameInstance();
	void leastRecentlyUsedIsFreed();
	void clearFrees();
	void dropFrees();
	void droppedItemMustBeReloaded();
};


#endif
