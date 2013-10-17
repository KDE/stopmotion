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

#ifndef IMAGECACHE_H_
#define IMAGECACHE_H_

#include <string>

class SDL_Surface;

/**
 * Type parameter {@c T} has the following members:
 *  {@c T::value_t} Type of value loaded.
 *  {@code{.cpp} T::value_t* T::load(const char* path) @endcode} Load the file.
 *  {@code{.cpp} T::free(T::value_t*) @endcode} Free the value.
 */
template<typename T> class LoadCache {
public:
	/**
	 * Constructs an image cache.
	 * @param cacheSize The number of images that the cache should hold.
	 */
	LoadCache(int cacheSize);
	~LoadCache();
	/**
	 * Pulls the named image into the cache, if necessary, and returns it.
	 * @param path The path of the file.
	 */
	typename T::value_t* get(const char* path);
	/**
	 * Removes the named image from the cache, if it is present.
	 * @param path The path of the file.
	 */
	void drop(const char* path);
	/**
	 * Clears the cache.
	 */
	void clear();
private:
	class Item {
		std::string n;
		typename T::value_t* v;
		Item* next;
		Item(const Item&);
		Item& operator=(const Item&);
	public:
		Item();
		~Item();
		/**
		 * Clear the path and value (so both will return null).
		 * Any previously returned path and value is destroyed.
		 */
		void clear();
		/**
		 * @return Ownership is not returned.
		 */
		const char* path() const;
		/**
		 * @return Ownership is not returned.
		 */
		typename T::value_t* value() const;
		/**
		 * Sets the path and value.
		 * @param path A copy is taken, so ownership is not passed and the
		 * lifetime of the original is immaterial.
		 * @param value Ownership is passed.
		 */
		void set(const char* path, typename T::value_t* value);
		/**
		 * Find the item in a list and move it to the front, if it exists.
		 * @param [in] path The path to find.
		 * @param [in,out] head Pointer to the head of the list. On exit,
		 * this will point to the found element (if found).
		 * @return The found element, or NULL if no such element.
		 */
		static Item* find(const char* path, Item*& head);
		/**
		 * Find the item in a list and move it to freeHead front, if it exists.
		 * If not, move the last element to {@c free} and clear it.
		 * @param [in] path The path to find.
		 * @param [in,out] head Pointer to the head of the list. On exit,
		 * this will point to the found element (if found).
		 * @param [in,out] freeHead Pointer to pointer to the head of the list
		 * to receive a cleared element if not found. If NULL acts the same as
		 * {@ref find} (i.e. does not move the last element on failure to find)
		 * @return The found element, or NULL if no such element.
		 */
		static Item* findOrFree(const char* path, Item*& head,
				Item** freeHead);
		/**
		 * Move the head of one list to another.
		 * @param [in,out] to The head of the list to receive the element.
		 * On return, this will hold the transferred element.
		 * @param [in,out] from The head of the list that needs its head
		 * removed. On return this will point to the second element in the
		 * list, or be NULL if there is none.
		 * @note if to and from are the same, this function has no effect.
		 */
		static void move(Item*& to, Item*& from);
	};
	Item* buffer;
	Item* cacheHead;
	Item* freeHead;
};

struct SurfaceLoader {
	typedef SDL_Surface value_t;
	value_t* load(const char*);
	void free(value_t*);
};

class ImageCache : public LoadCache<SurfaceLoader> {
};

#endif
