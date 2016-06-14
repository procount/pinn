#!/bin/sh

# backup the source code to pinn
git push -f pinn pinn:master
cp README.md ../pinn/README_PINN.md
cd ~/pinn
git checkout -f master
# move the readme documentation around
cp README.md README_PINN.md
cp README_SUM.md README.md
git add README.md
git add README_PINN.md
git commit --amend 

# Copy the distributable files
cd ~/noobs_test/output
dst=~/pinn-release/dist/

cp *.dtb $dst
cp bootcode.bin $dst
cp BUILD-DATA $dst
cp INSTRUC* $dst
cp -r defaults $dst
cp -r os $dst
cp -r overlays $dst
cp *.img $dst
cp recovery* $dst
cp riscos-boot.bin $dst
cp RECOVERY_FILES_DO_NOT_EDIT $dst

# Make amendments
cp ~/pinn-release/recovery.cmdline $dst

# Create the Zip file
cd $dst
zip -r ../pinn-lite.zip *
