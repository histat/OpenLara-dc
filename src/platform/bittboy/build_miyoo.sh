/opt/miyoo-toolchain/bin/arm-miyoo-linux-uclibcgnueabi-g++ -std=c++11 -O3 -s -fno-unroll-loops -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,--gc-sections -D__MIYOO__ -DNDEBUG -D_POSIX_THREADS -D_POSIX_READER_WRITER_LOCKS main.cpp ../../libs/stb_vorbis/stb_vorbis.c ../../libs/minimp3/minimp3.cpp ../../libs/tinf/tinflate.c -I../../ -o../../../bin/OpenLara -lm -lpthread -lSDL -lasound