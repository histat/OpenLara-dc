ronindir = $(HOME)/repo/libronin
SH_ELF_BIN = /opt/toolchains/dc/bin

DEBUG = 1

OPTIMISE = -Wall -Os -fomit-frame-pointer -ffast-math
#OPTIMISE += -funroll-loops -fschedule-insns2 -fdelete-null-pointer-checks 

CC = $(SH_ELF_BIN)/sh-elf-gcc -ml -m4-single-only
CXX = $(SH_ELF_BIN)/sh-elf-g++ -ml -m4-single-only -std=gnu++11
AS = $(SH_ELF_BIN)/sh-elf-as -little

INCLUDES = $(ronindir)/include ../.. ../dc

CFLAGS = $(OPTIMISE) -D__DC__ $(addprefix -I,$(INCLUDES)) -Wno-format -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-value -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = -Wl,-Tshlelf.xc -nostartfiles $(ronindir)/lib/crt0.o
LIBS = -L$(ronindir)/lib -lronin -lz -lm -lstdc++

TARGET	= oldc-debug.elf

ifeq ($(DEBUG), 0)
  CFLAGS  += -DNOSERIAL
  LIBS = -L$(ronindir)/lib -lronin-noserial -lz -lm -lstdc++
  TARGET = oldc-nodebug.elf
endif


OBJS = main.o
OBJS += ../../libs/stb_vorbis/stb_vorbis.o #../../libs/tinf/tinflate.o 
OBJS += dc_hardware.o libpspvram/valloc.o vm_file.o vmu.o
OBJS += primitive/polygon.o primitive/modifier.o primitive/prim_buffer.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

clean:
	$(RM) $(OBJS) $(TARGET)

partial:
	$(RM) main.o $(TARGET)
	make all

run: $(TARGET)
	dc-tool -x $< -i $(HOME)/Documents/dcdev/TR1-PSX.ISO

dump: $(TARGET)
	$(SH_ELF_BIN)/sh-elf-objdump -D $< > dump

distclean:
	$(RM) *.ISO *.CDI *.BIN *.elf *.bin *.iso ip.txt cd/1ST_READ.BIN

dist: 1ST_READ.BIN IP.BIN

1ST_READ.BIN: demo.bin
	scramble $< $@

ip.txt: ip.txt.in
	 sed -e 's/[@]DATE[@]/$(shell date '+%Y%m%d')/' < $< > $@ 

demo.bin: $(TARGET)
	$(SH_ELF_BIN)/sh-elf-objcopy -S -R .stack -O binary $< $@

IP.BIN: ip.txt
	makeip $< $@

dcdist: dist
	cp 1ST_READ.BIN $(HOME)/Documents/dcdev/dcdist/
	mkisofs -C 0,11702 -G IP.BIN -l -o TMP.ISO $(HOME)/Documents/dcdev/dcdist
	cdi4dc TMP.ISO TEST.CDI

cdi: dist
	cp 1ST_READ.BIN cd/
	mkisofs -C 0,11702 -G IP.BIN -l -o TMP.ISO cd
	cdi4dc TMP.ISO 'OpenLara_dc_$(shell date '+%Y%m%d').CDI'

rel:
	zip 'OpenLara_dc_$(shell date '+%Y%m%d').CDI' OpenLara_dc.zip

demul:
	/mnt/c/opt/demul/demul.exe -run=dc -image=TEST.CDI

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
