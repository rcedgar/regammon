#!/bin/bash


if [ -d c:/cygwin64/bin ] ; then
	bin=c:/cygwin64/bin
elif [ -d c:/cygwin/bin ] ; then
	bin=c:/cygwin/bin
else
	echo 'c:/cygwin[64] not found' >> /sys/stderr
fi

v=`$bin/svnversion`
echo \"$v\" > /tmp/svnversion.h

if [ ! -f svnversion.h ] ; then
	$bin/cp /tmp/svnversion.h .
else
	$bin/diff svnversion.h /tmp/svnversion.h >> /dev/null

	if [ $? != 0 ] ; then
		echo New version
		$bin/cp /tmp/svnversion.h .
	else
		echo Same version
	fi
	$bin/cat svnversion.h
fi
