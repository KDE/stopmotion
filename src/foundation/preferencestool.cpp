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

#include "preferencestool.h"
#include "logger.h"
#include "uiexception.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/file.h>

#include <libxml/parser.h>

using namespace std;


PreferencesTool* PreferencesTool::preferencesTool = 0;


class XmlProp {
	xmlChar* p;
public:
	XmlProp(const _xmlNode* node, const char* name) : p(0) {
		p = xmlGetProp(node, BAD_CAST name);
	}
	~XmlProp() {
		xmlFree(p);
	}
	const char* value() const {
		return reinterpret_cast<char*>(p);
	}
};


PreferencesTool::PreferencesTool() :
		doc(0), dtd(0), rootNode(0), preferences(0), versionNode(0),
		dirty(false), preferencesFile(0) {
	LIBXML_TEST_VERSION;
}


PreferencesTool::~PreferencesTool() {
	cleanTree();
	delete[] preferencesFile;
}


PreferencesTool* PreferencesTool::get() {
	if (preferencesTool == NULL) {
		preferencesTool = new PreferencesTool();
	}
	return preferencesTool;
}

void PreferencesTool::setSavePath(const char* s, bool wantSave) {
	unsigned long length = strlen(s) + 1;
	preferencesFile = new char[length];
	strncpy(preferencesFile, s, length);
	if (wantSave)
		dirty = true;
}

bool PreferencesTool::load(const char* filePath) {
	if (preferencesFile) {
		delete [] preferencesFile;
		preferencesFile = NULL;
	}
	
	if (doc) {
		cleanTree();
	}
	dirty = false;

	doc = xmlReadFile(filePath, NULL, 0);
	if (!doc)
		return false;

	rootNode = xmlDocGetRootElement(doc);
	if (!doc)
		return false;

	xmlNode* node = rootNode->children;
	for (; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE) {
			if (xmlStrEqual(node->name, BAD_CAST "version")) {
				versionNode = node;
			}
		}
		if (node->type == XML_ELEMENT_NODE) {
			if (xmlStrEqual(node->name, BAD_CAST "preferences")) {
				preferences = node;
			}
		}
	}
	if (!preferences || !versionNode) {
		Logger::get().logWarning("Error while parsing preferences file");
		return false;
	}
	return true;
}

void PreferencesTool::setDefaultPreferences(const char* version) {
	delete[] preferencesFile;
	preferencesFile = 0;
	dirty = true;

	doc = xmlNewDoc(BAD_CAST "1.0");
	dtd = xmlCreateIntSubset(doc, BAD_CAST "root", NULL, NULL);

	rootNode = xmlNewNode(NULL, BAD_CAST "root");
	xmlNewProp(rootNode, BAD_CAST "xml:lang", BAD_CAST "en");
	xmlNewProp(rootNode, BAD_CAST "title", BAD_CAST "Preferences");
	xmlDocSetRootElement(doc, rootNode);

	versionNode = xmlNewChild(rootNode, NULL, BAD_CAST "version", NULL);
	xmlNewProp(versionNode, BAD_CAST "version", BAD_CAST version);
	preferences = xmlNewChild(rootNode, NULL, BAD_CAST "preferences", NULL);
}

bool PreferencesTool::isVersion(const char* version) {
	XmlProp v(versionNode, "version");
	const char* versionLoaded = v.value();
	return versionLoaded && strcmp(versionLoaded, version) == 0;
}


void PreferencesTool::setVersion(const char* version) {
	checkInitialized();
	xmlSetProp(versionNode, BAD_CAST "version", BAD_CAST version);
	dirty = true;
}


void PreferencesTool::setPreference(const char* key, const char* attribute) {
	checkInitialized();
	xmlNodePtr node = NULL;
	node = findNode(key);
	
	if (node == NULL) {
		node = xmlNewChild(preferences, NULL, BAD_CAST "pref", NULL);
		xmlNewProp(node, BAD_CAST "key", BAD_CAST key);
		xmlNewProp(node, BAD_CAST "attribute", BAD_CAST attribute);
		dirty = true;
	} else {
		const char* currentValue = (const char*) xmlGetProp(node, BAD_CAST "attribute");
		if (strcmp(currentValue, attribute) != 0) {
			xmlSetProp(node, BAD_CAST "attribute", BAD_CAST attribute);
			dirty = true;
		}
	}
}


void PreferencesTool::setPreference(const char* key, const int attribute) {
	char tmp[11] = {0};
	snprintf(tmp, 11, "%d", attribute);
	setPreference(key, tmp);
}


const char* PreferencesTool::getPreference(const char* key) {
	checkInitialized();
	xmlNode *node = findNode(key);
	return (const char*)xmlGetProp(node, BAD_CAST "attribute");
}


int PreferencesTool::getPreference(const char* key, const int defaultValue) {
	checkInitialized();
	xmlNode *node = findNode(key);
	if (!node) {
		return defaultValue;
	}
	xmlChar *tmp = xmlGetProp(node, BAD_CAST "attribute");
	int ret = atoi((char*)tmp);
	xmlFree(tmp);
	return ret;
}


void PreferencesTool::removePreference(const char* key) {
	checkInitialized();
	xmlNode *node = findNode(key);
	if (node != NULL) {
		xmlUnlinkNode(node);
		xmlFreeNode(node);
		dirty = true;
	}
}


// looks after a locked, writable file
class WriteableFile {
	FILE* f;
public:
	WriteableFile() : f(0) {
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
	FILE* file() const {
		return f;
	}
};

void PreferencesTool::flush() {
	if (dirty) {
		WriteableFile prefs;
		if (!prefs.open(preferencesFile)
				|| xmlDocFormatDump(prefs.file(), doc, 1) == -1) {
			throw UiException(UiException::failedToWriteToPreferencesFile,
					preferencesFile);
		}
		dirty = false;
	}
}


xmlNodePtr PreferencesTool::findNode(const char * key) {
	//Search through the preferences for the element with a key which
	//equals the key parameter.
	xmlNode *node = preferences->children;
	for (; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE) {
			xmlChar *prop = xmlGetProp(node, BAD_CAST "key");
			int found = xmlStrEqual(prop, BAD_CAST key);
			xmlFree(prop);
			if (found)
				return node;
		}
	}
	return 0;
}


void PreferencesTool::checkInitialized() {
	if (doc == NULL) {
		Logger::get().logFatal("A preferencesfile has to be specified before "
				"using the PreferencesTool.");
		exit(1);
	}
}


void PreferencesTool::cleanTree() {
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	doc             = NULL;
	dtd             = NULL;
	rootNode        = NULL;
	preferences     = NULL;
}

Preference::Preference(const char* key) : val(0), owns(false) {
	val = PreferencesTool::get()->getPreference(key);
	if (val)
		owns = true;
}

Preference::Preference(const char* key, const char* defaultValue)
	: val(0), owns(false) {
	val = PreferencesTool::get()->getPreference(key);
	if (val) {
		owns = true;
	} else {
		val = defaultValue;
	}
}

Preference::~Preference() {
	if (owns)
		xmlFree((xmlChar*)val);
}

const char* Preference::get() const {
	return val;
}

bool Preference::equals(const char* str) {
	if (str) {
		return val? 0 == strcmp(val, str) : false;
	} else {
		return !val;
	}
}
