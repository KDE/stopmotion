/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cstdio>
#include <QCoreApplication>
#include <QTest>

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
