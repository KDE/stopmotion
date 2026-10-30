/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "thumbdragger.h"
#include "thumbview.h"

ThumbDragger::ThumbDragger(ThumbView* source) : QDrag(source) {
	source->addRef();
}

ThumbDragger::~ThumbDragger() {
	static_cast<ThumbView*>(source())->delRef();
}
