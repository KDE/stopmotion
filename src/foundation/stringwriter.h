/***************************************************************************
 *   Copyright (C) 2017 by Linuxstopmotion contributors;                   *
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

#ifndef STRINGWRITER_H_
#define STRINGWRITER_H_

#include <string>
#include <stdint.h>

class StringWriter {
	bool startOfLine;
	std::string buffer;
public:
	StringWriter();
	/**
	 * Returns the written string.
	 * @return The null-terminated string written to.
	 */
	const char* result() const;
	/**
	 * Begins a new line, reusing the same buffer.
	 */
	void reset();
	/**
	 * Returns the number of characters that would have been written to the
	 * buffer if it has been long enough. If it was long enough, this will be
	 * the length of the string written into the buffer passed in SetBuffer.
	 */
	int32_t length() const;
	/**
	 * Writes a single character to the buffer.
	 */
	void writeChar(char c);
	/**
	 * Writes a space to the buffer, if we are not at the start of a line.
	 */
	void beginArgument();
	/**
	 * Writes a decimal (or octal!) digit.
	 */
	void writeDigit(int32_t d);
	/**
	 * Writes a string surrounded by double quotes.
	 * @param s The null-terminated string to write.
	 */
	void writeString(const char* s);
	/**
	 * Writes a decimal integer to the buffer. Writes negative numbers preceded
	 * with '-' and positive numbers without prefix.
	 */
	void writeInteger(int32_t n);
	/**
	 * Writes an identifier, which must not contain whitespace or backslashes.
	 * @param id The null-terminated string to write.
	 */
	void writeIdentifier(const char* id);
};

#endif
