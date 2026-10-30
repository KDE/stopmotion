/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PREFERENCESMENU_H
#define PREFERENCESMENU_H

#include <QDialog>
#include <QTabWidget>
#include <QPushButton>

class ImportTab;
class ExportTab;
class DeviceTab;

/**
 * The import and export preferences menu.
 * 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class PreferencesMenu : public QDialog 
{
	Q_OBJECT
public:
	/**
	 * Sets up the filepreview-label.
	 * @param parent the parent of the widget.
	 */
	PreferencesMenu(QWidget *parent = nullptr);
	void display();
	void retranslateStrings();
	
private:
	ImportTab *importVideoTab;
	ExportTab *exportVideoTab;
	DeviceTab *videoDeviceTab;
	QTabWidget *tabWidget;
	QPushButton *applyButton;
	QPushButton *closeButton;

	void makeVideoImportTab();
	void makeVideoExportTab();
	void makeVideoDeviceTab();
	
private slots:
	void apply();
};

#endif
