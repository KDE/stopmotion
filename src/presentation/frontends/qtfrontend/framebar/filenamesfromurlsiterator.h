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
