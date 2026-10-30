/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DEVICETAB_H
#define DEVICETAB_H

#include <qobjectdefs.h>
#include <qstring.h>
#include <qwidget.h>
#include <vector>

class QTableWidget;
class QLineEdit;
class QPushButton;
class QLabel;
class QGroupBox;
class QObject;
class QResizeEvent;
class QTextEdit;
class QTableWidgetItem;

class DeviceTab : public QWidget
{
	Q_OBJECT
public:
	DeviceTab(QWidget *parent = nullptr);

	void initialize();
	void apply();
	void retranslateStrings();

protected:
	void resizeEvent(QResizeEvent *event) override;

private slots:
	void contentsChanged(int row, int column);
	void activeCellChanged(int row, int column);
	void editDevice();
	void closeChangeBox();
	void updateDeviceString(const QString &txt);
	void addDevice();
	void removeDevice();

private:
	QTableWidget *deviceTable;
	QPushButton *addButton;
	QPushButton *removeButton;
	QPushButton *editButton;
	QPushButton *closeChangeBoxButton;
	QLineEdit *deviceEdit;
	QGroupBox *devicePreferences;
	QLabel *deviceLabel;
	QTableWidgetItem *checkTableItem;
	QTextEdit *informationText;
	std::vector<QString> deviceStrings;
	std::vector<QString> deviceNameStrings;
	std::vector<QString> deviceDescriptionStrings;
	int numAutoDetectedDevices;

	void makeGUI();
};

#endif

