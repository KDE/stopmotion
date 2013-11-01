/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#ifndef SELECTION_H_
#define SELECTION_H_

/**
 * Interface for retrieving selection.
 */
class Selection {
public:
	virtual ~Selection() {}

	/**
	 * Returns true if the user is currently selecting several thumbviews.
	 * @return true if the user is currently selecting several thumbviews.
	 */
	virtual bool isSelecting() const = 0;

	/**
	 * Returns The frame number of the anchor of the selection.
	 * @return The anchor of the current selection, or returns the same value
	 * as {@ref getActiveFrame} if there is no selection.
	 */
	virtual int getSelectionThumb() const = 0;

	/**
	 * Returns the current active frame.
	 */
	virtual int getActiveFrame() const = 0;

	/**
	 * Returns the current active scene.
	 */
	virtual int getActiveScene() const = 0;
};

#endif
