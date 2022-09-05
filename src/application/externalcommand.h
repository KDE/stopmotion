/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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
	ExternalCommand(QWidget *parent = 0);
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

