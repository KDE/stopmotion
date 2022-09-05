/***************************************************************************
 *   Copyright (C) 2005-2017 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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
#include "src/presentation/frontends/qtfrontend/exporttab.h"

#include <stdlib.h>
#include <algorithm>
#include <memory>

#include "graphics/icons/close.xpm"
#include "flexiblelineedit.h"
#include "src/foundation/preferencestool.h"
#include "src/foundation/logger.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolBox>


ExportTab::ExportTab(QWidget *parent) : QWidget(parent)
{
	addButton          = 0;
	removeButton       = 0;
	editButton         = 0;
	browseButton       = 0;
	yesButton          = 0;
	noButton           = 0;
	closeButton        = 0;
	encoderPrefs       = 0;
	encoderTable       = 0;
	startEncoder       = 0;
	stopEncoder        = 0;
	startEncoderLabel  = 0;
	stopEncoderLabel   = 0;
	defaultOutput      = 0;
	defaultOutputLabel = 0;
	askForOutput       = 0;
	infoText           = 0;

	makeGUI();
}


void ExportTab::makeGUI()
{
	infoText = new QTextEdit;
	infoText->setReadOnly(true);
	infoText->setMinimumWidth(440);
	infoText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	encoderTable = new QTableWidget;
	encoderTable->setColumnCount(2);
	encoderTable->setRowCount(0);
	encoderTable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
	encoderTable->setSelectionMode(QAbstractItemView::SingleSelection);
	encoderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	encoderTable->verticalHeader()->setVisible(false);

	connect(encoderTable, SIGNAL(cellClicked(int, int)), this, SLOT(activeCellChanged(int, int)));
	connect(encoderTable, SIGNAL(cellChanged(int, int)), this, SLOT(contentsChanged(int, int)));

	addButton = new QPushButton(tr("&Add"));
	addButton->setFocusPolicy( Qt::NoFocus );
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEncoder()) );

	removeButton = new QPushButton(tr("&Remove"));
	connect( removeButton, SIGNAL(clicked()), this, SLOT(removeEncoder()) );

	editButton = new QPushButton(tr("&Edit"));
	QObject::connect( editButton, SIGNAL(clicked()), this, SLOT(editSettings()) );

	encoderPrefs = new QGroupBox;
	encoderPrefs->setTitle( tr("Encoder settings") );
	encoderPrefs->hide();

	closeButton = new QPushButton;
	closeButton->setIcon(QPixmap(closeicon));
	closeButton->setFlat(true);
	closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
	connect( closeButton, SIGNAL(clicked()),this, SLOT(closeSettings()) );

	askForOutput = new QLabel(
			tr("Do you want to be asked for an output file every time you choose to export?"));

	yesButton = new QRadioButton(tr("Yes"));
	yesButton->setChecked(true);
	yesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect( yesButton, SIGNAL(clicked()), this, SLOT(setYesButtonOn()) );

	noButton = new QRadioButton(tr("No"));
	noButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	noButton->setChecked(false);
	connect( noButton, SIGNAL(clicked()), this, SLOT(setNoButtonOn()) );

	defaultOutputLabel = new QLabel( tr("Set default output file:"));
	defaultOutput = new FlexibleLineEdit;
	defaultOutput->setEnabled(false);
	connect( defaultOutput, SIGNAL(textChanged(const QString &)),
			this, SLOT(setDefaultOutput(const QString &)) );

	browseButton = new QPushButton(tr("Browse"));
	browseButton->setEnabled(false);
	browseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect( browseButton, SIGNAL(clicked()), this, SLOT(browseFiles()) );

	startEncoderLabel = new QLabel( tr("Start encoder:") );
	startEncoder = new FlexibleLineEdit;
	connect( startEncoder, SIGNAL(textChanged(const QString &)),
			this, SLOT(updateStartString(const QString &)) );

	stopEncoderLabel = new QLabel( tr("Stop encoder:") );
	stopEncoder = new FlexibleLineEdit;
	connect( stopEncoder, SIGNAL(textChanged(const QString &)),
			this, SLOT(updateStopString(const QString &)) );

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(infoText);
	QVBoxLayout *buttonLayout = new QVBoxLayout;
	buttonLayout->setMargin(0);
	buttonLayout->setSpacing(2);
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(addButton);
	buttonLayout->addWidget(removeButton);
	buttonLayout->addWidget(editButton);
	QHBoxLayout *encoderLayout = new QHBoxLayout;
	encoderLayout->addWidget(encoderTable);
	encoderLayout->addLayout(buttonLayout);
	mainLayout->addLayout(encoderLayout);
	mainLayout->addWidget(encoderPrefs);
	setLayout(mainLayout);

	QGridLayout *encoderPrefsLayout = new QGridLayout;
	encoderPrefsLayout->addWidget(closeButton, 0, 2);
	encoderPrefsLayout->addWidget(askForOutput, 0, 0, 1, 2);
	QHBoxLayout* hbLayout = new QHBoxLayout;
	hbLayout->addWidget(yesButton);
	hbLayout->addWidget(noButton);
	hbLayout->addStretch(1);
	encoderPrefsLayout->addLayout(hbLayout, 1, 1);
	encoderPrefsLayout->addWidget(defaultOutputLabel, 2, 0);
	hbLayout = new QHBoxLayout;
	hbLayout->addWidget(defaultOutput);
	hbLayout->addWidget(browseButton);
	encoderPrefsLayout->addLayout(hbLayout, 2, 1, 1, 2);
	encoderPrefsLayout->addWidget(startEncoderLabel, 3, 0);
	encoderPrefsLayout->addWidget(startEncoder, 3, 1, 1, 2);
	encoderPrefsLayout->addWidget(stopEncoderLabel, 4, 0);
	encoderPrefsLayout->addWidget(stopEncoder, 4, 1, 1, 2);
	encoderPrefs->setLayout(encoderPrefsLayout);
}


void ExportTab::initialize()
{
	Logger::get().logDebug("Initializing encoder settings");
	PreferencesTool *pref = PreferencesTool::get();

	int numEncoders = pref->getPreference("numEncoders", 0);
	encoderTable->setRowCount(numEncoders);
	for (int i = 0; i < numEncoders; ++i) {
		Preference name(QString("encoderName%1").arg(i).toLatin1().constData(),"");
		encoderTable->setItem(i, 0, new QTableWidgetItem(name.get()) );
		Preference desc(QString("encoderDescription%1").arg(i).toLatin1().constData(),"");
		encoderTable->setItem(i, 1, new QTableWidgetItem(desc.get()) );
		Preference start(QString("startEncoder%1").arg(i).toLatin1().constData(), "");
		startEncoderStrings.push_back(QString(start.get()));
		Preference stop(QString("stopEncoder%1").arg(i).toLatin1().constData(), "");
		stopEncoderStrings.push_back(QString(stop.get()));
		Preference output(QString("outputFile%1").arg(i).toLatin1().constData(), "" );
		outputFiles.push_back( QString(output.get()));
	}

	int active = pref->getPreference("activeEncoder", -1);
	if (active > -1) {
		encoderTable->setCurrentCell(active, 0);
	}
}


void ExportTab::resizeEvent(QResizeEvent *event)
{
	contentsChanged(0, 0);
	QWidget::resizeEvent(event);
}


void ExportTab::apply() {
	PreferencesTool *prefs = PreferencesTool::get();

	// Remove old preferences
	int numEncoders = prefs->getPreference("numEncoders", -1);
 	if (numEncoders > 0) {
		for (int i = 0; i < numEncoders; ++i) {
			prefs->removePreference(QString("encoderName%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("encoderDescription%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("startEncoder%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("stopEncoder%1").arg(i).toLatin1().constData());
			prefs->removePreference(QString("outputFile%1").arg(i).toLatin1().constData());
		}
	}

	// Set new preferences
	numEncoders = encoderTable->rowCount();
	if (numEncoders > 0) {
		prefs->setPreference("numEncoders", numEncoders);
		prefs->setPreference("activeEncoder", encoderTable->currentRow());
		for (int i = 0; i < numEncoders; ++i) {
			prefs->setPreference(QString("encoderName%1").arg(i).toLatin1().constData(),
					encoderTable->item(i, 0)->text().toLatin1().constData());
			prefs->setPreference(QString("encoderDescription%1").arg(i).toLatin1().constData(),
					encoderTable->item(i, 1)->text().toLatin1().constData());
			prefs->setPreference(QString("startEncoder%1").arg(i).toLatin1().constData(),
					startEncoderStrings[i].toLatin1().constData());
			prefs->setPreference(QString("stopEncoder%1").arg(i).toLatin1().constData(),
					stopEncoderStrings[i].toLatin1().constData());
			prefs->setPreference(QString("outputFile%1").arg(i).toLatin1().constData(),
					outputFiles[i].toLatin1().constData());
		}
	} else {
		prefs->setPreference("numEncoders", -1);
		prefs->setPreference("activeEncoder", -1);
	}
}


void ExportTab::addEncoder()
{
	int newRow = encoderTable->rowCount();
	encoderTable->setRowCount(newRow + 1);
	encoderTable->setItem(newRow, 0, new QTableWidgetItem(QString("")));
	encoderTable->setItem(newRow, 1, new QTableWidgetItem(QString("")));
	encoderTable->setCurrentCell(newRow, 0);

	startEncoderStrings.push_back("");
	stopEncoderStrings.push_back("");
	outputFiles.push_back("");
}


void ExportTab::removeEncoder()
{
	int selectedRow = encoderTable->currentRow();
	if (selectedRow >= 0) {
		startEncoderStrings.erase(startEncoderStrings.begin() + selectedRow);
		stopEncoderStrings.erase(stopEncoderStrings.begin() + selectedRow);
		outputFiles.erase(outputFiles.begin() + selectedRow);
		encoderTable->removeRow(selectedRow);
		contentsChanged(0, 0);
	}
}


void ExportTab::contentsChanged(int, int)
{
	encoderTable->resizeColumnsToContents();
	int totalWidth = encoderTable->columnWidth(0) + encoderTable->columnWidth(1);
	int tableWidth = encoderTable->width() - 5;
	if ( totalWidth < tableWidth) {
		encoderTable->setColumnWidth( 1, tableWidth - encoderTable->columnWidth(0) );
	}
}


void ExportTab::activeCellChanged(int, int)
{
	if ( encoderPrefs->isVisible() ) {
		editSettings();
	}
}


void ExportTab::editSettings()
{
	int selected = encoderTable->currentRow();
	if (selected >= 0) {
		startEncoder->setText(startEncoderStrings[selected]);
		stopEncoder->setText(stopEncoderStrings[selected]);
		if ( outputFiles[selected] == "" ) {
			setYesButtonOn();
		}
		else {
			setNoButtonOn();
		}
		encoderPrefs->show();
	}
}


void ExportTab::closeSettings( )
{
	encoderPrefs->hide();
	this->resize(minimumSize());
}


void ExportTab::updateStartString(const QString &txt)
{
	startEncoderStrings[encoderTable->currentRow()] = txt;
}


void ExportTab::updateStopString(const QString &txt)
{
	stopEncoderStrings[encoderTable->currentRow()] = txt;
}


void ExportTab::setDefaultOutput(const QString &txt)
{
	outputFiles[encoderTable->currentRow()] = txt;
}


void ExportTab::setYesButtonOn()
{
	yesButton->setChecked(true);
	noButton->setChecked(false);
	defaultOutput->setText("");
	defaultOutput->setEnabled(false);
	browseButton->setEnabled(false);
	outputFiles[encoderTable->currentRow()] = "";
}


void ExportTab::setNoButtonOn()
{
	noButton->setChecked(true);
	yesButton->setChecked(false);
	defaultOutput->setEnabled(true);
	browseButton->setEnabled(true);
	defaultOutput->setText(outputFiles[encoderTable->currentRow()]);
}


void ExportTab::browseFiles()
{
	QString file = QFileDialog::getSaveFileName(this, tr("Choose output file"), getenv("PWD"));
	if ( !file.isEmpty() ) {
		outputFiles[encoderTable->currentRow()] = file;
		defaultOutput->setText(file);
	}
}

void ExportTab::retranslateStrings()
{
	infoText->setHtml(
		"<p>" + tr("Below you can set which program/process Stopmotion should use "
		"for encoding the currently active project to a video file.") + "</p><p>" +
		tr("You should always use <b>$IMAGEPATH</b> and <b>$VIDEOFILE</b> to represent "
		"the image path and the video file, respectively.") + "</p><p>" +
		tr("You can use <b>$FRAMERATE</b> to represent the frame rate currently in use "
				"in the project.") + "</p><p>" +
		tr("Example with mencoder (jpeg images to mpeg4 video):") + "<br>" +
		"mencoder -ovc lavc -lavcopts vcodec=msmpeg4v2:vpass=1 -mf type=jpg:fps=$FRAMERATE -o "
		"$VIDEOFILE mf://$IMAGEPATH/*.jpg");

	QStringList lst;
	lst << tr("Name") << tr("Description");
	encoderTable->setHorizontalHeaderLabels(lst);

	addButton->setText( tr("&Add") );
	removeButton->setText( tr("&Remove") );
	editButton->setText( tr("&Edit") );

	askForOutput->setText(
			tr("Do you want to be asked for an output file every time you choose to export?"));

	yesButton->setText(tr("Yes"));
	noButton->setText(tr("No"));
	defaultOutputLabel->setText( tr("Set default output file:"));
	browseButton->setText(tr("Browse"));
	startEncoderLabel->setText(tr("Start encoder:"));
	stopEncoderLabel->setText(tr("Stop encoder:"));
}
