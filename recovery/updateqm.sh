#!/bin/sh

export QT_SELECT=4

lupdate -no-obsolete recovery.pro  || true
lrelease recovery.pro
