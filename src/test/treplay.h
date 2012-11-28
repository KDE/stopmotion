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
public:
	TestCommandFactory();
	~TestCommandFactory();
private slots:
	void emptyCommandReplayerThrows();
	void allMakeCallsParse();
};

#endif /* TREPLAY_H_ */
