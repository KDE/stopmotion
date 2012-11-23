#include <QtTest/QtTest>

#include "treplay.h"

int max(int a, int b) {
	return a < b? b : a;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TestCommandFactory t1;
    int overall = QTest::qExec(&t1, argc, argv);
    return overall;
}
