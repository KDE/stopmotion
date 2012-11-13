/*   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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

#include "replay.h"
#include <map>
#include <string>
#include <string.h>
#include <stdint.h>
#include <memory>

/**
 * Reads a space-separated list of strings and numbers. Strings are quoted,
 * quotes within strings can be escaped with backslashes.
 */
class StringReader {
	const char* p;
	const char* end;
	/**
	 * Writes out characters, but never more than the maximum allowed.
	 * Upon deletion, writes out the number of characters that were requested
	 * to be written out.
	 */
	class Writer {
		char* start;
		char* p;
		char* end;
		int32_t& lenOut;
	public:
		Writer(char* out, int max, int32_t& lengthOut)
			: start(out), p(out), end(out + max), lenOut(lengthOut) {
		}
		~Writer() {
			lenOut = p - start;
		}
		void operator()(char c) {
			if (p < end) {
				*p = c;
			}
			++p;
		}
	};
public:
	enum IsEof {
		isNotEof = 0,
		isEof = 1
	};
	enum IsEol {
		isNotEol = 0,
		isEol = 1
	};
	enum ParseSucceeded {
		parseFailed = 0,
		parseSucceeded = 1
	};
	StringReader(const char* input) {
		p = input;
		end = p + strlen(input);
	}
	StringReader(const char* input, int endChar) {
		p = input;
		end = strchr(input, endChar);
	}
	IsEol GetCharFromLine(char& out) {
		if (p == end)
			return isEol;
		if (*p == '\n' || *p == '\r')
			return isEol;
		out = *p;
		return isNotEol;
	}
	IsEof ChompEol() {
		if (p != end && *p == '\r') {
			++p;
		}
		if (p != end && *p == '\n') {
			++p;
		}
		return p == end? isEof : isNotEof;
	}
	IsEof ChompSpace() {
		while (p != end) {
			if (*p != ' ')
				return isNotEof;
			++p;
		}
		return isEof;
	}
	bool GetQuote() {
		if (p != end && *p == '"') {
			++p;
			return true;
		}
		return false;
	}
	bool IsEndOfLine() {
		return p == end || *p == '\n' || *p == '\r';
	}
	bool IsFinishedArgument() {
		return *p == ' ' || IsEndOfLine();
	}
	ParseSucceeded GetDigit(int32_t& digit) {
		if (p == end)
			return parseFailed;
		if (*p < '0' || '9' < *p)
			return parseFailed;
		digit = *p - '0';
		++p;
		return parseSucceeded;
	}
	ParseSucceeded GetOctalDigit(int32_t& digit) {
		if (p == end)
			return parseFailed;
		if (*p < '0' || '7' < *p)
			return parseFailed;
		digit = *p - '0';
		++p;
		return parseSucceeded;
	}
	ParseSucceeded GetNumber(uint32_t& out) {
		const char *old = p;
		if (isEof == ChompSpace())
			return parseFailed;
		int32_t n = 0;
		int32_t sign = 1;
		if (GetDigit(n) == parseFailed) {
			if (*p == '-')
				sign = -1;
			else
				return parseFailed;
		}
		int32_t soFar = n;
		while (GetDigit(n) == parseSucceeded) {
			soFar = soFar * 10 + n;
		}
		if (IsFinishedArgument()) {
			out = soFar * sign;
			return parseSucceeded;
		}
		p = old;
		return parseFailed;
	}
	ParseSucceeded GetString(int32_t& length, char* out, int32_t maxLength) {
		ChompSpace();
		if (!GetQuote())
			return parseFailed;
		Writer w(out, maxLength, length);
		char c;
		while (!GetQuote()) {
			// normal state
			if (GetCharFromLine(c) == isEol)
				return parseFailed;
			if (c != '\\') {
				w(c);
			} else {
				// backslash state
				if (GetCharFromLine(c) == isEol)
					// line continuation
					ChompEol();
				if (c == ' ')
					w(' ');
				else if (c == 'n')
					w('\n');
				else if (c == 'r')
					w('\r');
				else if ('0' <= c && c <= '7') {
					// octal
					int32_t soFar = c - '0';
					int32_t n;
					while (parseSucceeded == GetOctalDigit(n)) {
						soFar = soFar * 8 + n;
					}
				}
				else
					w(c);
			}
		}
		return parseSucceeded;
	}
};

class StringWriter {
	bool startOfLine;
public:
	StringWriter() : startOfLine(true) {
	}
	void BeginArgument() {
		if (!startOfLine) {
			WriteChar(' ');
		}
		startOfLine = false;
	}
	void WriteChar(char c) {
		// TODO: output a character (what should the output buffer look like?)
	}
	void WriteOctalDigit(int32_t d) {
		WriteChar(static_cast<char>('0' + d));
	}
	void WriteString(const char* s) {
		BeginArgument();
		WriteChar('"');
		bool allowDigits = true;
		while (s) {
			char c = *s;
			bool normalChar = false;
			if (32 < c && c < 128) {
				normalChar = allowDigits || c < '0' || '9' < c;
			}
			if (normalChar)
				WriteChar(c);
			else if (c == ' ')
				WriteChar(' ');
			else if (c == '\r')
				WriteChar('\r');
			else if (c == '\n')
				WriteChar('\n');
			else if (c == '\\')
				WriteChar('\\');
			else {
				WriteChar('\\');
				bool started = false;
				int32_t power = 64;
				int32_t ci = c;
				while (1 < power) {
					int32_t digit = ci / power;
					ci %= power;
					power /= 8;
					if (digit != 0)
						started = true;
					if (started || power == 1)
						WriteOctalDigit(digit);
				}
				allowDigits = false;
			}
		}
		WriteChar('"');
	}
	void WriteNumber(int32_t n) {
		BeginArgument();
		if (n < 0) {
			WriteChar('-');
			n = -n;
		}
		int power = 1;
		while (power * 10 <= n) {
			power *= 10;
		}
		while (power) {
			int32_t digit = n / power;
			n %= power;
			power /= 10;
			WriteChar('0' + digit);
		}
	}
};

class CommandReplayer {
	typedef std::map<std::string, CommandFactory*> map_t;
	map_t reg;
public:
	CommandReplayer() {
	}
	/**
	 * Deletes all the command factories owned.
	 */
	~CommandReplayer() {
		for (map_t::iterator i = reg.begin(); i != reg.end(); ++i)
			delete i->second;
	}
	/**
	 * Registers a factory.
	 * @param name The name by which the factory will be invoked.
	 * @param f The factory. Ownership is passed. If registration fails, f is
	 * deleted.
	 */
	void AddFactory(const char* name, CommandFactory* f) {
		std::auto_ptr<CommandFactory> a(f);
		reg[name] = f;
		a.release();
	}
	/**
	 * Returns the factory previously registered by name, or 0 if there was no
	 * such registration.
	 */
	CommandFactory* GetFactory(const char* name) {
		map_t::iterator i = reg.find(name);
		if (i == reg.end())
			return 0;
		return i->second;
	}
};

// To create a command in the first place:
// Get CommandAndDescription Factory
// Make command
// Allocate redo space
// Write command description to log
// Put command on redo stack
// Execute command
// Set a flag saying that "Done" must be written to the log before
// any other command.
// What about partial command executions?
// Write "Part" after each one, I suppose...
// Need composites to be partially doable...

// To replay a command from the log:
// Parse command and arguments.
// If it is a domain command:
//   Get command factory (not CommandAndDescription factory)
//   Make command
//   execute command

// It will be very good to have application commands (undo, redo, save) as
// objects themselves, so that we can do an "optimize" operation on opening.

// There are three different types of operation on the whole history that might
// be useful at startup or shutdown:
// 1) purge old files
// 2) trim undo length
// 3) optimize history (prune unreachable branches) -- speeds up recovery
// But how do we determine which files are "old"? We can tell the order in
// which they were created from their timestamps, but that doesn't tell us when
// they were deleted. Ideally we'd remove the least recently deleted.
// An unoptimized history will tell us the order; we need to do a depth-first
// walk (walking earlier branches first). Perhaps we only prune branches if
// they are old enough that we want to purge their files.
// If we have a setting for number of frames to keep, we can use that.
// So, don't prune unreachable branches; only prune old commands. That means
// walking the tree looking for files. We only have the tree at startup, so
// have to do it then. In fact, we don't have the tree, we only have the list
// of application commands; we infer the tree from the Undos and Redos. This
// means that the walk is already naturally in order. Store the latest access
// of each file. This means we must perform this operation as we optimzie
// the tree so that we know (for example) which files are being unreferenced in
// an undo operation.
