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
#ifndef PREFERENCESTOOL_H
#define PREFERENCESTOOL_H

#include <libxml/tree.h>


/**
 * A xml based tool for adding, changing and removing of
 * various preferences. Everything is saved to a xml organized file
 * and can be readed by the tool for later usage.
 * 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class PreferencesTool
{
public:
	/**
	 * Retrieves the instance of the PreferencesTool class.
	 * @return the PreferencesTool singleton instance.
	 */
	static PreferencesTool* get();
	
	/**
	 * Sets the file to store preferences in.
	 * @param filePath the path to the file where the preferences are
	 * stored.
	 * @param version the version of the program
	 * @return true if file exists. false if the file doesn't exists
	 * and the preferencesTool makes it.
	 */
	bool setPreferencesFile(const char* filePath, const char* version);
	
	/**
	 * Sets the version of the preference file.
	 * @param version the version number of the preference file
	 */
 	void setVersion(const char *version);
	
	/**
	 * Returns the version number of the preference file or 0 if
	 * there are no version tag.
	 * @return the version number of the preference file or 0 if
	 * there are no version tag.
	 */
 	const char* getOldVersion();
	
	/**
	 * Adds a string preference.
	 * @param key the key for retrieving the preference.
	 * @param attribute the attribute for the preference.
	 * @param flushLater true if you don't want the preferencesTool to flush
	 * the preferences to disk at once. This is given for to allow 
	 * optimalization when several preferences ar saved at the same time.
	 * flushPreferences() should be called at once after the preferences are
	 * set to store them to disk.
	 * @return true if the preference was succesfully saved. If flushLater is
	 * set to true this function will return true automaticaly.
	 */
	bool setPreference(const char* key, const char* attribute, bool flushLater = false);
	
	/**
	 * Adds an int preference.
	 * @param key the key for retrieving the preference.
	 * @param attribute the attribute for the preference.
	 * @param flushLater true if you don't want the preferencesTool to flush
	 * the preferences to disk at once. This is given for to allow 
	 * optimalization when several preferences ar saved at the same time.
	 * flushPreferences() should be called at once after the preferences are
	 * set to store them to disk.
	 * @return true if the preference was succesfully saved. If flushLater is
	 * set to true this function will return true automaticaly.
	 */
	bool setPreference(const char* key, const int attribute, bool flushLater = false);
	
	/**
	 * Retrieves a string preference.
	 * @param key the key of the preference to retrieve.
	 * @param defaultValue a default value for preferences which aren't set
	 * by the user yet.
	 * @return the attribute for the given key or "defaultValue" if the key 
	 * wasn't found.
	 */
	const char* getPreference(const char* key, const char* defaultValue);
	
	/**
	 * Retrieves an int preference.
	 * @param key the key of the preference to retrieve.
	 * @param defaultValue a default value for preferences which aren't set
	 * by the user yet.
	 * @return the attribute for the given key or "defaultValue" if the key 
	 * wasn't found.
	 */
	int getPreference(const char* key, const int defaultValue);
	
	/**
	 * Removes the preference with the key "key". (Which, in practice, means 
	 * setting it to default).
	 * @param key the key of the preference to remove.
	 */
	void removePreference(const char* key);
	
	/**
	 * Flushes the preferences to the file specified with setPreferencesFile(..).
	 * @return true if the preferences were succesfully saved.
	 */
	bool flushPreferences();
	
	
protected:
	/**
	 * Protected constructor to deny external instanciation of the singleton.
	 */
	PreferencesTool();
	
	/**
	 * Cleans up after the preferencestool.
	 */
	~PreferencesTool();
	
private:
	/**The singleton instance of this class.*/
	static PreferencesTool *preferencesTool;
	
	xmlDocPtr doc;
	xmlDtdPtr dtd; 
	xmlNodePtr rootNode;
	xmlNodePtr preferences;
	xmlNodePtr versionNode;
	
	char *preferencesFile;
	char *oldVersion;
	
	/**
	 * Retrieves the node with key "key".
	 * @param key the key of the node to retrieve.
	 * @return the node with the given key.
	 */
	xmlNodePtr findNode(const char* key);
	
	/**
	 * Checks if the file at filePath exists.
	 * @return true if the file exist.
	 */
	bool fileExists(const char* filePath);
	
	/**
	 * Checks if the preferencestool has been initialized and exits with an error
	 * if it hasn't.
	 */
	void checkInitialized();
	
	/**
	 * Cleans the xml tree.
	 */
	void cleanTree();
};

#endif
