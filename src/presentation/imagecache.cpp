/*
 * SPDX-FileCopyrightText: 2013-2016 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "imagecache.h"
#include "loadcache.h"

#include <QPixmap>
#include <memory>

struct SurfaceLoader {
	using value_t = QPixmap;
	static value_t* load(const char*);
	static void free(value_t*);
};

SurfaceLoader::value_t* SurfaceLoader::load(const char* path) {
	std::unique_ptr<QPixmap> p(new QPixmap);
	if (p->load(path)) {
		return p.release();
	}
	return nullptr;
}

void SurfaceLoader::free(SurfaceLoader::value_t* s) {
	delete s;
}

ImageCache::ImageCache(int cacheSize) : delegate(nullptr) {
	delegate = new LoadCache<SurfaceLoader>(cacheSize);
}

ImageCache::~ImageCache() {
	delete delegate;
}

QPixmap* ImageCache::get(const char* path) {
	if (!path)
		return nullptr;
	return delegate->get(path);
}

void ImageCache::drop(const char* path) {
	delegate->drop(path);
}

void ImageCache::clear() {
	delegate->clear();
}
