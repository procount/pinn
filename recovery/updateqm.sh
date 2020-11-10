#!/bin/sh

export QT_SELECT=5

lupdate -no-obsolete recovery.pro  || true
lrelease recovery.pro
