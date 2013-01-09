/*
 * treplay.h
 *
 *  Created on: 23 Nov 2012
 *      Author: tim
 */

#ifndef TREPLAY_H_
#define TREPLAY_H_

class CommandReplayer;

class TestCommandFactory : public QObject
{
	Q_OBJECT
	CommandReplayer* cr;
	char* str;	// output from RandomString
	char* strNext;
	int32_t strAllocLen;
	void AddCharToRandomString(char);
public:
	TestCommandFactory();
	~TestCommandFactory();
	const char* RandomString();
private slots:
	void emptyCommandReplayerThrows();
	void allMakeCallsParse();
	void parsingDescriptionIsCloning();
	void replaySequenceProducesSameOutput();
	void undoPutsModelBack();
};

#endif /* TREPLAY_H_ */
