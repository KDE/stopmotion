/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "filenamesfromurlsiterator.h"

#include <QString>
#include <QUrl>

void FileNamesFromUrlsIterator::set() {
	if (!atEnd()) {
		QString file = b->toLocalFile();
		buffer.assign(file.toLocal8Bit());
	}
}

FileNamesFromUrlsIterator::FileNamesFromUrlsIterator(
		QList<QUrl>::Iterator begin, QList<QUrl>::Iterator end)
		: b(begin), e(end) {
	set();
}

FileNamesFromUrlsIterator::~FileNamesFromUrlsIterator() {
}

int FileNamesFromUrlsIterator::count() {
	int c = 0;
	for (QList<QUrl>::Iterator i(b); i != e; ++i) {
		++c;
	}
	return c;
}

bool FileNamesFromUrlsIterator::atEnd() const {
	return b == e;
}

const char* FileNamesFromUrlsIterator::get() const {
	return buffer.c_str();
}

void FileNamesFromUrlsIterator::next() {
	++b;
	set();
}
