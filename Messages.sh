#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2021 Linuxstopmotion contributors; see the AUTHORS file for details
# SPDX-License-Identifier: GPL-2.0-or-later

$EXTRACT_TR_STRINGS `find . -name \*.qml -o -name \*.cpp -o -name \*.h` -o $podir/stopmotion_qt.pot
