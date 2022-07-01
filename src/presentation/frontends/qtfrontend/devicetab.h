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
	DeviceTab(QWidget *parent = 0);

	void initialize();
	void apply();
	void retranslateStrings();

protected:
	void resizeEvent(QResizeEvent *event);

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

