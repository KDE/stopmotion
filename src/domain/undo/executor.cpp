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
#include "commandfactory.h"
#include "command.h"

#include <stdarg.h>
#include <map>
#include <string>
#include <memory>

Executor::~Executor() {
}

class VaListParameters : public Parameters {
	va_list& args;
	CommandLogger* logger;
public:
	VaListParameters(va_list& a, const char* name,
			CommandLogger* commandLogger)
			: args(a), logger(commandLogger) {
		//TODO write name to the log
	}
	~VaListParameters() {
		va_end(args);
	}
	int32_t GetInteger() {
		int32_t r = va_arg(args, int32_t);
		//TODO write r to the log
		return r;
	}
	int32_t GetString(char* out, int32_t maxLength) {
		const char* s = va_arg(args, const char*);
		int len = strncpy(out, s, maxLength);
		//TODO write 'out' to the log
		return len;
	}
	void Flush() {
		//TODO flush logged line to the log
	}
};

class ConcreteExecutor : public Executor {
	CommandHistory history;
	typedef std::map<std::string, CommandFactory*> FactoryMap;
	FactoryMap factories;
public:
	ConcreteExecutor() {
	}
	~ConcreteExecutor() {
		for (FactoryMap::iterator i = factories.begin();
				i != factories.end(); ++i) {
			delete i->second;
		}
	}
	void Execute(const char* name, ...) {
		std::string n(name);
		CommandFactory* f = factories.find(n);
		if (f == factories.end())
			throw UnknownCommandException();
		va_list args;
		va_start(args, name);
		VaListParameters vps(args);
		Command* c = f->Create(vps);
		history.Do(*c);
	}
	void AddCommand(const char* name, CommandFactory* factory) {
		std::auto_ptr<CommandFactory> f = factory;
		std::string n(name);
		if (factories.find(n))
			throw UnknownCommandException();
		std::pair<std::string, CommandFactory*> p(n, factory);
		factories.insert(p);
		f.release();
	}
};
