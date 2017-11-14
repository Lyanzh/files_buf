#!/bin/sh

rename 's/\.c/.cc/' *.c
rename 's/\.h/.hh/' *.h

for dir in `ls .`
do
	if [ -d $dir ]
	then
		echo $dir
		cd $dir
		rename 's/\.c/.cc/' *.c
		rename 's/\.h/.hh/' *.h
		cd ..
	fi
done
