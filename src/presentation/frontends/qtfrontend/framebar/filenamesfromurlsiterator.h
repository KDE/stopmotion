/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILENAMESFROMURLSITERATOR_H_
#define FILENAMESFROMURLSITERATOR_H_

#include "src/technical/stringiterator.h"

#include <string>
#include <QList>

class QUrl;

class FileNamesFromUrlsIterator : public StringIterator {
	QList<QUrl>::Iterator b;
	QList<QUrl>::Iterator e;
	std::string buffer;
	void set();
public:
	FileNamesFromUrlsIterator(QList<QUrl>::Iterator begin,
			QList<QUrl>::Iterator end);
	~FileNamesFromUrlsIterator() override;
	int count() override;
	bool atEnd() const override;
	const char* get() const override;
	void next() override;
};

#endif
