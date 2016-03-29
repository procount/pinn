#!/bin/sh

git push -f pinn pinn:master
cp README.md ../pinn/README_PINN.md
cd ~/pinn
#git checkout master
cp README.md README_PINN.md
cp README_SUM.md README.md
git add README.md
git add README_PINN.md
git commit --amend 
