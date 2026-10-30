/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef EXTERNALCOMMAND_H
#define EXTERNALCOMMAND_H

#include <QAction>
#include <QButtonGroup>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QProcess>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>


class ExternalCommand : public QWidget
{
	Q_OBJECT
public:
	ExternalCommand(QWidget *parent = nullptr);
	void run(const QString &command);

private slots:
	void readFromStandardOutput();
	void readFromStandardError();
	void submitInputToProgram();
	void displayExitStatus(int exitCode, QProcess::ExitStatus exitStatus);

private:
	QVBoxLayout *vboxLayout;
	QTextBrowser *textBrowser;
	QHBoxLayout *hboxLayout;
	QLabel *label;
	QLineEdit *lineEdit;
	QPushButton *submitButton;
	QPushButton *closeButton;
	QProcess *process;
};

#endif

