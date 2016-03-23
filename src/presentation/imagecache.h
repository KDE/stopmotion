/***************************************************************************
 *   Copyright (C) 2013-2016 by Linuxstopmotion contributors;              *
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

#ifndef IMAGECACHE_H_
#define IMAGECACHE_H_

class QImage;

struct SurfaceLoader;
template<typename T> class LoadCache;

class ImageCache {
	LoadCache<SurfaceLoader>* delegate;
public:
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
	QImage* get(const char* path);
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
