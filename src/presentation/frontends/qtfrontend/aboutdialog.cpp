/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "aboutdialog.h"

#include "licence.h"
#include "graphics/stopmotion_logo.xpm"

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	tabWidget = new QTabWidget;

	QWidget *widget = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;
	QLabel *label = new QLabel;
	label->setPixmap(QPixmap(stopmotion_logo));
	label->setScaledContents(true);
	layout->addWidget(label);
	layout->addWidget( new QLabel(
				QStringLiteral("<p>") + tr("This is the Stopmotion application for creating stop motion animations.") + QStringLiteral("</p>")
				+ QString::fromUtf8(
						"<p> &copy; 2005-2016 Bj\xc3\xb8rn Erik Nilsen, Fredrik Berg Kj\xc3\xb8lstad et al </p>")));
	widget->setLayout(layout);

	tabWidget->addTab(widget, tr("&About") );

	tabWidget->addTab( new QLabel(
				QStringLiteral("<p><b>") + tr("Main developers") + QStringLiteral("</b><br>")
				+ QString::fromUtf8(
						"Bj\xc3\xb8rn Erik Nilsen &lt;bjoern.nilsen@bjoernen.com&gt;<br>"
						"Fredrik Berg Kj\xc3\xb8lstad &lt;fredrikbk@hotmail.com&gt;<br>")
				+ QStringLiteral("<p><b>") + tr("Contributors") + QStringLiteral("</b><br>")
				+ QString::fromUtf8(
					"Eric Stolten &lt;stoltene2@gmail.com&gt;<br>"
					"Josh Green &lt;josh@resonance.org&gt;<br>"
					"Jehan Pag\xc3\xa9s<br>"
				"Tim Band<br>"
				"Ying-Chun Liu (PaulLiu) &lt;paulliu@debian.org&gt;<br>"
				"Barak A. Pearlmutter &lt;bap@debian.org&gt;<br>"
				"Cyril Brulebois &lt;kibi@debian.org&gt;<br>")
				+ QStringLiteral("<p>")),
				tr("A&uthors"));

	QTextEdit *qte = new QTextEdit;
	qte->setHtml(
				QStringLiteral("<p><b>") + tr("Translation") + QStringLiteral("</b><br>"
				"George Helebrant &lt;helb@skatekralovice.com&gt; (") + tr("Czech") + QStringLiteral(")<br>"
				"Gorazd Bizjak and Matej Lavreni &lt;info@zapstudio.net&gt; (") + tr("Slovenian") + QStringLiteral(")<br>"
				"Guillaume B &lt;littletux@zarb.org&gt; (") + tr("French") + QStringLiteral(")<br>"
				"Martin Herweg &lt;m.herweg@gmx.de&gt; (") + tr("German") + QStringLiteral(")<br>")
				+ QString::fromUtf8(
				"Jos\xc3\xa9 Jorge &lt;jjorge@free.fr&gt; (") + tr("Portuguese") + QStringLiteral(")<br>")
				+ QString::fromUtf8(
				"Manuel Qui\xc3\xb1ones &lt;manuel.por.aca@gmail.com&gt; (") + tr("Spanish") + QStringLiteral(")<br>"
				"David Hjelm &lt;dav.hjelm@spray.se&gt; (") + tr("Swedish") + QStringLiteral(")<br>")
				+ QString::fromUtf8(
				"Koray L\xc3\xb6ker &lt;loker@pardus.org.tr&gt; (") + tr("Turkish") + QStringLiteral(")<br>"
				"Diego Giordano &lt;fusion_machine@tin.it&gt; (") + tr("Italian") + QStringLiteral(")<br>"
				"Matteo Comisso &lt;teomatteo8.9@hotmail.it&gt; (") + tr("Italian") + QStringLiteral(")<br>"
				"Wei-Lun Chao &lt;bluebat@member.fsf.org&gt; (") + tr("Traditional Chinese") +QStringLiteral(")<br>"
				"Maysara Abdulhaq and Ahmad Gharbeia of Arab Digital Expression Foundation (") + tr("Arabic") + QStringLiteral(")<br>"
				"Denis Fokin (") + tr("Russian") + QStringLiteral(")<br>"
				"Yaron (sh-yaron) (") + tr("Hebrew") + QStringLiteral(")<br>"
				"</p>"
				"<p><b>") + tr("Logo") + QStringLiteral("</b><br>"
				"Gorazd Bizjak &lt;gorazd@zapstudio.net&gt;<br>"
				"<p><b>") + tr("Coordinating") + QStringLiteral("</b><br>"
				"Herman Robak &lt;herman@skolelinux.no&gt;<br>")
				+ QString::fromUtf8(
				"\xc3\x98yvind Kol\xc3\xa5s &lt;pippin@gimp.org&gt;</p>")
				+ QStringLiteral("<p><b>") + tr("Testing") + QStringLiteral("</b><br>")
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
	connect(okButton, &QAbstractButton::clicked, this, &QWidget::close);

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

