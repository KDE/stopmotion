/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
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
#include "aboutdialog.h"

#include "licence.h"
#include "graphics/stopmotion_logo.xpm"

#include <QtGui>


AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	tabWidget = new QTabWidget;
	
	QWidget *widget = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;
	QLabel *label = new QLabel;
	label->setPixmap(stopmotion_logo);
	label->setScaledContents(true);
	layout->addWidget(label);
	layout->addWidget( new QLabel(
				"<p>" + tr("This is the stopmotion application for creating stopmotion animations.") + "</p>"
				"<p>(c) 2005, Fredrik Berg Kj�lstad and Bj�rn Erik Nilsen</p>"));
	widget->setLayout(layout);
	
	tabWidget->addTab(widget, tr("&About") );
	
	tabWidget->addTab( new QLabel(
				"<p><b>" + tr("Main developers") + "</b><br>"
				"Fredrik Berg Kj�lstad &lt;fredrikbk@hotmail.com&gt;<br>"
				"Bj�rn Erik Nilsen &lt;bjoern.nilsen@bjoernen.com&gt;</p>"), 
				tr("A&uthors"));
	
	QTextEdit *qte = new QTextEdit;
	qte->setHtml( 
				"<p><b>" + tr("Translation") + "</b><br>"
				"George Helebrant &lt;helb@skatekralovice.com&gt; (" + tr("Czech") + ")<br>"
				"Gorazd Bizjak and Matej Lavreni &lt;info@zapstudio.net&gt; (" + tr("Slovenian") + ")<br>"
				"Guillaume B &lt;littletux@zarb.org&gt; (" + tr("French") + ")<br>"
				"</p>"
				"<p><b>" + tr("Logo") + "</b><br>"
				"Gorazd Bizjak &lt;gorazd@zapstudio.net&gt;<br>"
				"<p><b>" + tr("Coordinating") + "</b><br>"
				"Herman Robak &lt;herman@skolelinux.no&gt;<br>"
				"�yvind Kol�s &lt;pippin@gimp.org&gt;</p>"
				"<p><b>" + tr("Testing") + "</b><br>"
				"Tore Sinding Bekkedal &lt;toresbe@ifi.uio.no&gt;<br>"
				"Finn Arne Johansen &lt;faj@bzz.no&gt;<br>"
				"Halvor Borgen &lt;halvor.borgen@hig.no&gt;<br>"
				"Bj�rn Are Hansen &lt;post@bahansen.net&gt;<br>"
				"John Steinar Bild�y &lt;johnsbil@haldenfriskole.no&gt;<br>"
				"Ole-Anders Andreassen &lt;ole-anders.andreassen@sunndal.kommune.no&gt;<br>"
				"</p>"
				);
	qte->setReadOnly(true);
	qte->setBackgroundRole(backgroundRole());
	tabWidget->addTab(qte, tr("&Thanks To"));
	
	QTextEdit *licenceText = new QTextEdit;
	licenceText->setReadOnly(true);
	licenceText->setPlainText(licence);
	tabWidget->addTab(licenceText, tr("&Licence Agreement"));

	QPushButton *okButton = new QPushButton(tr("OK"), this);
	connect(okButton, SIGNAL(clicked()), this, SLOT(close()));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(okButton);
	
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("About"));
	setMinimumWidth(550);
}

