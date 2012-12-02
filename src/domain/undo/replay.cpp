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
		/**
		 * Constructs a Writer operating on a buffer.
		 * @param out The start of the buffer. After destruction of this
		 * `Writer`, will hold a null-terminated string of all the characters
		 * written, but only if `lengthOut` holds a value no greater than
		 * `max`.
		 * @param max The length of the buffer. This Writer will not write any
		 * characters beyond `out + max - 1`.
		 * @param lengthOut After the destruction of this `Writer`, `lengthOut`
		 * will hold the number of characters that were attempted to be
		 * written to the buffer. If this is no greater than `max`, `out` will
		 * hold a null-terminated string of the characters written.
		 */
		Writer(char* out, int max, int32_t& lengthOut)
			: start(out), p(out), end(out + max), lenOut(lengthOut) {
		}
		/**
		 * Destructor null-terminates the string (if there is room) and sets
		 * `lengthOut` to be the number of characters (including the null
		 * termination character) attempted to be written to `out`.
		 */
		~Writer() {
			(*this)('\0');
			lenOut = p - start;
		}
		/**
		 * Outputs a character if it will fit in the buffer.
		 */
		void operator()(char c) {
			if (p < end) {
				*p = c;
			}
			++p;
		}
	};
public:
	/**
	 * Indicates whether the end of the input buffer has been reached.
	 */
	enum IsEof {
		isNotEof = 0,
		isEof = 1
	};
	/**
	 * Indicates whether the end of a line has been reached.
	 */
	enum IsEol {
		isNotEol = 0,
		isEol = 1
	};
	/**
	 * Indicates whether the parse succeeded.
	 */
	enum ParseSucceeded {
		parseFailed = 0,
		parseSucceeded = 1
	};
	/**
	 * Constructs a StringReader without a buffer. The buffer must be set with
	 * SetBuffer before use.
	 */
	StringReader() : p(0), end(0) {
	}
	/**
	 * Constructs a `StringReader`.
	 * @param input The null-terminated buffer to read.
	 */
	StringReader(const char* input) {
		p = input;
		end = p + strlen(input);
	}
	/**
	 * Constructs a `StringReader`.
	 * @param input The start of the buffer to read.
	 * @param inputEnd The end of the buffer to read.
	 */
	StringReader(const char* input, const char* endInput)
		: p(input), end(endInput) {
	}
	const char* GetPos() const {
		return p;
	}
	void SetPos(const char* pos) {
		p = pos;
	}
	/**
	 * Sets the buffer to be read.
	 * @param input The start of the buffer to read.
	 * @param inputEnd The end of the buffer to read.
	 */
	void SetBuffer(const char* input, const char* endInput) {
		p = input;
		end = endInput;
	}
	/**
	 * Sets the buffer to be read.
	 * @param input The null-terminated string to read.
	 */
	void SetBuffer(const char* input) {
		p = input;
		end = input + strlen(input);
	}
	/**
	 * Gets the next character from the input buffer, as long as it isn't an
	 * end-of-line character.
	 * @param out Stores the character output, only if `isNotEol` is returned.
	 * @return `isNotEol` on success; in this case `out` will be set. `isEol`
	 * if we are at the end of the buffer or the next character is a line-
	 * termination character, in which case `out` will not be set.
	 */
	IsEol GetCharFromLine(char& out) {
		if (p == end)
			return isEol;
		if (*p == '\n' || *p == '\r')
			return isEol;
		out = *p;
		++p;
		return isNotEol;
	}
	/**
	 * Consume an end-of-line marker (\r, \n or \r\n), if present at the
	 * position we are reading from.
	 * @return `isEof` if we have reached the end of the buffer, whether or not
	 * we consumed an end-of-line marker. `isNotEof` otherwise.
	 */
	IsEof ChompEol() {
		if (p != end && *p == '\r') {
			++p;
		}
		if (p != end && *p == '\n') {
			++p;
		}
		return p == end? isEof : isNotEof;
	}
	/**
	 * Consume whitespace.
	 * @return `isEof` if we reached the end of the buffer, whether or not we
	 * consumed any space. `isNotEof` otherwise.
	 */
	IsEof ChompSpace() {
		while (p != end) {
			if (*p != ' ')
				return isNotEof;
			++p;
		}
		return isEof;
	}
	/**
	 * Consumes a quote, if present.
	 * returns true if a quote was consumed, false otherwise.
	 */
	bool GetQuote() {
		if (p != end && *p == '"') {
			++p;
			return true;
		}
		return false;
	}
	/**
	 * Tests whether we are at the end of the current line.
	 * @return true if we are at the end of the buffer or the next character is
	 * an end-of-line marker.
	 */
	bool IsEndOfLine() {
		return p == end || *p == '\n' || *p == '\r';
	}
	/**
	 * Tests whether we are at an argument-delimiting character.
	 * @returns true if we are at a whitespace, the end of the line or the end
	 * of the buffer. false otherwise.
	 */
	bool IsFinishedArgument() {
		return *p == ' ' || *p == '!' || *p == '.' || IsEndOfLine();
	}
	/**
	 * Return the end-of-command marker, which is zero or more dots followed by
	 * zero or one exclamation mark followed by the end of the line.
	 * @param dotCount Number of dots read, if parseSucceeded is returned.
	 * @param exclamation True if there was an exclamation mark and
	 * parseSceeded is returned.
	 * @return parseSucceeded if this is an end-of-command marker, parseFailed
	 * if not. If parseFailed, dotCount and exclamation may be set to arbitrary
	 * values, and the parse will be returned to how it was before the call.
	 */
	ParseSucceeded GetEndOfCommand(int32_t& dotCount, bool& exclamation) {
		const char* old = p;
		dotCount = 0;
		exclamation = false;
		char c;
		while (isEol != GetCharFromLine(c)) {
			if (!exclamation && c == '.') {
				++dotCount;
			} else if (!exclamation && c == '!') {
				exclamation = true;
			} else {
				p = old;
				return parseFailed;
			}
		}
		return parseSucceeded;
	}
	/**
	 * Consumes a decimal digit, if one is next in the buffer. Does not consume
	 * non-digit characters.
	 * @param digit The digit output, if `parseSuceeded` is returned.
	 * @return `parseSucceeded` on success; in this case `digit` will be set.
	 * `parseFailed` if the next character is not a digit. In this case `digit`
	 * will not be set and no characters will have been consumed.
	 */
	ParseSucceeded GetDigit(int32_t& digit) {
		if (p == end)
			return parseFailed;
		if (*p < '0' || '9' < *p)
			return parseFailed;
		digit = *p - '0';
		++p;
		return parseSucceeded;
	}
	/**
	 * As for `GetDigit` but '8' and '9' are not considered digits and are
	 * not consumed.
	 */
	ParseSucceeded GetOctalDigit(int32_t& digit) {
		if (p == end)
			return parseFailed;
		if (*p < '0' || '7' < *p)
			return parseFailed;
		digit = *p - '0';
		++p;
		return parseSucceeded;
	}
	/**
	 * Consume a number, possibly negative (although '+' is not consumed and
	 * causes a parse failure).
	 * @param `out` The parsed number; only if `parseSucceeded` is returned.
	 * @return `parseSucceeded` if the next characters were possibly a '-',
	 * a string of numbers then an argument delimiter. All except the
	 * delimiter are consumed. `out` is set to the parsed number. In other
	 * cases, no characters are consumed, `out` is not set and `parseFailed` is
	 * returned.
	 */
	ParseSucceeded GetNumber(int32_t& out) {
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
			++p;
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
	/**
	 * Gets a string. The string must be in double quotes and the final quote
	 * must be followed by an argument delimiter. The string and its quotes are
	 * consumed. Line terminations within the string are not permitted.
	 * Double quotes and backslashes must be quoted with backslashes. '\r' and
	 * '\n' must be used for carriage return and line-feed characters. \nnn can
	 * be used for ASCII codes in octal-- these must not be followed by another
	 * digit (if present, this should be quoted in octal as well).
	 * @param length The length of the string that would be required to hold
	 * the string parsed (or the actual length if no greater than `maxLength`)
	 * @param out The output buffer; will be null-terminated as long as
	 * `length` returns no greater than `maxLength`.
	 * @param maxLength The length of the output buffer.
	 * @return `parseSuccessful` on success. `length` is set and `out` is
	 * filled. `parseFailed` on failure. Nothing is consumed. Some of `out` may
	 * have been set and `length` may have been overwritten.
	 */
	ParseSucceeded GetString(int32_t& length, char* out, int32_t maxLength) {
		const char *old = p;
		ChompSpace();
		if (!GetQuote()) {
			p = old;
			return parseFailed;
		}
		Writer w(out, maxLength, length);
		char c;
		while (!GetQuote()) {
			// normal state
			if (GetCharFromLine(c) == isEol) {
				p = old;
				return parseFailed;
			}
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
	/**
	 * Reads an identifier.
	 * @param length The length of the string that would be required to hold
	 * the string parsed (or the actual length if no greater than `maxLength`)
	 * @param out The output buffer; will be null-terminated as long as
	 * `length` returns no greater than `maxLength`.
	 * @param maxLength The length of the output buffer.
	 * @return `parseSuccessful` on success. `length` is set and `out` is
	 * filled. `parseFailed` on failure. No non-whitespace characters will have
	 * been consumed.
	 */
	ParseSucceeded GetIdentifier(int32_t& length, char* out, int32_t maxLength) {
		ChompSpace();
		if (IsEndOfLine())
			return parseFailed;
		Writer w(out, maxLength, length);
		while (!IsFinishedArgument()) {
			w(*p);
			++p;
		}
		return parseSucceeded;
	}
};

class StringWriter {
	bool startOfLine;
	char* start;
	char* p;
	char* end;
public:
	StringWriter() : startOfLine(true), start(0), p(0), end(0) {
	}
	/**
	 * Returns the buffer
	 */
	const char* Buffer() const {
		return start;
	}
	/**
	 * Begins a new line, reusing the same buffer.
	 */
	void Reset() {
		startOfLine = true;
		p = start;
	}
	/**
	 * Sets the memory for this StringWriter to write into.
	 * It will not write beyond bufferEnd. StringWriter will not put a null at
	 * the end of its string unless TerminateBuffer is called.
	 */
	void SetBuffer(char* bufferStart, char* bufferEnd) {
		start = p = bufferStart;
		end = bufferEnd;
	}
	/**
	 * Returns true if and only if the characters written so
	 * far have all fitted in the buffer provided by SetBuffer.
	 */
	bool FitsInBuffer() const {
		return p <= end;
	}
	/**
	 * Returns the number of characters that would have been written to the
	 * buffer if it has been long enough. If it was long enough, this will be
	 * the length of the string written into the buffer passed in SetBuffer.
	 */
	int32_t Length() const {
		return p - start;
	}
	/**
	 * Writes a null to the string. Any further calls to writing functions will
	 * overwrite this null.
	 * @return If the null fits into the buffer, returns bufferStart as passed
	 * to SetBuffer. If the null does not fit, returns 0.
	 */
	char* TerminateBuffer() {
		if (p < end) {
			*p = '\0';
			return start;
		}
		return 0;
	}
	/**
	 * Writes a single character to the buffer, if there is room for it.
	 */
	void WriteChar(char c) {
		if (p < end) {
			*p = c;
			++p;
		}
	}
	/**
	 * Writes a space to the buffer, if we are not at the start of a line.
	 */
	void BeginArgument() {
		if (!startOfLine) {
			WriteChar(' ');
		}
		startOfLine = false;
	}
	/**
	 * Writes a decimal (or octal!) digit, if there is room.
	 */
	void WriteDigit(int32_t d) {
		WriteChar(static_cast<char>('0' + d));
	}
	/**
	 * Writes a string surrounded by double quotes. Writes as much as will fit.
	 * @param s The null-terminated string to write.
	 */
	void WriteString(const char* s) {
		BeginArgument();
		WriteChar('"');
		bool allowDigits = true;
		while (s) {
			unsigned char c = *reinterpret_cast<const unsigned char*>(s);
			bool normalChar = false;
			if (32 < c && c < 128) {
				normalChar = allowDigits || c < '0' || '9' < c;
			}
			if (normalChar)
				WriteChar(c);
			else if (strchr("\r\n\\\"", c)) {
				WriteChar('\\');
				if (c == '\r')
					WriteChar('r');
				else if (c == '\n')
					WriteChar('n');
				else
					WriteChar(c);
			} else {
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
						WriteDigit(digit);
				}
				allowDigits = false;
			}
		}
		WriteChar('"');
	}
	/**
	 * Writes a decimal number to the buffer. Writes as many digits as will
	 * fit. Writes negative numbers preceded with '-' and positive numbers
	 * without prefix.
	 */
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
	/**
	 * Writes an identifier, which must not contain whitespace or backslashes.
	 * @param id The null-terminated string to write.
	 */
	void WriteIdentifier(const char* id) {
		BeginArgument();
		while (*id) {
			WriteChar(*id);
		}
	}
};

class Buffer {
	char* buffer;
	int32_t bufferLength;
public:
	Buffer() : buffer(0), bufferLength(0) {
	}
	~Buffer() {
		Destroy();
	}
	const char* Get() const {
		return buffer;
	}
	char* Get() {
		return buffer;
	}
	int32_t Length() const {
		return bufferLength;
	}
	void Destroy() {
		delete[] buffer;
		buffer = 0;
		bufferLength = 0;
	}
	/**
	 * Reallocates the buffer if `length` is longer than the current buffer
	 * allocated. Returns true if a reallocation was necessary, false if
	 * `length` is no greater than the length of the current buffer.
	 */
	bool Reallocate(int length) {
		if (length <= bufferLength)
			return false;
		Destroy();
		buffer = new char[length];
		bufferLength = length;
		return true;
	}
};

class RealCommandAndDescriptionFactory : public CommandFactory {
	CommandFactory* delegate;
	mutable CommandReplayer::Logger* logger;
	const char* name;
	mutable Buffer buffer;
	mutable StringWriter writer;

	/**
	 * Reallocates the buffer if `writer` required more than currently
	 * allocated. Returns true if a reallocation was necessary, false if all
	 * the text fitted.
	 */
	bool ReallocateBufferIfNecessary() const {
		return buffer.Reallocate(writer.Length());
	}
	void EndWrite() const {
		writer.TerminateBuffer();
	}
	void Done() const {
		if (logger)
			logger->WriteCommand(writer.Buffer());
	}
public:
	RealCommandAndDescriptionFactory() : delegate(0), logger(0), name(0) {
	}
	~RealCommandAndDescriptionFactory() {
		delegate = 0;
		logger = 0;
		name = 0;
	}
	/**
	 * Sets the name of the command being produced. This will be written to the
	 * logger at the start of every line, to indicate what is being made.
	 * @param n Null-terminated string representing the name of the command
	 * produced by the factory passed to SetFactory. Ownership is not passed.
	 */
	void SetName(const char* n) {
		name = n;
	}
	void SetLogger(CommandReplayer::Logger* l) {
		logger = l;
	}
	/**
	 * Sets the factory that will actually produce the commands.
	 * @param f The delegate factory. Ownership is not passed.
	 */
	void SetDelegate(CommandFactory* f) {
		delegate = f;
	}
	Command& Make() const {
		do {
			writer.WriteIdentifier(name);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make());
		Done();
		return *com.release();
	}
	Command& Make(int32_t a) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteNumber(a);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(a));
		Done();
		return *com.release();
	}
	Command& Make(int32_t a, int32_t b) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteNumber(a);
			writer.WriteNumber(b);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(a, b));
		Done();
		return *com.release();
	}
	Command& Make(int32_t a, int32_t b, int32_t c) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteNumber(a);
			writer.WriteNumber(b);
			writer.WriteNumber(c);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(a, b, c));
		Done();
		return *com.release();
	}
	Command& Make(int32_t a, int32_t b, int32_t c, int32_t d) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteNumber(a);
			writer.WriteNumber(b);
			writer.WriteNumber(c);
			writer.WriteNumber(d);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(a, b, c, d));
		Done();
		return *com.release();
	}
	Command& Make(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteNumber(a);
			writer.WriteNumber(b);
			writer.WriteNumber(c);
			writer.WriteNumber(d);
			writer.WriteNumber(e);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(a, b, c, d, e));
		Done();
		return *com.release();
	}
	Command& Make(const char* s) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteString(s);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(s));
		Done();
		return *com.release();
	}
	Command& Make(int32_t a, const char* s) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteNumber(a);
			writer.WriteString(s);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(a, s));
		Done();
		return *com.release();
	}
	Command& Make(int32_t a, int32_t b, const char* s) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteNumber(a);
			writer.WriteNumber(b);
			writer.WriteString(s);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(a, b, s));
		Done();
		return *com.release();
	}
	Command& Make(int32_t a, int32_t b, int32_t c, const char* s) const {
		do {
			writer.WriteIdentifier(name);
			writer.WriteNumber(a);
			writer.WriteNumber(b);
			writer.WriteNumber(c);
			writer.WriteString(s);
			EndWrite();
		} while (ReallocateBufferIfNecessary());
		std::auto_ptr<Command> com(&delegate->Make(a, b, c, s));
		Done();
		return *com.release();
	}
};

class RealCommandReplayer : public CommandReplayer {
	typedef std::map<std::string, CommandFactory*> map_t;
	map_t reg;
	RealCommandAndDescriptionFactory* describer;
	Buffer buffer;
	StringReader reader;
	/**
	 * Number of dots following the command, if it parsed successfully.
	 */
	int32_t dotCount;
	/**
	 * Whether an exclamation mark follows the command, if it parsed
	 * successfully.
	 */
	bool exclamation;

	void FailParse() {
		throw CommandFactoryParseFailureException();
	}
	/**
	 * Gets an identifier from a StringReader and puts it into the buffer.
	 */
	void GetIdentifier() {
		int32_t len;
		const char* pos = reader.GetPos();
		do {
			reader.SetPos(pos);
			if (reader.GetIdentifier(len, buffer.Get(), buffer.Length())
					== StringReader::parseFailed)
				FailParse();
		} while (buffer.Reallocate(len));
	}
	/**
	 * Gets a string from a StringReader and puts it into the buffer.
	 * Returns whether the parse succeeded or not.
	 */
	bool GetString() {
		int32_t len;
		const char* pos = reader.GetPos();
		do {
			reader.SetPos(pos);
			if (reader.GetString(len, buffer.Get(), buffer.Length())
					== StringReader::parseFailed)
				return false;
		} while (buffer.Reallocate(len));
		return true;
	}
	/**
	 * Gets a number from a StringReader and returns it. Throws an
	 * exception on parse failure.
	 */
	int32_t GetNumber() {
		int32_t ret;
		if (reader.GetNumber(ret) == StringReader::parseFailed)
			FailParse();
		return ret;
	}
	/**
	 * Parses the end of the string that was passed into reader.
	 * @return true if we are at the end of the command, false otherwise.
	 */
	bool GetEnd() {
		return reader.GetEndOfCommand(dotCount, exclamation)
				== StringReader::parseSucceeded;
	}
public:
	RealCommandReplayer() : describer(0), dotCount(0), exclamation(false) {
		describer = new RealCommandAndDescriptionFactory();
	}
	/**
	 * Deletes all the command factories owned.
	 */
	~RealCommandReplayer() {
		for (map_t::iterator i = reg.begin(); i != reg.end(); ++i)
			delete i->second;
	}
	void SetLogger(Logger* l) {
		describer->SetLogger(l);
	}
	/**
	 * Registers a factory.
	 * @param name The name by which the factory will be invoked.
	 * @param f The factory. Ownership is passed. If registration fails, f is
	 * deleted.
	 */
	void RegisterCommandFactory(const char* name, CommandFactory& f) {
		std::auto_ptr<CommandFactory> a(&f);
		reg[name] = &f;
		a.release();
	}
	/**
	 * Returns the factory previously registered by name, or 0 if there was no
	 * such registration.
	 * @param The name as a null-terminated string. Ownership is not passed.
	 * @returns The  command factory. Ownership is not returned.
	 */
	CommandFactory* GetFactory(const char* name) {
		map_t::iterator i = reg.find(name);
		if (i == reg.end())
			return 0;
		return i->second;
	}
	/**
	 * Returns the factory previously registered by name wrapped in a
	 * CommandAndDescriptionFactory, or 0 if no factory has been registered
	 * with this replayer by this name.
	 * @param The name as a null-terminated string. Ownership is not passed.
	 * @returns The wrapped command factory. Ownership is not returned.
	 */
	const CommandFactory* GetCommandFactory(const char* name) {
		CommandFactory* del = GetFactory(name);
		if (!del)
			return 0;
		describer->SetDelegate(del);
		return describer;
	}
	/**
	 * Parses a string (as a command type followed by arguments) to create a
	 * command.
	 * @param null or end-of-line terminated string representing the command to
	 * be created
	 * @return The constructed command.
	 */
	Command& MakeCommand(const char* s) {
		reader.SetBuffer(s);
		GetIdentifier();
		std::string id = buffer.Get();
		map_t::iterator cfi = reg.find(id);
		if (cfi == reg.end())
			throw CommandFactoryNoSuchCommandException();
		if (GetEnd()) {
			return cfi->second->Make();
		}
		if (GetString()) {
			std::string s1 = buffer.Get();
			if (GetEnd()) {
				return cfi->second->Make(s1.c_str());
			}
		}
		int32_t n1 = GetNumber();
		if (GetEnd()) {
			return cfi->second->Make(n1);
		}
		if (GetString()) {
			std::string s2 = buffer.Get();
			if (GetEnd()) {
				return cfi->second->Make(n1, s2.c_str());
			}
		}
		int32_t n2 = GetNumber();
		if (GetEnd()) {
			return cfi->second->Make(n1, n2);
		}
		if (GetString()) {
			std::string s3 = buffer.Get();
			if (GetEnd()) {
				return cfi->second->Make(n1, n2, s3.c_str());
			}
		}
		int32_t n3 = GetNumber();
		if (GetEnd()) {
			return cfi->second->Make(n1, n2, n3);
		}
		if (GetString()) {
			std::string s4 = buffer.Get();
			if (GetEnd()) {
				return cfi->second->Make(n1, n2, n3, s4.c_str());
			}
		}
		int32_t n4 = GetNumber();
		if (GetEnd()) {
			return cfi->second->Make(n1, n2, n3, n4);
		}
		int32_t n5 = GetNumber();
		if (!GetEnd()) {
			FailParse();
		}
		return cfi->second->Make(n1, n2, n3, n4, n5);
	}
};

CommandReplayer* CommandReplayer::Make() {
	return new RealCommandReplayer();
}

CommandReplayer::~CommandReplayer() {
}

CommandFactory::~CommandFactory() {
}

Command& CommandFactory::Make() const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(int32_t) const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(int32_t, int32_t) const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(int32_t, int32_t, int32_t) const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(int32_t, int32_t, int32_t, int32_t) const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(int32_t, int32_t, int32_t, int32_t, int32_t) const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(const char*) const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(int32_t, const char*) const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(int32_t, int32_t, const char*) const {
	throw CommandFactoryIncorrectParametersException();
}

Command& CommandFactory::Make(int32_t, int32_t, int32_t, const char*) const {
	throw CommandFactoryIncorrectParametersException();
}

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
