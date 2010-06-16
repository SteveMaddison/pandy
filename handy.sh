#?bin/sh

BIOS=lynxboot.img

if [ ! -f ./$BIOS ] ; then
	cp "`zenity --file-selection --title="Select your $BIOS file"`" $BIOS
	if [ $? != 0 ] ; then
		zenity --error --text="Sorry, emulator cannot run without a BIOS file."
		exit 1
	fi
	md5sum -c lynxboot.md5 || zenity --question --text="BIOS does not appear to be the correct version. Use it anyway?"
	if [ $? != 0 ] ; then
		rm -f $BIOS
		exit 1
	fi
fi

export LD_LIBRARY_PATH=.
./handy_sdl

