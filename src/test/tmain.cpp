/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#include <stdio.h>
#include <QCoreApplication>
#include <QtTest/QtTest>

#include "texecutor.h"
#include "tcache.h"
#include "tstopmotionundo.h"
#include "tworkspace.h"

int main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);
	int overall = 0;
	TestCommandFactory t1;
	overall = QTest::qExec(&t1, argc, argv);
	TestCache t2;
	overall += QTest::qExec(&t2, argc, argv);
	TestStopmotionUndo t3;
	overall += QTest::qExec(&t3, argc, argv);
	TestWorkspace t4;
	overall += QTest::qExec(&t4, argc, argv);
	if (overall == 0)
		printf("Success!\n");
	else
		printf("Failed: %d\n", overall);
	return overall;
}
