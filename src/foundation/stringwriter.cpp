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

#include "stringwriter.h"

#include <string.h>

StringWriter::StringWriter() : startOfLine(true) {
}

const char* StringWriter::result() const {
	return buffer.c_str();
}

void StringWriter::reset() {
	startOfLine = true;
	buffer.clear();
}

int32_t StringWriter::length() const {
	return buffer.length();
}

void StringWriter::writeChar(char c) {
	buffer.append(1, c);
}

void StringWriter::beginArgument() {
	if (!startOfLine) {
		writeChar(' ');
	}
	startOfLine = false;
}

void StringWriter::writeDigit(int32_t d) {
	writeChar(static_cast<char>('0' + d));
}

void StringWriter::writeString(const char* s) {
	beginArgument();
	writeChar('"');
	bool allowDigits = true;
	while (*s) {
		unsigned char c = *reinterpret_cast<const unsigned char*>(s);
		++s;
		if (strchr("\r\n\\\"", c)) {
			writeChar('\\');
			if (c == '\r')
				writeChar('r');
			else if (c == '\n')
				writeChar('n');
			else
				writeChar(c);
			allowDigits = true;
		} else if ((32 <= c && c < '0') || ('9' < c && c < 128)) {
			writeChar(c);
			allowDigits = true;
		} else if (allowDigits && '0' <= c && c <= '9') {
			writeChar(c);
		} else {
			writeChar('\\');
			bool started = false;
			int32_t power = 64;
			int32_t ci = c;
			while (0 < power) {
				int32_t digit = ci / power;
				digit %= 8;
				power /= 8;
				if (digit != 0)
					started = true;
				if (started || power == 1)
					writeDigit(digit);
			}
			allowDigits = false;
		}
	}
	writeChar('"');
}

void StringWriter::writeInteger(int32_t n) {
	beginArgument();
	if (n < 0) {
		writeChar('-');
		n = -n;
	}
	int power = 1;
	int nOver10 = n / 10;
	while (power <= nOver10) {
		power *= 10;
	}
	while (power) {
		int32_t digit = n / power;
		n %= power;
		power /= 10;
		writeChar('0' + digit);
	}
}

void StringWriter::writeIdentifier(const char* id) {
	beginArgument();
	while (*id) {
		writeChar(*id);
		++id;
	}
}
