#!/bin/bash

#Build recovery only
#./BUILDME.sh skip-kernel-rebuild $@

#Build 64-bit kernel
#./BUILDME.sh skip-kernel-6 skip-kernel-7 skip-kernel-7l  $@

#Build 32-bit kernels
#./BUILDME.sh skip-kernel-8  $@

#Build kernel-7l
./BUILDME.sh skip-kernel-6 skip-kernel-7 skip-kernel-8  $@

