/*
 * treplay.h
 *
 *  Created on: 23 Nov 2012
 *      Author: tim
 */

#ifndef TREPLAY_H_
#define TREPLAY_H_

class TestCommandFactory : public QObject
{
	Q_OBJECT
private slots:
	void emptyCommandReplayerThrows();
};

#endif /* TREPLAY_H_ */
