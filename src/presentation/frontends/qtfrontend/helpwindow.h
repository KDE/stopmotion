/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QString>

#include "ui_helpbrowser.h"


class HelpWindow : public QMainWindow
{
	Q_OBJECT
public:
	HelpWindow(QWidget *parent = nullptr);

private:
	Ui::MainWindow ui;
};

#endif
