#!/bin/sh
set -x
# backup the source code to pinn
cd ~/pinn
git checkout p2.4.5a
cd ~/noobs_test
git push -f pinn pinn2:master
cd ~/pinn
git checkout -f master
# move the readme documentation around
#cp ../noobs_test/README.md README_PINN.md
cp ../noobs_test/README_SUM.md README.md
git add README.md
git add README_PINN.md
git commit --amend 

# Copy the distributable files
cp history.md ../pinn-release/README.md

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
cp ~/pinn-os/os/overrides.json $dst

# Make amendments
cp ~/pinn-release/recovery.cmdline $dst
cp ~/pinn-release/recovery.cmdline.new $dst

# Create the Zip file
cd $dst
zip -r ../pinn-lite.zip *
cp BUILD-DATA ..
cd ..
./create_etcher_img
