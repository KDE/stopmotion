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

#ifndef TREPLAY_H_
#define TREPLAY_H_

#include <list>
#include <string>

class Executor;
class CloneLogger;

class TestCommandFactory : public QObject
{
	Q_OBJECT
	Executor* ce;
	CloneLogger* cl;
	char* str;	// output from RandomString
	char* strNext;
	int32_t strAllocLen;
public:
	typedef std::list<std::string> output_t;
private:
	output_t executionOutput;
	void AddCharToRandomString(char);
public:
	TestCommandFactory();
	~TestCommandFactory();
	const char* RandomString();
private slots:
	void emptyCommandReplayerThrows();
	void canParseFromLog();
	void parsingDescriptionIsCloning();
	void replaySequenceProducesSameOutput();
	void undoPutsModelBack();
	void replayIsRobust();
};

#endif /* TREPLAY_H_ */
