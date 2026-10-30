/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef IMPORTTAB_H
#define IMPORTTAB_H

#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QWidget>
#include <QTextEdit>

#include <vector>

using namespace std;


/**
 * The import tab in the preferences menu.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ImportTab : public QWidget
{
	Q_OBJECT
public:
	/**
	 * Sets up the import tab.
	 * @param parent the parent of the widget.
	 */
	ImportTab( QWidget *parent = nullptr );

	/**
	 * Sets up the gui for the tab and connects the buttons.
	 */
	void makeGUI();

	/**
	 * Initializes the table and fills it with starting values.
	 */
	void initializeImportValues();

	/**
	 * Applies the settings in the import tab.
	 */
	void apply();

	void retranslateStrings();

protected:
	void resizeEvent(QResizeEvent *event) override;

private slots:
	void addImportProgram();
	void removeImportProgram();
	void contentsChanged(int row, int column);
	void activeCellChanged(int row, int column);
	void changeSettings();
	void closeChangeBox();
	void updatePrePollString(const QString &txt);
	void updateStartDaemonString(const QString &txt);
	void updatestopDaemonString(const QString &txt);

private:
	vector<QString>prePollStrings;
	vector<QString>startDaemonStrings;
	vector<QString>stopDaemonStrings;

	QTableWidget *deviceSelectionTable;
	QPushButton *addButton;
	QPushButton *removeButton;
	QPushButton *changeButton;
	QPushButton *closeChangeBoxButton;
	QLineEdit *prePollEdit;
	QLineEdit *startDaemonEdit;
	QLineEdit *stopDaemonEdit;
	QGroupBox *grabberPreferences;
	QLabel *prePollLabel;
	QLabel *startDaemonLabel;
	QLabel *stopDaemonLabel;
	QTableWidgetItem *checkTableItem;
	QTextEdit *informationText;
};

#endif
