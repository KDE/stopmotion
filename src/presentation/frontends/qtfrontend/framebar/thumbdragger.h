/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef THUMBDRAGGER_H_
#define THUMBDRAGGER_H_

#include <QDrag>

class ThumbView;

class ThumbDragger: public QDrag {
	Q_OBJECT
public:
	~ThumbDragger() override;
	explicit ThumbDragger(ThumbView* source);
};

#endif /* THUMBDRAGGER_H_ */
