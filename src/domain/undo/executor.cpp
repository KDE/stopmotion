/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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

#include "executor.h"
#include "command.h"
#include "random.h"
#include "commandlogger.h"

#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <memory>

/**
 * Reads a space-separated list of strings and numbers. Strings are quoted,
 * quotes within strings can be escaped with backslashes.
 */
class StringReader {
	const char* p;
	const char* end;
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
	 * Constructs a @c StringReader without a buffer. The buffer must be set with
	 * @c SetBuffer before use.
	 */
	StringReader() : p(0), end(0) {
	}
	/**
	 * Constructs a @c StringReader.
	 * @param input The null-terminated buffer to read.
	 */
	StringReader(const char* input) {
		p = input;
		end = p + strlen(input);
	}
	/**
	 * Constructs a @c StringReader.
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
	ParseSucceeded GetInteger(int32_t& out) {
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
	 * @param out Returns the parsed and decoded string.
	 * @return @c parseSuccessful on success. @c out is filled.
	 * @c parseFailed on failure. Nothing is consumed. Some of @c out may
	 * have been set.
	 */
	ParseSucceeded GetString(std::string& out) {
		out.clear();
		const char *old = p;
		ChompSpace();
		if (!GetQuote()) {
			p = old;
			return parseFailed;
		}
		char c;
		while (!GetQuote()) {
			// normal state
			if (GetCharFromLine(c) == isEol) {
				p = old;
				return parseFailed;
			}
			if (c != '\\') {
				out.append(1, c);
			} else {
				// backslash state
				if (GetCharFromLine(c) == isEol)
					// line continuation
					ChompEol();
				if (c == ' ')
					out.append(1, ' ');
				else if (c == 'n')
					out.append(1, '\n');
				else if (c == 'r')
					out.append(1, '\r');
				else if ('0' <= c && c <= '7') {
					// octal
					int32_t soFar = c - '0';
					int32_t n;
					while (parseSucceeded == GetOctalDigit(n)) {
						soFar = soFar * 8 + n;
					}
					out.append(1, soFar);
				}
				else
					out.append(1, c);
			}
		}
		return parseSucceeded;
	}
	/**
	 * Reads an identifier.
	 * @param out Returns the parsed and decoded string.
	 * @return @c parseSuccessful on success. @c out is filled.
	 * @c parseFailed on failure. No non-whitespace characters will have
	 * been consumed.
	 */
	ParseSucceeded GetIdentifier(std::string& out) {
		out.clear();
		ChompSpace();
		if (IsEndOfLine())
			return parseFailed;
		while (!IsFinishedArgument()) {
			out.append(1, *p);
			++p;
		}
		return parseSucceeded;
	}
};

class StringReaderParameters : public Parameters {
	StringReader& reader;
public:
	StringReaderParameters(StringReader& r) : reader(r) {
	}
	~StringReaderParameters() {
	}
	int32_t GetInteger() {
		int32_t r;
		if (StringReader::parseFailed == reader.GetInteger(r))
			throw IncorrectParameterException();
		return r;
	}
	void GetString(std::string& out) {
		if (StringReader::parseFailed == reader.GetString(out))
			throw IncorrectParameterException();
	}
};


class StringWriter {
	bool startOfLine;
	std::string buffer;
public:
	StringWriter() : startOfLine(true) {
	}
	/**
	 * Returns the written string.
	 * @return The null-terminated string written to.
	 */
	const char* Result() const {
		return buffer.c_str();
	}
	/**
	 * Begins a new line, reusing the same buffer.
	 */
	void Reset() {
		startOfLine = true;
		buffer.clear();
	}
	/**
	 * Returns the number of characters that would have been written to the
	 * buffer if it has been long enough. If it was long enough, this will be
	 * the length of the string written into the buffer passed in SetBuffer.
	 */
	int32_t Length() const {
		return buffer.length();
	}
	/**
	 * Writes a single character to the buffer.
	 */
	void WriteChar(char c) {
		buffer.append(1, c);
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
	 * Writes a decimal (or octal!) digit.
	 */
	void WriteDigit(int32_t d) {
		WriteChar(static_cast<char>('0' + d));
	}
	/**
	 * Writes a string surrounded by double quotes.
	 * @param s The null-terminated string to write.
	 */
	void WriteString(const char* s) {
		BeginArgument();
		WriteChar('"');
		bool allowDigits = true;
		while (*s) {
			unsigned char c = *reinterpret_cast<const unsigned char*>(s);
			++s;
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
	 * Writes a decimal integer to the buffer. Writes negative numbers preceded
	 * with '-' and positive numbers without prefix.
	 */
	void WriteInteger(int32_t n) {
		BeginArgument();
		if (n < 0) {
			WriteChar('-');
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
			++id;
		}
	}
};

CommandFactory::~CommandFactory() {
}

Executor::~Executor() {
}

class VaListParameters : public Parameters {
	va_list& args;
	CommandLogger* logger;
	StringWriter writer;
public:
	/**
	 * Construct a Parameters facade over a variable argument list.
	 * @param a The va_list, which must have had va_start called on it.
	 * va_end should not be called after use.
	 * @param name The name of the command being constructed (i.e.
	 * the @c name parameter of @c Execute)
	 * @param commandLogger The logger that will receive a string
	 * representation of the command and its parameters. Ownership is
	 * not passed.
	 */
	VaListParameters(va_list& a, const char* name,
			CommandLogger* commandLogger)
			: args(a), logger(commandLogger) {
		writer.WriteIdentifier(name);
	}
	~VaListParameters() {
		va_end(args);
		logger = 0;
	}
	int32_t GetInteger() {
		int32_t r = va_arg(args, int32_t);
		writer.WriteInteger(r);
		return r;
	}
	void GetString(std::string& out) {
		const char* s = va_arg(args, const char*);
		writer.WriteString(s);
		out.assign(s);
	}
	/**
	 * Write the command out to the command logger.
	 */
	void Flush() {
		if (logger) {
			logger->WriteCommand(writer.Result());
		}
		writer.Reset();
	}
};

class ConcreteExecutor : public Executor {
	CommandHistory history;
	CommandLogger* logger;
	typedef std::map<std::string, CommandFactory*> FactoryMap;
	FactoryMap factories;
	// does not own its factories
	std::vector<CommandFactory*> constructiveCommands;
	CommandFactory* Factory(const char* name) {
		std::string n(name);
		FactoryMap::iterator found = factories.find(n);
		if (found == factories.end())
			throw UnknownCommandException();
		return found->second;
	}
public:
	ConcreteExecutor() : logger(0) {
	}
	~ConcreteExecutor() {
		for (FactoryMap::iterator i = factories.begin();
				i != factories.end(); ++i) {
			delete i->second;
		}
	}
	void Execute(const char* name, ...) {
		CommandFactory* f = Factory(name);
		va_list args;
		va_start(args, name);
		VaListParameters vps(args, name, logger);
		Command* c = f->Create(vps);
		vps.Flush();
		history.Do(*c, logger);
	}
	bool ExecuteFromLog(const char* line) {
		StringReader reader;
		reader.SetBuffer(line);
		std::string id;
		if (StringReader::parseSucceeded == reader.GetIdentifier(id)) {
			CommandFactory* f = Factory(id.c_str());
			StringReaderParameters sps(reader);
			Command* c = f->Create(sps);
			history.Do(*c, logger);
			return true;
		}
		if (StringReader::parseSucceeded == reader.IsEndOfLine())
			return false;
		throw MalformedLineException();
	}
	void ExecuteRandomCommands(RandomSource& rng) {
		int n = factories.size();
		if (n == 0)
			throw UnknownCommandException();
		std::vector<CommandFactory*> factoriesByIndex;
		for (FactoryMap::iterator i = factories.begin();
				i != factories.end(); ++i) {
			factoriesByIndex.push_back(i->second);
		}
		while (true) {
			int r = rng.GetUniform(n);
			if (r == n)
				return;
			Command* c = factoriesByIndex[r]->CreateRandom(rng);
			history.Do(*c, logger);
		}
	}
	virtual void ExecuteRandomConstructiveCommands(RandomSource& rng) {
		int n = constructiveCommands.size();
		if (n == 0)
			throw UnknownCommandException();
		while (true) {
			int r = rng.GetUniform(n);
			if (r == n)
				return;
			Command* c = constructiveCommands[r]->CreateRandom(rng);
			history.Do(*c, logger);
		}
	}
	void SetCommandLogger(CommandLogger* log) {
		logger = log;
	}
	void AddCommand(const char* name,
			std::auto_ptr<CommandFactory> factory, bool constructive) {
		std::string n(name);
		std::pair<std::string, CommandFactory*> p(n, factory.get());
		if (constructive)
			constructiveCommands.reserve(constructiveCommands.size() + 1);
		factories.insert(p);
		if (constructive)
			constructiveCommands.push_back(factory.get());
		factory.release();
	}
	void ClearHistory() {
		history.Clear();
	}
	bool Undo() {
		if (!history.CanUndo())
			return false;
		history.Undo();
		return true;
	}
	bool Redo() {
		if (!history.CanRedo())
			return false;
		history.Redo();
		return true;
	}
};

Executor* MakeExecutor() {
	return new ConcreteExecutor();
}

Parameters::~Parameters() {
}
