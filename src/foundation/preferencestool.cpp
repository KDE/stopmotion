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
#include "preferencestool.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
using namespace std;


PreferencesTool* PreferencesTool::preferencesTool = 0;


PreferencesTool::PreferencesTool() :
		doc(0), dtd(0), rootNode(0), preferences(0), versionNode(0),
		preferencesFile(0), oldVersion(0) {
	LIBXML_TEST_VERSION;
}


PreferencesTool::~PreferencesTool()
{
	cleanTree();
	
	delete[] preferencesFile;
	preferencesFile = NULL;

	delete [] oldVersion;
	oldVersion = NULL;
}


PreferencesTool* PreferencesTool::get() 
{
	//Lazy initialization
	if (preferencesTool == NULL) {
		preferencesTool = new PreferencesTool();
	}
	return preferencesTool;
}


bool PreferencesTool::setPreferencesFile( const char *filePath, const char *version )
{
	xmlNode *node        = NULL;
 	char* currentVersion = NULL;
	
	if (preferencesFile != NULL) {
		delete [] preferencesFile;
		preferencesFile = NULL;
	}
	
	if (doc != NULL) {
		cleanTree();
	}
	
	//Deepcopies the path
	preferencesFile = new char[strlen(filePath)+1];
	strcpy(preferencesFile, filePath);

	if ( fileExists(filePath) ) {
		//Parse the xml file as an xml tree
		doc = xmlReadFile(filePath, NULL, 0);
		if (doc == NULL) {
			printf("Could not parse the preferences file");
		}

		rootNode = xmlDocGetRootElement(doc);

		node = rootNode->children;
		for (; node; node = node->next) {
			if (node->type == XML_ELEMENT_NODE) {
				if (xmlStrEqual(node->name, BAD_CAST "version")) {
					versionNode = node;
				}
			}
		}
		currentVersion = (char*)xmlGetProp(versionNode, BAD_CAST "version");

		//There are no version in the file
		if (currentVersion == NULL) {
			cleanTree();
		}
		else {
			//The version in the file is wrong
			if (strcmp(currentVersion, version) != 0) {
				oldVersion = new char[strlen(currentVersion)+1];
				strcpy(oldVersion, currentVersion);
				cleanTree();
			}
			xmlFree((xmlChar*)currentVersion);
		}
	}

	if (rootNode != NULL) {
		node = rootNode->children;
		for (; node; node = node->next) {
			if (node->type == XML_ELEMENT_NODE) {
				if (xmlStrEqual(node->name, BAD_CAST "preferences")) {
					preferences = node;
				}
			}
		}
		
		if (preferences == NULL) {
			printf("Error while parsing preferences file");
		}
		return true;
	}
	//If there are no file, no version or if the version is wrong a new preferences 
	//file are made.
	else {
		//Create the xml tree
		doc = xmlNewDoc(BAD_CAST "1.0");
		dtd = xmlCreateIntSubset(doc, BAD_CAST "root", NULL, BAD_CAST filePath);
		
		rootNode = xmlNewNode(NULL, BAD_CAST "root");
		xmlNewProp(rootNode, BAD_CAST "xml:lang", BAD_CAST "en");
		xmlNewProp(rootNode, BAD_CAST "title", BAD_CAST "Preferences");
		xmlDocSetRootElement(doc, rootNode);
		
		versionNode = xmlNewChild(rootNode, NULL, BAD_CAST "version", NULL);
		xmlNewProp(versionNode, BAD_CAST "version", BAD_CAST version);
		preferences = xmlNewChild(rootNode, NULL, BAD_CAST "preferences", NULL);
		flushPreferences();
		
		return false;
	}
}


void PreferencesTool::setVersion(const char* version)
{
	checkInitialized();
	xmlSetProp(versionNode, BAD_CAST "version", BAD_CAST version);
	flushPreferences();
}


const char* PreferencesTool::getOldVersion()
{
	return oldVersion;
}


bool PreferencesTool::setPreference(const char* key, const char* attribute, bool flushLater )
{
	checkInitialized();
	xmlNodePtr node = NULL;
	node = findNode(key);
	
	if (node == NULL) {
		node = xmlNewChild(preferences, NULL, BAD_CAST "pref", NULL);
		xmlNewProp(node, BAD_CAST "key", BAD_CAST key);
		xmlNewProp(node, BAD_CAST "attribute", BAD_CAST attribute);
	}
	else {
		xmlSetProp(node, BAD_CAST "attribute", BAD_CAST attribute);
	}
	
	return (!flushLater) ? flushPreferences() : true;
}


bool PreferencesTool::setPreference(const char * key, const int attribute, bool flushLater)
{
	checkInitialized();
	xmlNodePtr node = NULL;
	node = findNode(key);
	
	char tmp[11] = {0};
	snprintf(tmp, 11, "%d", attribute);
	
	if (node == NULL) {
		node = xmlNewChild(preferences, NULL, BAD_CAST "pref", NULL);
		xmlNewProp(node, BAD_CAST "key", BAD_CAST key);
		xmlNewProp(node, BAD_CAST "attribute", BAD_CAST tmp);
	}
	else {
		xmlSetProp(node, BAD_CAST "attribute", BAD_CAST tmp);
	}
	
	return (!flushLater) ? flushPreferences() : true;
}


const char* PreferencesTool::getPreference(const char* key, const char* defaultValue)
{
	checkInitialized();
	xmlNode *node = findNode(key);
	if (node != NULL) {
		return (const char*)xmlGetProp(node, BAD_CAST "attribute");
	}
	else {
		return defaultValue;
	}
}


int PreferencesTool::getPreference(const char * key, const int defaultValue)
{
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


void PreferencesTool::removePreference( const char * key )
{
	checkInitialized();
	xmlNode *node = findNode(key);
	if (node != NULL) {
		xmlUnlinkNode(node);
		xmlFreeNode(node);
		flushPreferences();
	}
}


bool PreferencesTool::flushPreferences()
{
	if (xmlSaveFormatFile(preferencesFile, doc, 1) == -1) {
		return false;
	}
	else {
		return true;
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


bool PreferencesTool::fileExists(const char * filePath)
{
	if (access(filePath, R_OK) == -1) {
		return false;
	}
	return true;
}


void PreferencesTool::checkInitialized()
{
	if (doc == NULL) {
		printf(	"A preferencesfile has to be specified before "
				"using the PreferencesTool.");
		exit(1);
	}
}


void PreferencesTool::cleanTree()
{
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	doc             = NULL;
	dtd             = NULL;
	rootNode        = NULL;
	preferences     = NULL;
}
