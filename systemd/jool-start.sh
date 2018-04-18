#!/bin/bash

/usr/local/bin/$1 --instance --add
if [ ! $? -eq 0 ]; then
	exit 1
fi

/usr/local/bin/$1 --file $2
if [ ! $? -eq 0 ]; then
	/usr/local/bin/$1 --instance --remove
	exit 1
fi

exit 0
