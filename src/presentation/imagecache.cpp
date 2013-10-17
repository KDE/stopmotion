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

#include <string.h>
#include <SDL/SDL_image.h>

template<typename T>
LoadCache<T>::LoadCache(int cacheSize)
		: buffer(0), cacheHead(0), freeHead(0) {
	buffer = new Item[cacheSize < 3? 3 : cacheSize];
	// construct the free list
	for (int i = 0; i != cacheSize; ++i) {
		Item* p = buffer + i;
		Item::move(freeHead, p);
	}
}

template<typename T>
LoadCache<T>::~LoadCache() {
	clear();
	delete[] buffer;
}

template<typename T>
typename T::value_t* LoadCache<T>::get(const char* path) {
	Item* p = Item::findOrFree(path, cacheHead, &freeHead);
	if (p)
		return p->value();
	typename T::value_t* value = T::load(path);
	freeHead->set(path, value);
	Item::move(cacheHead, freeHead);
	return cacheHead->value();
}

template<typename T>
void LoadCache<T>::drop(const char* path) {
	Item* p = Item::find(path, cacheHead);
	if (p) {
		cacheHead->clear();
		Item::move(freeHead, cacheHead);
	}
}

template<typename T>
void LoadCache<T>::clear() {
	while (cacheHead) {
		cacheHead->clear();
		Item::move(freeHead, cacheHead);
	}
}

template<typename T>
LoadCache<T>::Item::Item() : v(0), next(0) {
}

template<typename T>
LoadCache<T>::Item::~Item() {
	clear();
}

template<typename T>
void LoadCache<T>::Item::clear() {
	n.clear();
	T::free(v);
	v = 0;
}

template<typename T>
const char* LoadCache<T>::Item::path() const {
	return n.c_str();
}

template<typename T>
typename T::value_t* LoadCache<T>::Item::value() const {
	return v;
}

template<typename T>
void LoadCache<T>::Item::set(const char* path, typename T::value_t* value) {
	clear();
	n = path;
	v = value;
}

template<typename T>
typename LoadCache<T>::Item* LoadCache<T>::Item::find(
		const char* path, Item*& head) {
	return findOrFree(path, head, 0);
}

template<typename T>
typename LoadCache<T>::Item* LoadCache<T>::Item::findOrFree(
		const char* path, Item*& head, Item** freeHead) {
	Item** pp = &head;
	Item** pLast = 0;
	while (*pp) {
		if (0 == strcmp((*pp)->n.c_str(), path)) {
			move(head, *pp);
			return head;
		}
		pLast = pp;
		pp = &(*pp)->next;
	}
	if (freeHead && pLast) {
		move(*freeHead, *pLast);
	}
	return 0;
}

template<typename T>
void LoadCache<T>::Item::move(Item*& to, Item*& from) {
	if (from) {
		Item* newTo = from;
		Item* newNext = to;
		Item* newFrom = newTo->next;
		from = newFrom;
		to = newTo;
		newTo->next = newNext;
	}
}

SurfaceLoader::value_t* SurfaceLoader::load(const char* path) {
	return IMG_Load(path);
}

void SurfaceLoader::free(SurfaceLoader::value_t* s) {
	SDL_FreeSurface(s);
}
