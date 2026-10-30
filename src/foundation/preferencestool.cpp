/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "preferencestool.h"
#include "logger.h"
#include "uiexception.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/file.h>

#include <libxml/parser.h>

using namespace std;


PreferencesTool* PreferencesTool::preferencesTool = nullptr;


class XmlProp {
	xmlChar* p;
public:
	XmlProp(const XmlProp&) = delete;
	XmlProp& operator=(const XmlProp&) = delete;
	XmlProp(const _xmlNode* node, const char* name) : p(nullptr) {
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
		doc(nullptr), dtd(nullptr), rootNode(nullptr), preferences(nullptr), versionNode(nullptr),
		dirty(false), preferencesFile(nullptr) {
	LIBXML_TEST_VERSION;
}


PreferencesTool::~PreferencesTool() {
	cleanTree();
	delete[] preferencesFile;
}


PreferencesTool* PreferencesTool::get() {
	if (preferencesTool == nullptr) {
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
		preferencesFile = nullptr;
	}
	
	if (doc) {
		cleanTree();
	}
	dirty = false;

	doc = xmlReadFile(filePath, nullptr, 0);
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
	preferencesFile = nullptr;
	dirty = true;

	doc = xmlNewDoc(BAD_CAST "1.0");
	dtd = xmlCreateIntSubset(doc, BAD_CAST "root", nullptr, nullptr);

	rootNode = xmlNewNode(nullptr, BAD_CAST "root");
	xmlNewProp(rootNode, BAD_CAST "xml:lang", BAD_CAST "en");
	xmlNewProp(rootNode, BAD_CAST "title", BAD_CAST "Preferences");
	xmlDocSetRootElement(doc, rootNode);

	versionNode = xmlNewChild(rootNode, nullptr, BAD_CAST "version", nullptr);
	xmlNewProp(versionNode, BAD_CAST "version", BAD_CAST version);
	preferences = xmlNewChild(rootNode, nullptr, BAD_CAST "preferences", nullptr);
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
	xmlNodePtr node = nullptr;
	node = findNode(key);
	
	if (node == nullptr) {
		node = xmlNewChild(preferences, nullptr, BAD_CAST "pref", nullptr);
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
	if (!tmp) {
		return defaultValue;
	}
	int ret = atoi(reinterpret_cast<const char*>(tmp));
	xmlFree(tmp);
	return ret;
}


void PreferencesTool::removePreference(const char* key) {
	checkInitialized();
	xmlNode *node = findNode(key);
	if (node != nullptr) {
		xmlUnlinkNode(node);
		xmlFreeNode(node);
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
	return nullptr;
}


void PreferencesTool::checkInitialized() {
	if (doc == nullptr) {
		Logger::get().logFatal("A preferencesfile has to be specified before "
				"using the PreferencesTool.");
		exit(1);
	}
}


void PreferencesTool::cleanTree() {
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	doc             = nullptr;
	dtd             = nullptr;
	rootNode        = nullptr;
	preferences     = nullptr;
}

Preference::Preference(const char* key) : val(nullptr), owns(false) {
	val = PreferencesTool::get()->getPreference(key);
	if (val)
		owns = true;
}

Preference::Preference(const char* key, const char* defaultValue)
	: val(nullptr), owns(false) {
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
