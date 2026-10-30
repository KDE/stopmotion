/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
	virtual int getSelectionAnchor() const = 0;

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
