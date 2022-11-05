#!/bin/sh

#-----------------------------------------------------------------
CMDNAME=`basename $0`
BASE=`pwd`
VER=$(date '+0.%y.%m.%d')
DATE=$(date '+%Y%m%d')
#-----------------------------------------------------------------
BINDIR="OpenLara_dc-plainfiles"
ELF="$2.elf"
BIN="$2.bin"

make_bin() {

	if [ ! -e $ELF ];then
	    	echo "file not exist $ELF" 1>&2
		exit
	fi

	if [ -e IP.BIN ];then
	    	echo "IP.BIN exist" 1>&2
		exit
	fi

	sh-elf-objcopy -S -R .stack -O binary $ELF $BIN

	/opt/toolchains/dc/kos/utils/scramble/scramble $BIN 1ST_READ.BIN

	cat <<EOF > ip.txt
Hardware ID   : SEGA SEGAKATANA
Maker ID      : SEGA ENTERPRISES
Device Info   : 0000 CD-ROM1/1
Area Symbols  : JUE
Peripherals   : E000F10
Product No    : T0000
Version       : V1.000
Release Date  : $DATE
Boot Filename : 1ST_READ.BIN
SW Maker Name : HISTAT
Game Title    : OpenLara Dreamcast
EOF
	/opt/toolchains/dc/kos/utils/makeip/makeip ip.txt IP.BIN

}

make_zip() {
	zip $BINDIR.zip IP.BIN 1ST_READ.BIN
}

case $1 in
    all)
    make_bin
    make_zip
    ;;
    bin)
    make_bin
    ;;
    zip)
    make_zip
    ;;
    *)
    echo "USAGE: $CMDNAME (all|bin|zip)" 1>&2
    ;;
esac
