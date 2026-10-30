/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRINGITERATOR_H_
#define STRINGITERATOR_H_

class StringIterator {
public:
	virtual ~StringIterator();
	/**
	 * The total number of elements in the iteration. After this call the
	 * iteration is permitted to be at the start of the iteration or where
	 * you left it, so it is safest to call this only when {@ref next} has
	 * never been called.
	 */
	virtual int count() = 0;
	/**
	 * Have we reached the end of the iteration?
	 * @return {@c false} if {@ref next} can be called.
	 */
	virtual bool atEnd() const = 0;
	/**
	 * The current value. Behaviour is undefined if {@ref atEnd} returned
	 * {@c true} (or would have done if called).
	 * @return The string. Ownership is not passed. The returned memory is not
	 * guaranteed to exist beyond the next call to {@ref next}.
	 */
	virtual const char* get() const = 0;
	/**
	 * Move to the next value. Behaviour is undefined if {@ref atEnd} returned
	 * {@c true} (or would have done if called).
	 */
	virtual void next() = 0;
};

#endif /* STRINGITERATOR_H_ */
