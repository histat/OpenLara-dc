OPTIMISE = -ffast-math -mpretend-cmove -fno-stack-protector

#DEBUG = 0

CC = $(KOS_CC)
CXX = $(KOS_CXX) 
LDFLAGS = $(KOS_LDFLAGS) $(KOS_START)

INCLUDES = ../.. ../dc

CFLAGS = $(KOS_CFLAGS) -D__DC__ $(addprefix -I,$(INCLUDES)) $(OPTIMISE) -Wno-format -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-value -Wno-sign-compare
CXXFLAGS = -std=gnu++11 $(KOS_CXXFLAGS) -fno-use-cxa-atexit

TARGET := oldc-debug.elf

ifeq ($(DEBUG), 0)
CFLAGS += -DNOSERIAL
TARGET := oldc-release.elf
endif


OBJS = main.o
#OBJS += ../../libs/stb_vorbis/stb_vorbis.o
OBJS += ../../libs/tinf/tinflate.o 
OBJS += dc_hardware.o libpspvram/valloc.o
OBJS += audio.o aica.bin.o
OBJS += xmtrx.o xmtrx_c.o
OBJS += primitive/polygon.o primitive/modifier.o primitive/prim_buffer.o
OBJS += watchdog.o profile.o


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET)  \
		$(OBJS) $(OBJEXTRA) -lz -lm -lstdc++ $(KOS_LIBS)

main.o: ../../gapi/ta.h ../../*.h

primitive/modifier.o: primitive/primitive.h
primitive/polygon.o: primitive/primitive.h
primitive/prim_buffer.o: primitive/primitive.h

include $(KOS_BASE)/Makefile.rules

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

strip:
	sh-elf-strip $(TARGET) -o $(TARGET)
addr:
	sh-elf-addr2line -e $(TARGET) $(ARG)

run: $(TARGET)
	$(KOS_LOADER) -x $< -i DATA.ISO

distclean:
	$(RM) *.ISO *.CDI *.BIN *.elf *.iso ip.txt 1ST_READ.BIN

emu:
	/mnt/c/opt/demul/demul.exe -run=dc -image=TEST.CDI

data:
	mkisofs -U -D -o $< DATA.ISO $(HOME)/data

iso:
	mkdcdisc -e $(TARGET) -d $(HOME)/data/DATA -d $(HOME)/data/DELDATA -n "Openlara" -a "histat" -N -o TEST.CDI

dist: 1ST_READ.BIN IP.BIN

OLDC.BIN: $(TARGET)
	sh-elf-objcopy -S -R .stack -O binary $< $@

1ST_READ.BIN: OLDC.BIN
	scramble $< $@

ip.txt: ip.txt.in
	sed -e 's/[@]DATE[@]/$(shell date '+%Y%m%d')/' < $< > $@ 

IP.BIN: ip.txt
	makeip ip.txt IP.BIN

banner_data:
	echo 'static unsigned char banner_data[] = {' >> $@.tmp
	hexdump -v -e '16/1 "0x%02x, " "\n"' \
	data/banner.bin >> $@.tmp
	echo '		0 };' >> $@.tmp

icon_data_2bpp:
	echo 'static unsigned char icon_data_2bpp[] = {' >> $@.tmp
	hexdump -v -s 9 -e '16/1 "0x%02x, " "\n"' \
	icon.pbm >> $@.tmp
	echo '		0 };' >> $@.tmp
