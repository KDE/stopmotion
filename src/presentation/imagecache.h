/*
 * SPDX-FileCopyrightText: 2013-2016 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef IMAGECACHE_H_
#define IMAGECACHE_H_

class QPixmap;

struct SurfaceLoader;
template<typename T> class LoadCache;

class ImageCache {
	LoadCache<SurfaceLoader>* delegate;
public:
	ImageCache(const ImageCache&) = delete;
	ImageCache& operator=(const ImageCache&) = delete;
	/**
	 * Constructs an image cache.
	 * @param cacheSize The number of images that the cache should hold.
	 */
	ImageCache(int cacheSize);
	~ImageCache();
	/**
	 * Pulls the named image into the cache, if necessary, and returns it.
	 * @param path The path of the file.
	 */
	QPixmap* get(const char* path);
	/**
	 * Removes the named image from the cache, if it is present.
	 * @param path The path of the file.
	 */
	void drop(const char* path);
	/**
	 * Clears the cache.
	 */
	void clear();
};

#endif
