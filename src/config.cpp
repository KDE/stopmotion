/*
 * SPDX-FileCopyrightText: 2012-2026 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "src/config.h"

#ifndef HTML_PATH
#define HTML_PATH "/usr/local/share/doc/stopmotion/manual"
#endif

#ifndef TRANSLATIONS_PATH
#define TRANSLATIONS_PATH "/usr/local/share/stopmotion/translations"
#endif

const char *const stopmotion::htmlDirectory = HTML_PATH;
const char *const stopmotion::translationsDirectory = TRANSLATIONS_PATH;
