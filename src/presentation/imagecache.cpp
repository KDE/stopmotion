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

#include "imagecache.h"
#include "loadcache.h"

#include <SDL/SDL_image.h>

struct SurfaceLoader {
	typedef SDL_Surface value_t;
	static value_t* load(const char*);
	static void free(value_t*);
};

SurfaceLoader::value_t* SurfaceLoader::load(const char* path) {
	return IMG_Load(path);
}

void SurfaceLoader::free(SurfaceLoader::value_t* s) {
	SDL_FreeSurface(s);
}

ImageCache::ImageCache(int cacheSize) : delegate(0) {
	delegate = new LoadCache<SurfaceLoader>(cacheSize);
}

ImageCache::~ImageCache() {
	delete delegate;
}

SDL_Surface* ImageCache::get(const char* path) {
	if (!path)
		return 0;
	return delegate->get(path);
}

void ImageCache::drop(const char* path) {
	delegate->drop(path);
}

void ImageCache::clear() {
	delegate->clear();
}
