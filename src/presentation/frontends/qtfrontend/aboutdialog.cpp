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
				"<p>" + tr("This is the Stopmotion application for creating stop motion animations.") + "</p>"
				+ QString::fromUtf8(
						"<p> &copy; 2005-2008, Bj\xc3\xb8rn Erik Nilsen and Fredrik Berg Kj\xc3\xb8lstad </p>")));
	widget->setLayout(layout);

	tabWidget->addTab(widget, tr("&About") );

	tabWidget->addTab( new QLabel(
				"<p><b>" + tr("Main developers") + "</b><br>"
				+ QString::fromUtf8(
						"Bj\xc3\xb8rn Erik Nilsen &lt;bjoern.nilsen@bjoernen.com&gt;<br>"
						"Fredrik Berg Kj\xc3\xb8lstad &lt;fredrikbk@hotmail.com&gt;<br>")
				+ "<p><b>" + tr("Contributors") + "</b><br>"
				+ QString::fromUtf8(
					"Eric Stolten &lt;stoltene2@gmail.com&gt;<br>"
					"Josh Green &lt;josh@resonance.org&gt;<br>"
					"Jehan Pag\xc3\xa9s<br>"
				"Tim Band<br>"
				"Ying-Chun Liu (PaulLiu) &lt;paulliu@debian.org&gt;<br>"
				"Barak A. Pearlmutter &lt;bap@debian.org&gt;<br>"
				"Cyril Brulebois &lt;kibi@debian.org&gt;<br>")
				+ "<p>"),
				tr("A&uthors"));

	QTextEdit *qte = new QTextEdit;
	qte->setHtml(
				"<p><b>" + tr("Translation") + "</b><br>"
				"George Helebrant &lt;helb@skatekralovice.com&gt; (" + tr("Czech") + ")<br>"
				"Gorazd Bizjak and Matej Lavreni &lt;info@zapstudio.net&gt; (" + tr("Slovenian") + ")<br>"
				"Guillaume B &lt;littletux@zarb.org&gt; (" + tr("French") + ")<br>"
				"Martin Herweg &lt;m.herweg@gmx.de&gt; (" + tr("German") + ")<br>"
				+ QString::fromUtf8(
				"Jos\xc3\xa9 Jorge &lt;jjorge@free.fr&gt; (") + tr("Portuguese") + ")<br>"
				+ QString::fromUtf8(
				"Manuel Qui\xc3\xb1ones &lt;manuel.por.aca@gmail.com&gt; (") + tr("Spanish") + ")<br>"
				"David Hjelm &lt;dav.hjelm@spray.se&gt; (" + tr("Swedish") + ")<br>"
				+ QString::fromUtf8(
				"Koray L\xc3\xb6ker &lt;loker@pardus.org.tr&gt; (") + tr("Turkish") + ")<br>"
				"Diego Giordano &lt;fusion_machine@tin.it&gt; (" + tr("Italian") + ")<br>"
				"Matteo Comisso &lt;teomatteo8.9@hotmail.it&gt; (" + tr("Italian") + ")<br>"
				"Wei-Lun Chao &lt;bluebat@member.fsf.org&gt; (" + tr("Traditional Chinese") +")<br>"
				"Maysara Abdulhaq and Ahmad Gharbeia of Arab Digital Expression Foundation (" + tr("Arabic") + ")<br>"
				"Denis Fokin (" + tr("Russian") + ")<br>"
				"Yaron (sh-yaron) (" + tr("Hebrew") + ")<br>"
				"</p>"
				"<p><b>" + tr("Logo") + "</b><br>"
				"Gorazd Bizjak &lt;gorazd@zapstudio.net&gt;<br>"
				"<p><b>" + tr("Coordinating") + "</b><br>"
				"Herman Robak &lt;herman@skolelinux.no&gt;<br>"
				+ QString::fromUtf8(
				"\xc3\x98yvind Kol\xc3\xa5s &lt;pippin@gimp.org&gt;</p>")
				+ "<p><b>" + tr("Testing") + "</b><br>"
				+ QString::fromUtf8(
				"Tore Sinding Bekkedal &lt;toresbe@ifi.uio.no&gt;<br>"
				"Finn Arne Johansen &lt;faj@bzz.no&gt;<br>"
				"Halvor Borgen &lt;halvor.borgen@hig.no&gt;<br>"
				"Bj\xc3\xb8rn Are Hansen &lt;post@bahansen.net&gt;<br>"
				"John Steinar Bild\xc3\xb8y &lt;johnsbil@haldenfriskole.no&gt;<br>"
				"Ole-Anders Andreassen &lt;ole-anders.andreassen@sunndal.kommune.no&gt;<br>"
				"</p>")
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

