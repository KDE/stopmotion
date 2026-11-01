/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "helpwindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QTextBrowser>
#include <QUrl>

#include "src/config.h"

class QWidget;

HelpWindow::HelpWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	QObject::connect(ui.textBrowser, &QTextBrowser::highlighted, ui.lineEdit, [&](const QUrl &url) { ui.lineEdit->setText(url.path()); });
	// In a build directory, the executable is in bin/ and the manual next to it
	QString index = QCoreApplication::applicationDirPath()
			+ QStringLiteral(u"/../manual/index.html");
	if (!QFile::exists(index))
		index = QString(stopmotion::htmlDirectory) + "/index.html";
	ui.textBrowser->setSource(QUrl::fromLocalFile(index));
}
