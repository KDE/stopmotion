/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TEXECUTOR_H_
#define TEXECUTOR_H_

#include <cstdint>
#include <list>
#include <string>
#include <QObject>

class Executor;
class CloneLogger;

class TestCommandFactory : public QObject {
	Q_OBJECT
	Executor* ce;
	CloneLogger* cl;
	char* str;	// output from RandomString
	char* strNext;
	int32_t strAllocLen;
public:
	using output_t = std::list<std::string>;
private:
	output_t executionOutput;
	void AddCharToRandomString(char);
public:
	TestCommandFactory();
	~TestCommandFactory() override;
	const char* RandomString();
private slots:
	void emptyCommandReplayerThrows();
	void canParseFromLog();
	void parsingDescriptionIsCloning();
	void testUndo();
	void replayIsRobust();
};

#endif /* TEXECUTOR_H_ */
