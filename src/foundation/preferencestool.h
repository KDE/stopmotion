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
#ifndef PREFERENCESTOOL_H
#define PREFERENCESTOOL_H

#include <libxml/tree.h>

/**
 * Represents one string from the preferences file.
 */
class Preference {
	const char* val;
	bool owns;
public:
	/**
	 * Retrieves the value associated with {@a key}. {@ref get} will return
	 * this value, or {@c 0} if no such value exists.
	 */
	Preference(const char* key);
	/**
	 * Retrieves the value associated with {@a key}. {@ref get} will return
	 * this value, or {@a defaultValue} if no such value exists.
	 * @note No copy of the string pointed to by {@a defaultValue} will be
	 * taken; it must remain valid until any caller of {@ref get} has finished
	 * with it.
	 */
	Preference(const char* key, const char* defaultValue);
	~Preference();
	/**
	 * Retrieves the value associated with the key passed in the constructor.
	 * If there was no such value, the pointer passed as the default value in
	 * the constructor is returned. Otherwise, null is returned.
	 * @return The associated value (which becomes invalid after this object
	 * goes out of scope), the default value (which is not a copy), or null.
	 * Ownership is not returned.
	 */
	const char* get() const;
	/**
	 * Tests the retrieved (or default) value against the argument.
	 * @param str The string to compare against.
	 * @return {@c true} if both {@a str} and the value is null, or if both
	 * compare equal. {@c false} otherwise.
	 */
	bool equals(const char* str);
};

/**
 * A xml based tool for adding, changing and removing of
 * various preferences. Everything is saved to a xml organized file
 * and can be read by the tool for later usage.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class PreferencesTool {
public:
	/**
	 * Retrieves the instance of the PreferencesTool class.
	 * @return the PreferencesTool singleton instance.
	 */
	static PreferencesTool* get();

	/**
	 * Loads preferences from the file.
	 * @param filePath the path to the file where the preferences are
	 * stored.
	 * @return true if the file is loaded correctly, otherwise false.
	 */
	bool load(const char* filePath);

	/**
	 * Sets the preferences all to default values.
	 * @param version The version number to write into the preferences.
	 * @param filePath The path to save the preferences to when flushed.
	 */
	void setDefaultPreferences(const char* version);

	/**
	 * Checks if the version matches the string given.
	 * @param version The version to check.
	 * @return true if the version matches the version parameter, false if it does not.
	 */
	bool isVersion(const char* version);

	/**
	 * Sets the version of the preference file.
	 * @param version the version number of the preference file
	 */
 	void setVersion(const char *version);

	/**
	 * Returns the version number of the preference file or 0 if
	 * there is no version tag.
	 * @return the version number of the preference file or 0 if
	 * there is no version tag.
	 */
 	const char* getOldVersion();

	/**
	 * Adds a string preference.
	 * @param key the key for retrieving the preference.
	 * @param attribute the attribute for the preference.
	 * flushPreferences() should be called at once after the all preferences
	 * are set to store them to disk.
	 */
	void setPreference(const char* key, const char* attribute);

	/**
	 * Adds an int preference.
	 * @param key the key for retrieving the preference.
	 * @param attribute the attribute for the preference.
	 * flushPreferences() should be called at once after the all preferences
	 * are set to store them to disk.
	 */
	void setPreference(const char* key, const int attribute);

	/**
	 * Retrieves a string preference.
	 * @param key the key of the preference to retrieve.
	 * @return the attribute for the given key or null if the key wasn't found.
	 */
	const char* getPreference(const char* key);

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
	 * Sets the path that the preferences will be saved to.
	 * @param path The path to save to.
	 * @param wantSave true if the preferences should be saved to this file at
	 * the earliest opportunity. Should be true if the path set is different to
	 * the path loaded from.
	 */
	void setSavePath(const char* path, bool wantSave);

	/**
	 * Flushes the preferences to the file specified with setPreferencesFile(..).
	 * @throw UiException if there is an error.
	 */
	void flush();


protected:
	/**
	 * Protected constructor to deny external instantiation of the singleton.
	 */
	PreferencesTool();

	/**
	 * Cleans up after the preferencestool.
	 */
	~PreferencesTool();

private:
	/** The singleton instance of this class.*/
	static PreferencesTool *preferencesTool;

	xmlDocPtr doc;
	xmlDtdPtr dtd;
	xmlNodePtr rootNode;
	xmlNodePtr preferences;
	xmlNodePtr versionNode;
	bool dirty;

	char *preferencesFile;
	char *oldVersion;

	/**
	 * Retrieves the node with key "key".
	 * @param key the key of the node to retrieve.
	 * @return the node with the given key.
	 */
	xmlNodePtr findNode(const char* key);

	/**
	 * Checks if the preferences tool has been initialized and exits with an error
	 * if it hasn't.
	 */
	void checkInitialized();

	/**
	 * Cleans the xml tree.
	 */
	void cleanTree();
};

#endif
