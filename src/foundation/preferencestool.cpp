/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "preferencestool.h"
#include "logger.h"
#include "uiexception.h"

#include <QFile>
#include <QString>
#include <QStringList>
#include <QXmlStreamWriter>

#include <algorithm>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/file.h>

namespace {
QString fromUtf8(const char* s) {
	return QString::fromUtf8(s);
}

// QDomDocument writes attributes in an unpredictable order, so the
// preferences are written with QXmlStreamWriter in a fixed order instead.
void writeElement(QXmlStreamWriter& writer, const QDomElement& element) {
	static const QStringList knownOrder = {
		QStringLiteral("xml:lang"), QStringLiteral("title"),
		QStringLiteral("version"), QStringLiteral("key"),
		QStringLiteral("attribute")};
	const QDomNamedNodeMap attributes = element.attributes();
	QStringList names;
	for (int i = 0; i != attributes.count(); ++i)
		names.append(attributes.item(i).nodeName());
	std::sort(names.begin(), names.end(),
			[](const QString& a, const QString& b) {
		qsizetype ia = knownOrder.indexOf(a);
		qsizetype ib = knownOrder.indexOf(b);
		if (ia < 0)
			ia = knownOrder.size();
		if (ib < 0)
			ib = knownOrder.size();
		return ia != ib? ia < ib : a < b;
	});
	QDomElement child = element.firstChildElement();
	if (child.isNull())
		writer.writeEmptyElement(element.tagName());
	else
		writer.writeStartElement(element.tagName());
	for (const QString& name : names)
		writer.writeAttribute(name, element.attribute(name));
	if (!child.isNull()) {
		for (; !child.isNull(); child = child.nextSiblingElement())
			writeElement(writer, child);
		writer.writeEndElement();
	}
}

QByteArray toXml(const QDomDocument& doc) {
	QByteArray out;
	QXmlStreamWriter writer(&out);
	writer.setAutoFormatting(true);
	writer.setAutoFormattingIndent(2);
	writer.writeStartDocument();
	writer.writeDTD(QStringLiteral("<!DOCTYPE root>"));
	writeElement(writer, doc.documentElement());
	writer.writeEndDocument();
	return out;
}
}


PreferencesTool* PreferencesTool::preferencesTool = nullptr;


PreferencesTool::PreferencesTool() : dirty(false) {
}


PreferencesTool::~PreferencesTool() {
	cleanTree();
}


PreferencesTool* PreferencesTool::get() {
	if (preferencesTool == nullptr) {
		preferencesTool = new PreferencesTool();
	}
	return preferencesTool;
}

void PreferencesTool::setSavePath(const char* s, bool wantSave) {
	preferencesFile = s;
	if (wantSave)
		dirty = true;
}

bool PreferencesTool::load(const char* filePath) {
	preferencesFile.clear();
	cleanTree();
	dirty = false;

	QFile file(QString::fromLocal8Bit(filePath));
	if (!file.open(QIODevice::ReadOnly))
		return false;
	QDomDocument loaded;
	if (!loaded.setContent(&file))
		return false;

	QDomElement root = loaded.documentElement();
	QDomElement loadedVersion = root.firstChildElement(QStringLiteral("version"));
	QDomElement loadedPreferences =
			root.firstChildElement(QStringLiteral("preferences"));
	if (loadedPreferences.isNull() || loadedVersion.isNull()) {
		Logger::get().logWarning("Error while parsing preferences file");
		return false;
	}
	doc = loaded;
	versionNode = loadedVersion;
	preferences = loadedPreferences;
	return true;
}

void PreferencesTool::setDefaultPreferences(const char* version) {
	cleanTree();
	preferencesFile.clear();
	dirty = true;

	doc = QDomDocument(QStringLiteral("root"));
	QDomElement rootNode = doc.createElement(QStringLiteral("root"));
	rootNode.setAttribute(QStringLiteral("xml:lang"), QStringLiteral("en"));
	rootNode.setAttribute(QStringLiteral("title"), QStringLiteral("Preferences"));
	doc.appendChild(rootNode);

	versionNode = doc.createElement(QStringLiteral("version"));
	versionNode.setAttribute(QStringLiteral("version"), fromUtf8(version));
	rootNode.appendChild(versionNode);
	preferences = doc.createElement(QStringLiteral("preferences"));
	rootNode.appendChild(preferences);
}

bool PreferencesTool::isVersion(const char* version) {
	return versionNode.hasAttribute(QStringLiteral("version"))
			&& versionNode.attribute(QStringLiteral("version")) == fromUtf8(version);
}


void PreferencesTool::setVersion(const char* version) {
	checkInitialized();
	versionNode.setAttribute(QStringLiteral("version"), fromUtf8(version));
	dirty = true;
}


void PreferencesTool::setPreference(const char* key, const char* attribute) {
	checkInitialized();
	const QString value = fromUtf8(attribute);
	QDomElement node = findNode(key);
	if (node.isNull()) {
		node = doc.createElement(QStringLiteral("pref"));
		node.setAttribute(QStringLiteral("key"), fromUtf8(key));
		node.setAttribute(QStringLiteral("attribute"), value);
		preferences.appendChild(node);
		dirty = true;
	} else if (!node.hasAttribute(QStringLiteral("attribute"))
			|| node.attribute(QStringLiteral("attribute")) != value) {
		node.setAttribute(QStringLiteral("attribute"), value);
		dirty = true;
	}
}


void PreferencesTool::setPreference(const char* key, const int attribute) {
	char tmp[12] = {0};
	snprintf(tmp, sizeof(tmp), "%d", attribute);
	setPreference(key, tmp);
}


std::optional<QByteArray> PreferencesTool::getPreference(const char* key) {
	checkInitialized();
	QDomElement node = findNode(key);
	if (node.isNull() || !node.hasAttribute(QStringLiteral("attribute")))
		return std::nullopt;
	return node.attribute(QStringLiteral("attribute")).toUtf8();
}


int PreferencesTool::getPreference(const char* key, const int defaultValue) {
	std::optional<QByteArray> value = getPreference(key);
	if (!value)
		return defaultValue;
	return atoi(value->constData());
}


void PreferencesTool::removePreference(const char* key) {
	checkInitialized();
	QDomElement node = findNode(key);
	if (!node.isNull()) {
		preferences.removeChild(node);
		dirty = true;
	}
}


// looks after a locked, writable file
class WriteableFile {
	FILE* f;
public:
	WriteableFile(const WriteableFile&) = delete;
	WriteableFile& operator=(const WriteableFile&) = delete;
	WriteableFile() : f(nullptr) {
	}
	~WriteableFile() {
		if (f)
			fclose(f);
	}
	bool open(const char* filename) {
		f = fopen(filename, "w");
		if (!f)
			return false;
		flock(fileno(f), LOCK_EX);
		return true;
	}
	bool write(const QByteArray& data) {
		return fwrite(data.constData(), 1, data.size(), f)
				== static_cast<size_t>(data.size());
	}
	bool close() {
		int r = fclose(f);
		f = nullptr;
		return r == 0;
	}
};

void PreferencesTool::flush() {
	if (dirty) {
		WriteableFile prefs;
		if (!prefs.open(preferencesFile.c_str())
				|| !prefs.write(toXml(doc))
				|| !prefs.close()) {
			throw UiException(UiException::failedToWriteToPreferencesFile,
					preferencesFile.c_str());
		}
		dirty = false;
	}
}


QDomElement PreferencesTool::findNode(const char * key) {
	//Search through the preferences for the element with a key which
	//equals the key parameter.
	const QString k = fromUtf8(key);
	for (QDomElement node = preferences.firstChildElement(); !node.isNull();
			node = node.nextSiblingElement()) {
		if (node.attribute(QStringLiteral("key")) == k)
			return node;
	}
	return QDomElement();
}


void PreferencesTool::checkInitialized() {
	if (doc.isNull()) {
		Logger::get().logFatal("A preferencesfile has to be specified before "
				"using the PreferencesTool.");
		exit(1);
	}
}


void PreferencesTool::cleanTree() {
	doc = QDomDocument();
	preferences = QDomElement();
	versionNode = QDomElement();
}

Preference::Preference(const char* key)
	: value(PreferencesTool::get()->getPreference(key)),
	  defaultValue(nullptr) {
}

Preference::Preference(const char* key, const char* defaultValue)
	: value(PreferencesTool::get()->getPreference(key)),
	  defaultValue(defaultValue) {
}

Preference::~Preference() {
}

const char* Preference::get() const {
	return value? value->constData() : defaultValue;
}

bool Preference::equals(const char* str) {
	const char* val = get();
	if (str) {
		return val? 0 == strcmp(val, str) : false;
	} else {
		return !val;
	}
}
