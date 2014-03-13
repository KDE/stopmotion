/***************************************************************************
 *   Copyright (C) 2014 by Linuxstopmotion contributors;                   *
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

#ifndef STRINGITERATOR_H_
#define STRINGITERATOR_H_

class StringIterator {
public:
	virtual ~StringIterator();
	/**
	 * The total number of elements in the iteration. After this call the
	 * iteration may be at the start of the iteration or where you left it.
	 */
	virtual int count() = 0;
	/**
	 * Have we reached the end of the iteration?
	 * @return {@c false} if {@ref next} can be called.
	 */
	virtual bool atEnd() const = 0;
	/**
	 * The current value. Behaviour is undefined if {@ref atEnd} returned
	 * {@c false} (or would have done if called).
	 * @return The string. Ownership is not passed. The returned memory is not
	 * guaranteed to exist beyond the next call to {@ref next}.
	 */
	virtual const char* get() const = 0;
	/**
	 * Move to the next value. Behaviour is undefined if {@ref atEnd} returned
	 * {@c false} (or would have done if called).
	 */
	virtual void next() = 0;
};

#endif /* STRINGITERATOR_H_ */
