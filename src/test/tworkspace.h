/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TWORKSPACE_H_
#define TWORKSPACE_H_

#include <QObject>

class RealOggEmptyJpg;
class TestHome;

class TestWorkspace : public QObject {
	Q_OBJECT

	TestHome* testEnvFs;
	RealOggEmptyJpg* mfs;
public:
	TestWorkspace();
	~TestWorkspace() override;
private slots:
	void jpgsCanBeCopiedIntoWorkspace();
	void nonJpgImagesCannotBeCopied();
};

#endif
