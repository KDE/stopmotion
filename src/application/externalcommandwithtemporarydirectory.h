/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef EXTERNALCOMMANDWITHTEMPORARYDIRECTORY_H_
#define EXTERNALCOMMANDWITHTEMPORARYDIRECTORY_H_

#include <QMetaObject>

class ExternalCommand;
class TemporaryDirectory;
class QWidget;
class QString;

/** Deletes itself when its window is closed. */
class ExternalCommandWithTemporaryDirectory {
	ExternalCommand* ec;
	TemporaryDirectory* td;
	QMetaObject::Connection deleteWithWindow;
	ExternalCommandWithTemporaryDirectory&
	operator=(const ExternalCommandWithTemporaryDirectory&) = delete;
	ExternalCommandWithTemporaryDirectory(
			const ExternalCommandWithTemporaryDirectory&) = delete;
public:
	explicit ExternalCommandWithTemporaryDirectory(QWidget *parent = nullptr);
	~ExternalCommandWithTemporaryDirectory();
	void run(const QString &command);
	void show();
	const char* getTemporaryDirectoryPath() const;
};

#endif
