#!/bin/sh

rename 's/\.cc/.c/' *.cc
rename 's/\.hh/.h/' *.hh

for dir in `ls .`
do
	if [ -d $dir ]
	then
		echo $dir
		cd $dir
		rename 's/\.cc/.c/' *.cc
		rename 's/\.hh/.h/' *.hh
		cd ..
	fi
done
