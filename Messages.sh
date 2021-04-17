#!/usr/bin/env bash

$EXTRACT_TR_STRINGS `find . -name \*.qml -o -name \*.cpp -o -name \*.h` -o $podir/stopmotion_qt.pot
