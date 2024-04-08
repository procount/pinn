#!/bin/sh

export QT_SELECT=4

lupdate -no-obsolete -locations none recovery.pro  || true
lrelease recovery.pro
