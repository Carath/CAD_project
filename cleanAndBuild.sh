#!/bin/sh

# This script purpose is to build several projects with one single command.
# Launch it with the command:
# sh cleanAndBuild.sh


cleanAndBuild()
{
	cd $1
	echo '\n'//////////////////////////////////////////////////
	echo // Cleaning /$1/:
	echo //////////////////////////////////////////////////'\n'
	make clean
	echo '\n'//////////////////////////////////////////////////
	echo // Building /$1/:
	echo //////////////////////////////////////////////////'\n'
	make
	cd ..
}


# Do not change the following execution order:
cleanAndBuild NeuralLib
cleanAndBuild Doc9000
cleanAndBuild Animation


exit
