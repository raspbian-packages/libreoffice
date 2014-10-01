#!/bin/sh

libs=`grep libvlc.*\.so avmedia/source/vlc/wrapper/SymbolLoader.hxx | grep -v vlccore | cut -d= -f2 | perl -pe 's/\s+\"(.*)\".*/$1/'`

for l in $libs; do
	if [ -e "/usr/lib/$l" ]; then
#		# sanity check: do the libs match with what we would get
#		# for our libivlc version if we followed the .so symlink?
#		l1=`readlink /usr/lib/$l`
#		l2_tmp=`echo $l | perl -pe 's/(.*)\.\d+$/$1/'`
#		l2=`readlink /usr/lib/$l2_tmp`
#		if [ "$l1" = "$l2" ]; then
			dep=`dpkg -S /usr/lib/$l | cut -d: -f1`
#		fi
	fi
done

if [ -n "$dep" ]; then
	echo $dep
else
	echo "Cannot find libvlc dependency. None of the following libs found:"
	echo $libs
	exit 1
fi

