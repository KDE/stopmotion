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

#include <stdarg.h>

#include "executor.h"
#include "command.h"
#include "random.h"
#include "commandlogger.h"
#include "src/foundation/stringwriter.h"
#include "src/domain/animation/errorhandler.h"

#include <map>
#include <vector>
#include <string>
#include <memory>

#include <stdint.h>
#include <string.h>

// help out Eclipse's C++ parsing
#ifndef INT32_MAX
#define INT32_MAX 0x7FFFFFFF
#endif

class UndoRedoObserver;

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
	const char* getPos() const {
		return p;
	}
	void setPos(const char* pos) {
		p = pos;
	}
	/**
	 * Sets the buffer to be read.
	 * @param input The start of the buffer to read.
	 * @param inputEnd The end of the buffer to read.
	 */
	void setBuffer(const char* input, const char* endInput) {
		p = input;
		end = endInput;
	}
	/**
	 * Sets the buffer to be read.
	 * @param input The null-terminated string to read.
	 */
	void setBuffer(const char* input) {
		p = input;
		end = input + strlen(input);
	}
	/**
	 * Gets the next character from the input buffer, as long as it isn't an
	 * end-of-line character.
	 * @param out Stores the character output, only if @c isNotEol is returned.
	 * @return @c isNotEol on success; in this case @a out will be set.
	 * @c isEol if we are at the end of the buffer or the next character is a
	 * line-termination character, in which case @a out will not be set.
	 */
	IsEol getCharFromLine(char& out) {
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
	 * @return @c isEof if we have reached the end of the buffer, whether or
	 * not we consumed an end-of-line marker. @c isNotEof otherwise.
	 */
	IsEof chompEol() {
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
	 * @return @c isEof if we reached the end of the buffer, whether or not we
	 * consumed any space. @c isNotEof otherwise.
	 */
	IsEof chompSpace() {
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
	bool getQuote() {
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
	bool isEndOfLine() {
		return p == end || *p == '\n' || *p == '\r';
	}
	/**
	 * Tests whether we are at an argument-delimiting character.
	 * @returns true if we are at a whitespace, the end of the line or the end
	 * of the buffer. false otherwise.
	 */
	bool isFinishedArgument() {
		return *p == ' ' || *p == '!' || *p == '.' || isEndOfLine();
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
	ParseSucceeded getEndOfCommand(int32_t& dotCount, bool& exclamation) {
		const char* old = p;
		dotCount = 0;
		exclamation = false;
		char c;
		while (isEol != getCharFromLine(c)) {
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
	 * @param digit The digit output, if @c parseSuceeded is returned.
	 * @return @c parseSucceeded on success; in this case @a digit will be set.
	 * @c parseFailed if the next character is not a digit. In this case
	 * @a digit will not be set and no characters will have been consumed.
	 */
	ParseSucceeded getDigit(int32_t& digit) {
		if (p == end)
			return parseFailed;
		if (*p < '0' || '9' < *p)
			return parseFailed;
		digit = *p - '0';
		++p;
		return parseSucceeded;
	}
	/**
	 * As for @a GetDigit but '8' and '9' are not considered digits and are
	 * not consumed.
	 */
	ParseSucceeded getOctalDigit(int32_t& digit) {
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
	 * @param @a out The parsed number; only if @c parseSucceeded is returned.
	 * @return @c parseSucceeded if the next characters were possibly a '-',
	 * a string of numbers then an argument delimiter. All except the
	 * delimiter are consumed. @a out is set to the parsed number. In other
	 * cases, no characters are consumed, @a out is not set and @c parseFailed
	 * is returned.
	 */
	ParseSucceeded getInteger(int32_t& out) {
		const char *old = p;
		if (isEof == chompSpace())
			return parseFailed;
		int32_t n = 0;
		int32_t sign = 1;
		if (getDigit(n) == parseFailed) {
			if (*p == '-')
				sign = -1;
			else
				return parseFailed;
			++p;
		}
		int32_t soFar = n;
		while (getDigit(n) == parseSucceeded) {
			soFar = soFar * 10 + n;
		}
		if (isFinishedArgument()) {
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
	 * @return @c parseSuccessful on success. @a out is filled.
	 * @c parseFailed on failure. Nothing is consumed. Some of @a out may
	 * have been set.
	 */
	ParseSucceeded getString(std::string& out) {
		out.clear();
		const char *old = p;
		chompSpace();
		if (!getQuote()) {
			p = old;
			return parseFailed;
		}
		char c;
		while (!getQuote()) {
			// normal state
			if (getCharFromLine(c) == isEol) {
				p = old;
				return parseFailed;
			}
			if (c != '\\') {
				out.append(1, c);
			} else {
				// backslash state
				if (getCharFromLine(c) == isEol)
					// line continuation
					chompEol();
				if (c == 'n')
					out.append(1, '\n');
				else if (c == 'r')
					out.append(1, '\r');
				else if ('0' <= c && c <= '7') {
					// octal
					int32_t soFar = c - '0';
					int32_t n;
					while (parseSucceeded == getOctalDigit(n)) {
						soFar = soFar * 8 + n;
					}
					out += static_cast<char>(soFar);
				} else {
					// Backslash followed by anything else is literally that
					// anything else (the backslash is never output).
					out += static_cast<char>(c);
				}
			}
		}
		return parseSucceeded;
	}
	/**
	 * Reads an identifier.
	 * @param out Returns the parsed and decoded string.
	 * @return @c parseSuccessful on success. @a out is filled.
	 * @c parseFailed on failure. No non-whitespace characters will have
	 * been consumed.
	 */
	ParseSucceeded getIdentifier(std::string& out) {
		out.clear();
		chompSpace();
		if (isEndOfLine())
			return parseFailed;
		while (!isFinishedArgument()) {
			out.append(1, *p);
			++p;
		}
		return parseSucceeded;
	}
	int getUndoCount() {
		int count = 0;
		for (; p != end; ++p) {
			switch (*p) {
			case '?':
				++count;
				break;
			case '!':
				--count;
				break;
			default:
				return count;
			}
		}
		return count;
	}
};

class StringReaderParameters : public Parameters {
	StringReader& reader;
public:
	StringReaderParameters(StringReader& r) : reader(r) {
	}
	~StringReaderParameters() {
	}
	int32_t getInteger(int32_t, int32_t) {
		int32_t r;
		if (StringReader::parseFailed == reader.getInteger(r))
			throw IncorrectParameterException();
		return r;
	}
	void getString(std::string& out, const char*) {
		if (StringReader::parseFailed == reader.getString(out))
			throw IncorrectParameterException();
	}
};


Executor::~Executor() {
}

/**
 * Write to the log as the parameters are read from some other Parameters.
 * Also checks that the values produced are within the ranges specified,
 * throwing a {@ref ParametersOutOfRangeException} if a value is out-of-range.
 */
class WriterParametersWrapper : public Parameters {
	Parameters& delegate;
	StringWriter writer;
public:
	/**
	 * Construct a Parameters wrapper around another Parameters. Provides the
	 * same parameters as this delegate, but writes out the command as it goes.
	 * @param p The delegate.
	 * @param name The name of the command (to write to the logger).
	 */
	WriterParametersWrapper(Parameters& p, const char* name)
			: delegate(p) {
		writer.writeIdentifier(name);
	}
	~WriterParametersWrapper() {
	}
	int32_t getInteger(int32_t min, int32_t max) {
		int32_t r = delegate.getInteger(min, max);
		if(r < min || max < r)
			throw ParametersOutOfRangeException();
		writer.writeInteger(r);
		return r;
	}
	int32_t getHowMany() {
		int32_t r = delegate.getHowMany();
		if (r < 0)
			throw ParametersOutOfRangeException();
		writer.writeInteger(r);
		return r;
	}
	void getString(std::string& out, const char* pattern) {
		delegate.getString(out, pattern);
		writer.writeString(out.c_str());
	}
	void writeCommand(CommandLogger* logger) {
		if (logger)
			logger->writePendingCommand(writer.result());
		writer.reset();
	}
};

class VaListParameters : public Parameters {
	va_list& args;
public:
	/**
	 * Construct a Parameters facade over a variable argument list.
	 * @param a The va_list, which must have had va_start called on it.
	 * va_end should not be called after use.
	 * @param name The name of the command being constructed (i.e.
	 * the @a name parameter of {@ref CommandExecutor::execute})
	 * @param commandLogger The logger that will receive a string
	 * representation of the command and its parameters. Ownership is
	 * not passed.
	 */
	VaListParameters(va_list& a)
			: args(a) {
	}
	~VaListParameters() {
	}
	int32_t getInteger(int32_t, int32_t) {
		return va_arg(args, int32_t);
	}
	void getString(std::string& out, const char*) {
		const char* s = va_arg(args, const char*);
		out.assign(s);
	}
};

class RandomParameters : public Parameters {
	RandomSource& rs;
public:
	RandomParameters(RandomSource& rng)
			: rs(rng) {
	}
	int32_t getInteger(int32_t min, int32_t max) {
		return rs.getUniform(min, max);
	}
	int32_t getHowMany() {
		return 1 + rs.getLogInt(60);
	}
	void getString(std::string& out, const char* pattern) {
		if (!pattern)
			pattern = "?*";
		out.clear();
		for (; *pattern; ++pattern) {
			char c = *pattern;
			if (c == '?') {
				out.append(1, rs.getCharacter());
			} else if (c == '*') {
				rs.appendAlphanumeric(out);
			} else {
				out.append(1, c);
			}
		}
	}
};

class ConcreteExecutor : public Executor {
	CommandHistory history;
	CommandLogger* logger;
	typedef std::map<std::string, CommandFactory*> FactoryMap;
	FactoryMap factories;
	// does not own its factories
	std::vector<std::string> constructiveCommands;
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
	void execute(const char* name, ...) {
		CommandFactory* f = Factory(name);
		va_list args;
		va_start(args, name);
		VaListParameters vpsd(args);
		try {
			WriterParametersWrapper vps(vpsd, name);
			Command* c = f->create(vps, *ErrorHandler::getThrower());
			if (c) {
				vps.writeCommand(logger);
				history.execute(*c);
				logger->commit();
			}
		} catch(...) {
			// Unfortunately we can't put this in VaListParameters's destructor
			// because va_end must be called in the same function as va_start,
			// according to the standard.
			va_end(args);
			throw;
		}
		va_end(args);
	}
	void execute(const char* name, Parameters& params) {
		WriterParametersWrapper pw(params, name);
		CommandFactory* f = Factory(name);
		Command* c = f->create(pw, *ErrorHandler::getThrower());
		if (c) {
			pw.writeCommand(logger);
			history.execute(*c);
			logger->commit();
		}
	}
	bool executeFromLog(const char* line, ErrorHandler& e) {
		StringReader reader;
		reader.setBuffer(line);
		std::string id;
		int undoCount = reader.getUndoCount();
		for (; undoCount < 0; ++undoCount)
			history.redo();
		for (; undoCount != 0; --undoCount)
			history.undo();
		if (StringReader::parseSucceeded == reader.getIdentifier(id)) {
			const char* commandName = id.c_str();
			CommandFactory* f = Factory(commandName);
			StringReaderParameters sps(reader);
			Command* c = f->create(sps, e);
			// It is an error if a command executed from a log is invalid.
			// This would mean that the log has become out-of-sync with the
			// model.
			if (!c)
				throw ParametersOutOfRangeException();
			int32_t subcommands;
			bool finished;
			if (StringReader::parseFailed
					== reader.getEndOfCommand(subcommands, finished))
				throw MalformedLineException();
			if (finished)
				history.execute(*c);
			return true;
		}
		if (StringReader::parseSucceeded == reader.isEndOfLine())
			return false;
		throw MalformedLineException();
	}
	void executeRandomCommands(int& commandCount, RandomSource& rng,
			int minCount, int maxCount) {
		commandCount = 0;
		int n = factories.size();
		if (n == 0)
			throw UnknownCommandException();
		std::vector<std::string> factoryNames;
		for (FactoryMap::iterator i = factories.begin();
				i != factories.end(); ++i) {
			factoryNames.push_back(i->first);
		}
		bool dontEnd = false;
		while (commandCount < maxCount) {
			int r = rng.getUniform(dontEnd || commandCount < minCount?
					n - 1: n);
			if (r == n)
				return;
			std::string& name(factoryNames[r]);
			RandomParameters rpsd(rng);
			const char* commandName = name.c_str();
			WriterParametersWrapper rps(rpsd, commandName);
			Command* c = factories[name]->create(rps, *ErrorHandler::getThrower());
			if (c) {
				if (logger)
					rps.writeCommand(logger);
				history.execute(*c);
				if (logger)
					logger->commit();
				++commandCount;
			}
		}
	}
	virtual void executeRandomConstructiveCommands(RandomSource& rng) {
		int n = constructiveCommands.size();
		if (n == 0)
			throw UnknownCommandException();
		while (true) {
			int r = rng.getUniform(n);
			if (r == n)
				return;
			std::string& name(constructiveCommands[r]);
			RandomParameters rpsd(rng);
			WriterParametersWrapper rps(rpsd, name.c_str());
			Command* c = factories[name]->create(rps, *ErrorHandler::getThrower());
			if (c) {
				if (logger)
					rps.writeCommand(logger);
				history.execute(*c);
				if (logger)
					logger->commit();
			}
		}
	}
	void setCommandLogger(CommandLogger* log) {
		logger = log;
	}
	void addCommand(const char* name,
			std::unique_ptr<CommandFactory> factory, bool constructive) {
		std::string n(name);
		std::pair<std::string, CommandFactory*> p(n, factory.get());
		if (constructive)
			constructiveCommands.reserve(constructiveCommands.size() + 1);
		factories.insert(p);
		if (constructive)
			constructiveCommands.push_back(n);
		factory.release();
	}
	int commandCount() const {
		return factories.size();
	}
	void clearHistory() {
		history.clear();
	}
	bool undo() {
		if (!history.canUndo())
			return false;
		if (logger)
			logger->writePendingUndo();
		history.undo();
		if (logger)
			logger->commit();
		return true;
	}
	bool redo() {
		if (!history.canRedo())
			return false;
		if (logger)
			logger->writePendingRedo();
		history.redo();
		if (logger)
			logger->commit();
		return true;
	}
	bool canUndo() const {
		return history.canUndo();
	}
	bool canRedo() const {
		return history.canRedo();
	}
	void setUndoRedoObserver(UndoRedoObserver* observer) {
		history.setUndoRedoObserver(observer);
	}
};

Executor* makeExecutor() {
	return new ConcreteExecutor();
}

const char* IncorrectParameterException::what() const throw() {
	return "Command log corrupt (incorrect parameter type)!";
}

const char* MalformedLineException::what() const throw() {
	return "Command log corrupt (malformed line)!";
}

const char* ParametersOutOfRangeException::what() const throw() {
	return "Internal error or command log corrupt:\n"
			"Command parameters out of range.";
}

const char* UnknownCommandException::what() const throw() {
	return "Internal error or command log corrupt:\n"
			"Unknown command.";
}

const char* CommandNameAlreadyUsedException::what() const
		throw() {
	return "Internal error: Command registered twice!";
}
