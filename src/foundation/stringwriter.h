/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRINGWRITER_H_
#define STRINGWRITER_H_

#include <string>
#include <cstdint>

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
