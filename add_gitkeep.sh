#!/bin/sh

# This script purpose is to add an empty '.gitkeep' file in every empty directory
# of the project, in order for those empty directories to be pushed/pulled with git.


find . -type d -empty -not -path "./.git/*" -exec touch {}/.gitkeep \;
