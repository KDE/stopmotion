/***************************************************************************
 *   Copyright (C) 2014 by Linuxstopmotion contributors;                   *
 *   see the AUTHORS file for details.                                     *
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

#include "filenamesfromurlsiterator.h"

#include <QString>
#include <QUrl>

void FileNamesFromUrlsIterator::set() {
	if (!atEnd()) {
		QString file = b->toLocalFile();
		buffer.assign(file.toLocal8Bit());
		buffer.c_str();
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
	return &buffer[0];
}

void FileNamesFromUrlsIterator::next() {
	++b;
	set();
}
