#ifndef H_VIDEO
#define H_VIDEO

#include "utils.h"
#include "texture.h"
#include "sound.h"

#ifdef _OS_TNS
    #define NO_VIDEO
#endif

struct AC_ENTRY {
    uint8 code;
    uint8 skip;
    uint8 ac;
    uint8 length;
};

// ISO 13818-2 table B-14
static const AC_ENTRY STR_AC[] = {
// signBit = (8 + shift) - length
// AC_LUT_1 (shift = 1)
    { 0XC0 , 1  , 1  , 4  }, // 11000000
    { 0X80 , 0  , 2  , 5  }, // 10000000
    { 0XA0 , 2  , 1  , 5  }, // 10100000
    { 0X50 , 0  , 3  , 6  }, // 01010000
    { 0X60 , 4  , 1  , 6  }, // 01100000
    { 0X70 , 3  , 1  , 6  }, // 01110000
    { 0X20 , 7  , 1  , 7  }, // 00100000
    { 0X28 , 6  , 1  , 7  }, // 00101000
    { 0X30 , 1  , 2  , 7  }, // 00110000
    { 0X38 , 5  , 1  , 7  }, // 00111000
    { 0X10 , 2  , 2  , 8  }, // 00010000
    { 0X14 , 9  , 1  , 8  }, // 00010100
    { 0X18 , 0  , 4  , 8  }, // 00011000
    { 0X1C , 8  , 1  , 8  }, // 00011100
    { 0X40 , 13 , 1  , 9  }, // 01000000
    { 0X42 , 0  , 6  , 9  }, // 01000010
    { 0X44 , 12 , 1  , 9  }, // 01000100
    { 0X46 , 11 , 1  , 9  }, // 01000110
    { 0X48 , 3  , 2  , 9  }, // 01001000
    { 0X4A , 1  , 3  , 9  }, // 01001010
    { 0X4C , 0  , 5  , 9  }, // 01001100
    { 0X4E , 10 , 1  , 9  }, // 01001110
// AC_LUT_6 (shift = 6)
    { 0X80 , 16 , 1  , 11 }, // 10000000
    { 0X90 , 5  , 2  , 11 }, // 10010000
    { 0XA0 , 0  , 7  , 11 }, // 10100000
    { 0XB0 , 2  , 3  , 11 }, // 10110000
    { 0XC0 , 1  , 4  , 11 }, // 11000000
    { 0XD0 , 15 , 1  , 11 }, // 11010000
    { 0XE0 , 14 , 1  , 11 }, // 11100000
    { 0XF0 , 4  , 2  , 11 }, // 11110000
    { 0X40 , 0  , 11 , 13 }, // 01000000
    { 0X44 , 8  , 2  , 13 }, // 01000100
    { 0X48 , 4  , 3  , 13 }, // 01001000
    { 0X4C , 0  , 10 , 13 }, // 01001100
    { 0X50 , 2  , 4  , 13 }, // 01010000
    { 0X54 , 7  , 2  , 13 }, // 01010100
    { 0X58 , 21 , 1  , 13 }, // 01011000
    { 0X5C , 20 , 1  , 13 }, // 01011100
    { 0X60 , 0  , 9  , 13 }, // 01100000
    { 0X64 , 19 , 1  , 13 }, // 01100100
    { 0X68 , 18 , 1  , 13 }, // 01101000
    { 0X6C , 1  , 5  , 13 }, // 01101100
    { 0X70 , 3  , 3  , 13 }, // 01110000
    { 0X74 , 0  , 8  , 13 }, // 01110100
    { 0X78 , 6  , 2  , 13 }, // 01111000
    { 0X7C , 17 , 1  , 13 }, // 01111100
    { 0X20 , 10 , 2  , 14 }, // 00100000
    { 0X22 , 9  , 2  , 14 }, // 00100010
    { 0X24 , 5  , 3  , 14 }, // 00100100
    { 0X26 , 3  , 4  , 14 }, // 00100110
    { 0X28 , 2  , 5  , 14 }, // 00101000
    { 0X2A , 1  , 7  , 14 }, // 00101010
    { 0X2C , 1  , 6  , 14 }, // 00101100
    { 0X2E , 0  , 15 , 14 }, // 00101110
    { 0X30 , 0  , 14 , 14 }, // 00110000
    { 0X32 , 0  , 13 , 14 }, // 00110010
    { 0X34 , 0  , 12 , 14 }, // 00110100
    { 0X36 , 26 , 1  , 14 }, // 00110110
    { 0X38 , 25 , 1  , 14 }, // 00111000
    { 0X3A , 24 , 1  , 14 }, // 00111010
    { 0X3C , 23 , 1  , 14 }, // 00111100
    { 0X3E , 22 , 1  , 14 }, // 00111110
// AC_LUT_9 (shift = 9)
    { 0X80 , 0  , 31 , 15 }, // 10000000
    { 0X88 , 0  , 30 , 15 }, // 10001000
    { 0X90 , 0  , 29 , 15 }, // 10010000
    { 0X98 , 0  , 28 , 15 }, // 10011000
    { 0XA0 , 0  , 27 , 15 }, // 10100000
    { 0XA8 , 0  , 26 , 15 }, // 10101000
    { 0XB0 , 0  , 25 , 15 }, // 10110000
    { 0XB8 , 0  , 24 , 15 }, // 10111000
    { 0XC0 , 0  , 23 , 15 }, // 11000000
    { 0XC8 , 0  , 22 , 15 }, // 11001000
    { 0XD0 , 0  , 21 , 15 }, // 11010000
    { 0XD8 , 0  , 20 , 15 }, // 11011000
    { 0XE0 , 0  , 19 , 15 }, // 11100000
    { 0XE8 , 0  , 18 , 15 }, // 11101000
    { 0XF0 , 0  , 17 , 15 }, // 11110000
    { 0XF8 , 0  , 16 , 15 }, // 11111000
    { 0X40 , 0  , 40 , 16 }, // 01000000
    { 0X44 , 0  , 39 , 16 }, // 01000100
    { 0X48 , 0  , 38 , 16 }, // 01001000
    { 0X4C , 0  , 37 , 16 }, // 01001100
    { 0X50 , 0  , 36 , 16 }, // 01010000
    { 0X54 , 0  , 35 , 16 }, // 01010100
    { 0X58 , 0  , 34 , 16 }, // 01011000
    { 0X5C , 0  , 33 , 16 }, // 01011100
    { 0X60 , 0  , 32 , 16 }, // 01100000
    { 0X64 , 1  , 14 , 16 }, // 01100100
    { 0X68 , 1  , 13 , 16 }, // 01101000
    { 0X6C , 1  , 12 , 16 }, // 01101100
    { 0X70 , 1  , 11 , 16 }, // 01110000
    { 0X74 , 1  , 10 , 16 }, // 01110100
    { 0X78 , 1  , 9  , 16 }, // 01111000
    { 0X7C , 1  , 8  , 16 }, // 01111100
    { 0X20 , 1  , 18 , 17 }, // 00100000
    { 0X22 , 1  , 17 , 17 }, // 00100010
    { 0X24 , 1  , 16 , 17 }, // 00100100
    { 0X26 , 1  , 15 , 17 }, // 00100110
    { 0X28 , 6  , 3  , 17 }, // 00101000
    { 0X2A , 16 , 2  , 17 }, // 00101010
    { 0X2C , 15 , 2  , 17 }, // 00101100
    { 0X2E , 14 , 2  , 17 }, // 00101110
    { 0X30 , 13 , 2  , 17 }, // 00110000
    { 0X32 , 12 , 2  , 17 }, // 00110010
    { 0X34 , 11 , 2  , 17 }, // 00110100
    { 0X36 , 31 , 1  , 17 }, // 00110110
    { 0X38 , 30 , 1  , 17 }, // 00111000
    { 0X3A , 29 , 1  , 17 }, // 00111010
    { 0X3C , 28 , 1  , 17 }, // 00111100
    { 0X3E , 27 , 1  , 17 }, // 00111110
};

static const uint8 STR_ZSCAN[] = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63,
};

static const int32 STR_QTABLE[] = {
    0x00020000, 0x00163150, 0x00163150, 0x0018D321, 0x001EC830, 0x0018D321, 0x0019DE84, 0x0027DEA0,
    0x0027DEA0, 0x0019DE84, 0x00160000, 0x0023E1B0, 0x002C6282, 0x002724C0, 0x001A0000, 0x001536B1,
    0x00257337, 0x00297B55, 0x0027F206, 0x00241022, 0x00146D8E, 0x000E1238, 0x001D6CAF, 0x002346F9,
    0x00255528, 0x0025E3EF, 0x001F9AA9, 0x000FB1DC, 0x00096162, 0x001985B6, 0x0022E73A, 0x002219AE,
    0x002219AE, 0x001DC539, 0x00144489, 0x000772FB, 0x000B1918, 0x00148191, 0x001D9060, 0x00200000,
    0x001F6966, 0x00180AAA, 0x000E28D8, 0x000DB2B0, 0x00178BD2, 0x001B7FC9, 0x001B7FC9, 0x0015A314,
    0x000C9DD8, 0x000C53EE, 0x001490C8, 0x0018B140, 0x0015A5E0, 0x000CFA08, 0x000D3E30, 0x00146910,
    0x00138F5A, 0x000CB0EE, 0x000C2390, 0x001066E8, 0x000C928C, 0x000A4DA2, 0x000A4DA2, 0x00065187,
};

struct Video {

    struct Decoder : Sound::Decoder {
        int width, height, fps;

        Decoder(Stream *stream) : Sound::Decoder(stream, 2, 0) {}
        virtual ~Decoder() { /* delete stream; */ }
        virtual bool decodeVideo(Color32 *pixels) { return false; }
    };

    // based on ffmpeg https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/ implementation of escape codecs
    struct Escape : Decoder {
        int vfmt, bpp;
        int sfmt, rate, channels, bps;
        int framesCount, chunksCount, offset;
        int curVideoPos, curVideoChunk;
        int curAudioPos, curAudioChunk;

        Sound::Decoder *audioDecoder;

        uint8 *prevFrame, *nextFrame, *lumaFrame;

        struct Chunk {
            int32 offset;
            int32 videoSize;
            int32 audioSize;
            uint8 *data;
        } *chunks;

        union MacroBlock {
            uint32 pixels[4];
        };

        union SuperBlock {
            uint32 pixels[64];
        };

        struct Codebook {
            uint32      size;
            uint32      depth;
            MacroBlock  *blocks;
        } codebook[3];

        Escape(Stream *stream) : Decoder(stream), audioDecoder(NULL), prevFrame(NULL), nextFrame(NULL), lumaFrame(NULL), chunks(NULL) {
            for (int i = 0; i < 4; i++)
                skipLine();

            vfmt        = readValue();      // video format
            width       = readValue();      // x size in pixels
            height      = readValue();      // y size in pixels
            bpp         = readValue();      // bits per pixel RGB
            fps         = readValue();      // frames per second
            sfmt        = readValue();      // sound format
            rate        = readValue();      // Hz Samples
            channels    = readValue();      // channel
            bps         = readValue();      // bits per sample (LINEAR UNSIGNED)
            framesCount = readValue();      // frames per chunk
            chunksCount = readValue() + 1;  // number of chunks
            skipLine();                     // even chunk size
            skipLine();                     // odd chunk size
            offset      = readValue();      // offset to chunk cat
            skipLine();                     // offset to sprite
            skipLine();                     // size of sprite
            skipLine();                     // offset to key frames

            stream->setPos(offset);

            chunks = new Chunk[chunksCount];
            for (int i = 0; i < chunksCount; i++) {
                chunks[i].offset    = readValue();
                chunks[i].videoSize = readValue();
                chunks[i].audioSize = readValue();
                chunks[i].data      = NULL;
            }

            switch (vfmt) {
                case 124 :
                    prevFrame = new uint8[width * height * 4];
                    nextFrame = new uint8[width * height * 4];
                    memset(prevFrame, 0, width * height * sizeof(uint32));
                    memset(nextFrame, 0, width * height * sizeof(uint32));
                    break;
                case 130 :
                    // Y[w*h], Cb[w*h/4], Cr[w*h/4], F[w*h/4]
                    prevFrame = new uint8[width * height * 7 / 4];
                    nextFrame = new uint8[width * height * 7 / 4];
                    lumaFrame = new uint8[width * height / 4];
                    memset(prevFrame, 0, width * height);
                    memset(prevFrame + width * height, 16, width * height / 2);
                    break;
                default  :
                    LOG("! unsupported Escape codec version (%d)\n", vfmt);
                    ASSERT(false);
            }

            codebook[0].blocks =
            codebook[1].blocks =
            codebook[2].blocks = NULL;

            curVideoPos   = curAudioPos   = 0;
            curVideoChunk = curAudioChunk = 0;

            nextChunk(0, 0);

        #ifdef NO_SOUND
            audioDecoder = NULL;
        #else
            if (sfmt == 1)
                audioDecoder = new Sound::PCM(NULL, channels, rate, 0x7FFFFF, bps);      // TR2
            else if (sfmt == 101) {
                if (bps == 8)
                    audioDecoder = new Sound::PCM(NULL, channels, rate, 0x7FFFFF, bps);  // TR1
                else
                    audioDecoder = new Sound::IMA(NULL, channels, rate);          // TR3
            }
        #endif
        }

        virtual ~Escape() {
            {
                OS_LOCK(Sound::lock);
                audioDecoder->stream = NULL;
                delete audioDecoder;
            }
            for (int i = 0; i < chunksCount; i++)
                delete[] chunks[i].data;
            delete[] chunks;
            delete[] codebook[0].blocks;
            delete[] codebook[1].blocks;
            delete[] codebook[2].blocks;
            delete[] prevFrame;
            delete[] nextFrame;
            delete[] lumaFrame;
        }

        void skipLine() {
            char c;
            while (stream->read(c) != '\n');
        }

        int readValue() {
            char buf[255];
            for (uint32 i = 0; i < sizeof(buf); i++) {
                char &c = buf[i];
                stream->read(c);
                if (c == ' ' || c == '.' || c == ',' || c == ';' || c == '\n') {
                    if (c == ' ' || c == '.')
                        skipLine();
                    c = '\0';
                    return atoi(buf);
                }
            }
            ASSERT(false);
            return 0;
        }

        int getSkip124(BitStream &bs) {
            int value;

            if ((value  = bs.readBit()) != 1 ||
                (value += bs.read(3)) != 8 ||
                (value += bs.read(7)) != 135)
                return value;

            return value + bs.read(12);
        }

        int getSkip130(BitStream &bs) {
            int value;

            if ((value = bs.readBit())) return 0;
            if ((value = bs.read(3)))   return value;
            if ((value = bs.read(8)))   return value + 7;
            if ((value = bs.read(15)))  return value + 262;

            return -1;
        }

        void copySuperBlock(uint32 *dst, int dstWidth, uint32 *src, int srcWidth) {
            for (int i = 0; i < 8; i++) {
                memcpy(dst, src, 8 * sizeof(uint32));
                src += srcWidth;
                dst += dstWidth;
            }
        }

        void decodeMacroBlock(BitStream &bs, MacroBlock &mb, int &cbIndex, int sbIndex) {
            int value = bs.readBit();
            if (value) {
                static const int8 trans[3][2] = { {2, 1}, {0, 2}, {1, 0} };
                value = bs.readBit();
                cbIndex = trans[cbIndex][value];
            }

            Codebook &cb = codebook[cbIndex];
            uint32 bIndex = bs.read(cb.depth);

            if (cbIndex == 1)
                bIndex += sbIndex << cb.depth;

            memcpy(&mb, cb.blocks + bIndex, sizeof(mb));
        }

        void insertMacroBlock(SuperBlock &sb, const MacroBlock &mb, int index) {
            uint32 *dst = sb.pixels + (index + (index & -4)) * 2;
            dst[0] = mb.pixels[0];
            dst[1] = mb.pixels[1];
            dst[8] = mb.pixels[2];
            dst[9] = mb.pixels[3];
        }

        void nextChunk(int from, int to) {
            OS_LOCK(Sound::lock);

            if (from < curVideoChunk && from < curAudioChunk) {
                delete[] chunks[from].data;
                chunks[from].data = NULL;
            }

            Chunk &chunk = chunks[to];
            if (chunk.data)
                return;
            chunk.data = new uint8[chunk.videoSize + chunk.audioSize];
            stream->setPos(chunk.offset);
            stream->raw(chunk.data, chunk.videoSize + chunk.audioSize);
        }

        virtual bool decodeVideo(Color32 *pixels) {
            if (curVideoChunk >= chunksCount)
                return false;

            if (curVideoPos >= chunks[curVideoChunk].videoSize) {
                curVideoChunk++;
                curVideoPos = 0;
                if (curVideoChunk >= chunksCount)
                    return false;

                nextChunk(curVideoChunk - 1, curVideoChunk);
            }

            uint8 *data = chunks[curVideoChunk].data + curVideoPos;

            switch (vfmt) {
                case 124 : return decode124(data, pixels);
                case 130 : return decode130(data, pixels);
                default  : ASSERT(false);
            }

            return false;
        }

        bool decode124(uint8 *data, Color32 *pixels) {
            uint32 flags, size;
            memcpy(&flags, data + 0, 4);
            memcpy(&size,  data + 4, 4);
            data += 8;

            curVideoPos += size;

        // skip unchanged frame
            if (!(flags & 0x114) || !(flags & 0x7800000))
                return true;

            int sbCount = (width / 8) * (height / 8);

        // read data into bit stream
            size -= (sizeof(flags) + sizeof(size));

            BitStream bs(data, size);

        // read codebook changes
            for (int i = 0; i < 3; i++) {
                if (flags & (1 << (17 + i))) {

                    Codebook &cb = codebook[i];

                    if (i == 2) {
                        cb.size  = bs.read(20);
                        cb.depth = log2i(cb.size - 1) + 1;
                    } else {
                        cb.depth = bs.read(4);
                        cb.size  = (i == 0 ? 1 : sbCount) << cb.depth;
                    }

                    delete[] cb.blocks;
                    cb.blocks = new MacroBlock[cb.size];

                    for (uint32 j = 0; j < cb.size; j++) {
                        uint8  mask = bs.read(4);
                        Color32 cA, cB;
                        cA.SetRGB15(bs.read(15));
                        cB.SetRGB15(bs.read(15));

                        if (cA.value != cB.value && (mask == 6 || mask == 9) && // check for 0101 or 1010 mask
                            abs(int(cA.r) - int(cB.r)) <= 8 &&
                            abs(int(cA.g) - int(cB.g)) <= 8 &&
                            abs(int(cA.b) - int(cB.b)) <= 8) {

                            cA.r = (int(cA.r) + int(cB.r)) / 2;
                            cA.g = (int(cA.g) + int(cB.g)) / 2;
                            cA.b = (int(cA.b) + int(cB.b)) / 2;

                            cB = cA;
                        }

                        for (int k = 0; k < 4; k++)
                            cb.blocks[j].pixels[k] = (mask & (1 << k)) ? cB.value : cA.value;
                    }
                }
            }

            static const uint16 maskMatrix[] = { 0x0001, 0x0002, 0x0010, 0x0020,
                                                 0x0004, 0x0008, 0x0040, 0x0080,
                                                 0x0100, 0x0200, 0x1000, 0x2000,
                                                 0x0400, 0x0800, 0x4000, 0x8000};

            SuperBlock sb;
            MacroBlock mb;
            int cbIndex = 1;

            int skip = -1;
            for (int sbIndex = 0; sbIndex < sbCount; sbIndex++) {
                int sbLine   = width / 8;
                int sbOffset = ((sbIndex / sbLine) * width + (sbIndex % sbLine)) * 8;
                uint32 *src = (uint32*)prevFrame + sbOffset;
                uint32 *dst = (uint32*)nextFrame + sbOffset;

                uint16 multiMask = 0;

                if (skip == -1)
                    skip = getSkip124(bs);

                if (skip) {
                    copySuperBlock(dst, width, src, width);
                } else {
                    copySuperBlock(sb.pixels, 8, src, width);

                    while (!bs.readBit()) {
                        decodeMacroBlock(bs, mb, cbIndex, sbIndex);
                        uint16 mask = bs.read(16);
                        multiMask |= mask;
                        for (int i = 0; i < 16; i++)
                            if (mask & maskMatrix[i])
                                insertMacroBlock(sb, mb, i);
                    }

                    if (!bs.readBit()) {
                        uint16 invMask = bs.read(4);
                        for (int i = 0; i < 4; i++)
                            multiMask ^= ((invMask & (1 << i)) ? 0x0F : bs.read(4)) << (i * 4);
                        for (int i = 0; i < 16; i++)
                            if (multiMask & maskMatrix[i]) {
                                decodeMacroBlock(bs, mb, cbIndex, sbIndex);
                                insertMacroBlock(sb, mb, i);
                            }
                    } else 
                        if (flags & (1 << 16))
                            while (!bs.readBit()) {
                                decodeMacroBlock(bs, mb, cbIndex, sbIndex);
                                insertMacroBlock(sb, mb, bs.read(4));
                            }

                    copySuperBlock(dst, width, sb.pixels, 8);
                }

                skip--;
            }

            memcpy(pixels, nextFrame, width * height * 4);
            swap(prevFrame, nextFrame);

            return true;
        }

        bool decode130(uint8 *data, Color32 *pixels) {

            static const uint8 offsetLUT[] = { 
                2, 4, 10, 20
            };

            static const int8 signLUT[64][4] = {
                {  0,  0,  0,  0 }, { -1,  1,  0,  0 }, {  1, -1,  0,  0 }, { -1,  0,  1,  0 },
                { -1,  1,  1,  0 }, {  0, -1,  1,  0 }, {  1, -1,  1,  0 }, { -1, -1,  1,  0 },
                {  1,  0, -1,  0 }, {  0,  1, -1,  0 }, {  1,  1, -1,  0 }, { -1,  1, -1,  0 },
                {  1, -1, -1,  0 }, { -1,  0,  0,  1 }, { -1,  1,  0,  1 }, {  0, -1,  0,  1 },
                {  0,  0,  0,  0 }, {  1, -1,  0,  1 }, { -1, -1,  0,  1 }, { -1,  0,  1,  1 },
                { -1,  1,  1,  1 }, {  0, -1,  1,  1 }, {  1, -1,  1,  1 }, { -1, -1,  1,  1 },
                {  0,  0, -1,  1 }, {  1,  0, -1,  1 }, { -1,  0, -1,  1 }, {  0,  1, -1,  1 },
                {  1,  1, -1,  1 }, { -1,  1, -1,  1 }, {  0, -1, -1,  1 }, {  1, -1, -1,  1 },
                {  0,  0,  0,  0 }, { -1, -1, -1,  1 }, {  1,  0,  0, -1 }, {  0,  1,  0, -1 },
                {  1,  1,  0, -1 }, { -1,  1,  0, -1 }, {  1, -1,  0, -1 }, {  0,  0,  1, -1 },
                {  1,  0,  1, -1 }, { -1,  0,  1, -1 }, {  0,  1,  1, -1 }, {  1,  1,  1, -1 },
                { -1,  1,  1, -1 }, {  0, -1,  1, -1 }, {  1, -1,  1, -1 }, { -1, -1,  1, -1 },
                {  0,  0,  0,  0 }, {  1,  0, -1, -1 }, {  0,  1, -1, -1 }, {  1,  1, -1, -1 },
                { -1,  1, -1, -1 }, {  1, -1, -1, -1 }, {  0,  0,  0,  0 }, {  0,  0,  0,  0 },
                {  0,  0,  0,  0 }, {  0,  0,  0,  0 }, {  0,  0,  0,  0 }, {  0,  0,  0,  0 },
                {  0,  0,  0,  0 }, {  0,  0,  0,  0 }, {  0,  0,  0,  0 }, {  0,  0,  0,  0 },
            };

            static const int8 lumaLUT[] = {
                -4, -3, -2, -1, 1, 2, 3, 4
            };

            static const int8 chromaLUT[2][8] = {
                { 1, 1, 0, -1, -1, -1,  0,  1 },
                { 0, 1, 1,  1,  0, -1, -1, -1 }
            };

            static const uint8 chromaValueLUT[] = {
                 20,  28,  36,  44,  52,  60,  68,  76,
                 84,  92, 100, 106, 112, 116, 120, 124,
                128, 132, 136, 140, 144, 150, 156, 164,
                172, 180, 188, 196, 204, 212, 220, 228
            };

            Chunk &chunk = chunks[curVideoChunk];
            curVideoPos = chunk.videoSize;

            BitStream bs(data, chunk.videoSize);
            bs.data += 16; // skip 16 bytes (frame size, version, gamma/linear chroma flags etc.)

            uint8 *lumaPtr = lumaFrame;

            int skip = -1;
            int bCount = width * height / 4;
            uint32 luma = 0, Y[4] = { 0 }, U = 16, V = 16, F = 0;

            uint8 *oY = prevFrame, *oU = oY + width * height, *oV = oU + width * height / 4, *oF = oV + width * height / 4;
            uint8 *nY = nextFrame, *nU = nY + width * height, *nV = nU + width * height / 4, *nF = nV + width * height / 4;

            for (int bIndex = 0; bIndex < bCount; bIndex++) {
                if (skip == -1)
                    skip = getSkip130(bs);

                if (skip) {
                    Y[0] = oY[0];
                    Y[1] = oY[1];
                    Y[2] = oY[width];
                    Y[3] = oY[width + 1];
                    U    = oU[0];
                    V    = oV[0];
                    F    = oF[0];
                    luma = *lumaPtr;
                } else {
                    if (bs.readBit()) {
                        uint32 sign = bs.read(6);
                        uint32 diff = bs.read(2);

                        luma = bs.read(5) * 2;

                        for (int i = 0; i < 4; i++)
                            Y[i] = clamp(luma + offsetLUT[diff] * signLUT[sign][i], 0U, 63U);

                        F = 1;
                    } else {

                        if (bs.readBit())
                            luma = bs.readBit() ? bs.read(6) : ((luma + lumaLUT[bs.read(3)]) & 63);

                        for (int i = 0; i < 4; i++)
                            Y[i] = luma;

                        F = 0;
                    }

                    if (bs.readBit()) {
                        if (bs.readBit()) {
                            U = bs.read(5);
                            V = bs.read(5);
                        } else {
                            uint32 idx = bs.read(3);
                            U = (U + chromaLUT[0][idx]) & 31;
                            V = (V + chromaLUT[1][idx]) & 31;
                        }
                    }
                }
                *lumaPtr++ = luma;

                nY[0]         = Y[0];
                nY[1]         = Y[1];
                nY[width]     = Y[2];
                nY[width + 1] = Y[3];
                nU[0]         = U;
                nV[0]         = V;
                nF[0]         = F;

                nY += 2; nU++; nV++; nF++;
                oY += 2; oU++; oV++; oF++;

                if (!(((bIndex + 1) * 2) % width)) {
                    nY += width;
                    oY += width;
                }

                skip--;
            }

            nY = nextFrame;
            nU = nY + width * height;
            nV = nU + width * height / 4;
            nF = nV + width * height / 4;

            for (int y = 0; y < height / 2; y++) {
                for (int x = 0; x < width / 2; x++) {
                    int i = (y * width + x) * 2;
                    
                    Color32::YCbCr_T871_420(nY[i] << 2, nY[i + 1] << 2, nY[i + width] << 2, nY[i + width + 1] << 2, 
                                            chromaValueLUT[*nU] - 128, chromaValueLUT[*nV] - 128, *nF * 4, 
                                            pixels[i], pixels[i + 1], pixels[i + width], pixels[i + width + 1]);

                    nU++;
                    nV++;
                    nF++;
                }
            }

            swap(prevFrame, nextFrame);

            return true;
        }

        virtual int decode(Sound::Frame *frames, int count) {
        #ifdef NO_VIDEO
            return 0;
        #else
            if (!audioDecoder) return 0;

            if (bps != 4 && abs(curAudioChunk - curVideoChunk) > 1) { // sync with video chunk, doesn't work for IMA
                nextChunk(curAudioChunk, curVideoChunk);
                curAudioChunk = curVideoChunk;
                curAudioPos   = 0;
            }

            int i = 0;
            while (i < count) {
                if (curAudioChunk >= chunksCount) {
                    memset(&frames[i], 0, sizeof(Sound::Frame) * (count - i));
                    break;
                }

                Chunk *chunk = &chunks[curAudioChunk];

                if (curAudioPos >= chunk->audioSize) {
                    curAudioPos = 0;
                    curAudioChunk++;
                    nextChunk(curAudioChunk - 1, curAudioChunk);
                    continue;
                }

                int part = min(count - i, (chunk->audioSize - curAudioPos) / (channels * bps / 8));

                Stream *memStream = new Stream(NULL, chunk->data + chunk->videoSize + curAudioPos, chunk->audioSize - curAudioPos);
                audioDecoder->stream = memStream;

                while (part > 0) { 
                    int res = audioDecoder->decode(&frames[i], part);
                    i += res;
                    part -= res;
                }
                curAudioPos += memStream->pos;

                delete memStream;
            }

            return count;
        #endif
        }
    };

    // based on https://raw.githubusercontent.com/m35/jpsxdec/readme/jpsxdec/PlayStation1_STR_format.txt
    struct STR : Decoder {

        enum {
            MAGIC_STR         = 0x80010160,

            VIDEO_SECTOR_SIZE = 2016,
            VIDEO_SECTOR_MAX  = 16,
            AUDIO_SECTOR_SIZE = (16 + 112) * 18, // XA ADPCM data block size

            MAX_CHUNKS        = 4,
        };

        struct SyncHeader {
            uint32 sync[3];
            uint8  mins, secs, block, mode;
            uint8  interleaved;
            uint8  channel;
            struct {
                uint8 isEnd:1, isVideo:1, isAudio:1, isData:1, trigger:1, form:1, realtime:1, eof:1;
            } submode;
            struct {
                uint8 stereo:1, :1, rate:1, :1, bps:1, :3;
            } coding;
            uint32 dup;
        };

        struct Sector {
            uint32 magic;
            uint16 chunkIndex;
            uint16 chunksCount;
            uint32 frameIndex;
            uint32 chunkSize;
            uint16 width;
            uint16 height;
            uint16 blocks;
            uint16 unk3800;
            uint16 qscale;
            uint16 version;
            uint32 unk00000000;
        };

        struct VideoChunk {
            int    size;
            uint16 width;
            uint16 height;
            uint32 qscale;
            uint8  data[VIDEO_SECTOR_SIZE * VIDEO_SECTOR_MAX];
        };

        struct AudioChunk {
            int    size;
            uint8  data[AUDIO_SECTOR_SIZE];
        };

        uint8 AC_LUT_1[256];
        uint8 AC_LUT_6[256];
        uint8 AC_LUT_9[256];

        VideoChunk videoChunks[MAX_CHUNKS];
        AudioChunk audioChunks[MAX_CHUNKS];

        int   videoChunksCount;
        int   audioChunksCount;

        int   curVideoChunk;
        int   curAudioChunk;

        Sound::XA *audioDecoder;

        struct {
            uint8 code;
            uint8 length;
        } vlc[176];

        bool hasSyncHeader;

        STR(Stream *stream) : Decoder(stream), videoChunksCount(0), audioChunksCount(0), curVideoChunk(-1), curAudioChunk(-1), audioDecoder(NULL)
        {
            memset(videoChunks, 0, sizeof(videoChunks));
            memset(audioChunks, 0, sizeof(audioChunks));

            if (stream->pos >= stream->size)
            {
                LOG("Can't load STR format \"%s\"\n", stream->name);
                ASSERT(false);
                return;
            }

            memset(AC_LUT_1, 255, sizeof(AC_LUT_1));
            memset(AC_LUT_6, 255, sizeof(AC_LUT_6));
            memset(AC_LUT_9, 255, sizeof(AC_LUT_9));

            buildLUT(AC_LUT_1,  0,  22, 1);
            buildLUT(AC_LUT_6, 22,  62, 6);
            buildLUT(AC_LUT_9, 62, 110, 9);

            uint32 syncMagic[3];
            stream->raw(syncMagic, sizeof(syncMagic));
            stream->seek(-(int)sizeof(syncMagic));

            hasSyncHeader = syncMagic[0] == 0xFFFFFF00 && syncMagic[1] == 0xFFFFFFFF && syncMagic[2] == 0x00FFFFFF;
            
            if (!hasSyncHeader)
            {
                LOG("! No sync header found, please use jpsxdec tool to extract FMVs\n");
            }

            nextChunk();

            VideoChunk &chunk = videoChunks[0];
            width    = (chunk.width  + 15) / 16 * 16;
            height   = (chunk.height + 15) / 16 * 16;
            fps      = 150 / (chunk.size / VIDEO_SECTOR_SIZE);
            fps      = (fps < 20) ? 15 : 30;
            channels = 2;
            freq     = 37800;

        #ifdef NO_SOUND
            audioDecoder = NULL;
        #else
            audioDecoder = new Sound::XA(this, audioNextBlockCallback);
        #endif
        }

        virtual ~STR()
        {
            OS_LOCK(Sound::lock);
            audioDecoder->stream = NULL;
            delete audioDecoder;
        }

        void buildLUT(uint8 *LUT, int start, int end, int shift)
        {
            for (int i = start; i < end; i++)
            {
                const AC_ENTRY &e = STR_AC[i];
                uint8 trash = (1 << (8 + shift - e.length + 1));
            // fill the value and all possible endings
                while (trash--)
                {
                    LUT[e.code | trash] = i;
                }
            }
        }

        bool nextChunk()
        {
            OS_LOCK(Sound::lock);

            if (videoChunks[videoChunksCount % MAX_CHUNKS].size > 0)
            {
                return false;
            }

            if (stream->pos >= stream->size)
            {
                return false;
            }

            bool readingVideo = false;

            while (stream->pos < stream->size)
            {
                if (hasSyncHeader)
                {
                    stream->seek(24);
                }

                Sector sector;
                stream->raw(&sector, sizeof(Sector));

                if (sector.magic == MAGIC_STR)
                {
                    VideoChunk *chunk = videoChunks + (videoChunksCount % MAX_CHUNKS);

                    if (sector.chunkIndex == 0)
                    {
                        readingVideo  = true;
                        chunk->size   = 0;
                        chunk->width  = sector.width;
                        chunk->height = sector.height;
                        chunk->qscale = sector.qscale;
                    }

                    ASSERT(chunk->size + VIDEO_SECTOR_SIZE < sizeof(chunk->data));
                    stream->raw(chunk->data + chunk->size, VIDEO_SECTOR_SIZE);
                    chunk->size += VIDEO_SECTOR_SIZE;

                    if (hasSyncHeader)
                    {
                        stream->seek(280);
                    }

                    if (sector.chunkIndex == sector.chunksCount - 1)
                    {
                        videoChunksCount++;
                        return true;
                    }

                } else {
                    AudioChunk *chunk = audioChunks + (audioChunksCount++ % MAX_CHUNKS);

                    memcpy(chunk->data, &sector, sizeof(sector)); // audio chunk has no sector header (just XA data)
                    stream->raw(chunk->data + sizeof(sector), AUDIO_SECTOR_SIZE - sizeof(sector)); // !!! MUST BE 2304 !!! most of CD image tools copy only 2048 per sector, so "clicks" will be there
                    chunk->size = AUDIO_SECTOR_SIZE;
                    stream->seek(24);

                    if (!hasSyncHeader)
                    {
                        stream->seek(2048 - (AUDIO_SECTOR_SIZE + 24));
                    }

                    if (!readingVideo)
                    {
                        return true;
                    }
                };
            }
            return false;
        }

        // http://jpsxdec.blogspot.com/2011/06/decoding-mpeg-like-bitstreams.html
        bool readCode(BitStream &bs, int16 &skip, int16 &ac)
        {
            if (bs.readU(1))
            {
                if (bs.readU(1))
                {
                    skip = 0;
                    ac   = bs.readU(1) ? -1 : 1;
                    return true;
                }
                return false; // end of block
            }

            int nz = 1;
            while (!bs.readU(1))
            {
                nz++;
            }

            if (nz == 5) // escape code == 0b1000001
            {
                uint16 esc = bs.readU(16);
                skip = esc >> 10;
                ac   = esc & 0x3FF;
                if (ac & 0x200)
                    ac -= 0x400;
                return true;
            }

            uint8 *table, shift;
            if (nz < 6) {
                table = AC_LUT_1;
                shift = 1;
            } else if (nz < 9) {
                table = AC_LUT_6;
                shift = 6;
            } else {
                table = AC_LUT_9;
                shift = 9;
            }

            BitStream state = bs;
            uint32 code = (1 << 7) | state.readU(7);

            code >>= nz - shift;

            ASSERT(table);

            int index = table[code];

            ASSERT(index != 255);

            const AC_ENTRY &e = STR_AC[index];
            bs.skip(e.length - nz - 1);
            skip = e.skip;
            ac = (code & (1 << (8 + shift - e.length))) ? -e.ac : e.ac;
            return true;
        }

        // https://psx-spx.consoledev.net/macroblockdecodermdec/
        // https://github.com/grumpycoders/pcsx-redux/
        #define AAN_CONST_BITS 12
        #define AAN_PRESCALE_BITS 16

        #define AAN_EXTRA 12
        #define SCALE(x, n) ((x) >> (n))
        #define SCALER(x, n) (((x) + ((1 << (n)) >> 1)) >> (n))
        #define RLE_RUN(a) ((a) >> 10)
        #define RLE_VAL(a) (((int)(a) << (sizeof(int) * 8 - 10)) >> (sizeof(int) * 8 - 10))
        #define MULS(var, c) (SCALE((var) * (c), AAN_CONST_BITS))

        #define AAN_CONST_SIZE 24
        #define AAN_CONST_SCALE (AAN_CONST_SIZE - AAN_CONST_BITS)

        #define MULR(a) ((1434 * (a)))
        #define MULB(a) ((1807 * (a)))
        #define MULG2(a, b) ((-351 * (a)-728 * (b)))
        #define MULY(a) ((a) << 10)

        #define MAKERGB15(r, g, b, a) ((a | ((b) << 10) | ((g) << 5) | (r)))
        #define SCALE8(c) SCALER(c, 20)
        #define SCALE5(c) SCALER(c, 23)

        #define CLAMP5(c) (((c) < -16) ? 0 : (((c) > (31 - 16)) ? 31 : ((c) + 16)))
        #define CLAMP8(c) (((c) < -128) ? 0 : (((c) > (255 - 128)) ? 255 : ((c) + 128)))

        #define CLAMP_SCALE8(a) (CLAMP8(SCALE8(a)))
        #define CLAMP_SCALE5(a) (CLAMP5(SCALE5(a)))

        static inline void fillCol(int *blk, int val)
        {
            blk[0 * 8] = blk[1 * 8] = blk[2 * 8] = blk[3 * 8] = blk[4 * 8] = blk[5 * 8] = blk[6 * 8] = blk[7 * 8] = val;
        }

        static inline void fillRow(int *blk, int val)
        {
            blk[0] = blk[1] = blk[2] = blk[3] = blk[4] = blk[5] = blk[6] = blk[7] = val;
        }

        static void IDCT(int *block, int used_col)
        {
            #define FIX_1_082392200 4433
            #define FIX_1_414213562 5793
            #define FIX_1_847759065 7568
            #define FIX_2_613125930 10703

            int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
            int z5, z10, z11, z12, z13;
            int *ptr;
            int i;

            if (used_col == -1)
            {
                int v = block[0];
                for (i = 0; i < 64; i++)
                {
                    block[i] = v;
                }
                return;
            }

            ptr = block;
            for (i = 0; i < 8; i++, ptr++)
            {
                if ((used_col & (1 << i)) == 0)
                {
                    if (ptr[8 * 0])
                    {
                        fillCol(ptr, ptr[0]);
                        used_col |= (1 << i);
                    }
                    continue;
                }

                z10 = ptr[8 * 0] + ptr[8 * 4];
                z11 = ptr[8 * 0] - ptr[8 * 4];
                z13 = ptr[8 * 2] + ptr[8 * 6];
                z12 = MULS(ptr[8 * 2] - ptr[8 * 6], FIX_1_414213562) - z13;

                tmp0 = z10 + z13;
                tmp3 = z10 - z13;
                tmp1 = z11 + z12;
                tmp2 = z11 - z12;

                z13 = ptr[8 * 3] + ptr[8 * 5];
                z10 = ptr[8 * 3] - ptr[8 * 5];
                z11 = ptr[8 * 1] + ptr[8 * 7];
                z12 = ptr[8 * 1] - ptr[8 * 7];

                tmp7 = z11 + z13;

                z5 = (z12 - z10) * (FIX_1_847759065);
                tmp6 = SCALE(z10 * (FIX_2_613125930) + z5, AAN_CONST_BITS) - tmp7;
                tmp5 = MULS(z11 - z13, FIX_1_414213562) - tmp6;
                tmp4 = SCALE(z12 * (FIX_1_082392200)-z5, AAN_CONST_BITS) + tmp5;

                ptr[8 * 0] = (tmp0 + tmp7);
                ptr[8 * 7] = (tmp0 - tmp7);
                ptr[8 * 1] = (tmp1 + tmp6);
                ptr[8 * 6] = (tmp1 - tmp6);
                ptr[8 * 2] = (tmp2 + tmp5);
                ptr[8 * 5] = (tmp2 - tmp5);
                ptr[8 * 4] = (tmp3 + tmp4);
                ptr[8 * 3] = (tmp3 - tmp4);
            }

            ptr = block;
            if (used_col == 1)
            {
                for (i = 0; i < 8; i++) 
                {
                    fillRow(block + 8 * i, block[8 * i]);
                }
            } else {
                for (i = 0; i < 8; i++, ptr += 8)
                {
                    z10 = ptr[0] + ptr[4];
                    z11 = ptr[0] - ptr[4];
                    z13 = ptr[2] + ptr[6];
                    z12 = MULS(ptr[2] - ptr[6], FIX_1_414213562) - z13;

                    tmp0 = z10 + z13;
                    tmp3 = z10 - z13;
                    tmp1 = z11 + z12;
                    tmp2 = z11 - z12;

                    z13 = ptr[3] + ptr[5];
                    z10 = ptr[3] - ptr[5];
                    z11 = ptr[1] + ptr[7];
                    z12 = ptr[1] - ptr[7];

                    tmp7 = z11 + z13;
                    z5 = (z12 - z10) * FIX_1_847759065;
                    tmp6 = SCALE(z10 * FIX_2_613125930 + z5, AAN_CONST_BITS) - tmp7;
                    tmp5 = MULS(z11 - z13, FIX_1_414213562) - tmp6;
                    tmp4 = SCALE(z12 * FIX_1_082392200 - z5, AAN_CONST_BITS) + tmp5;

                    ptr[0] = tmp0 + tmp7;

                    ptr[7] = tmp0 - tmp7;
                    ptr[1] = tmp1 + tmp6;
                    ptr[6] = tmp1 - tmp6;
                    ptr[2] = tmp2 + tmp5;
                    ptr[5] = tmp2 - tmp5;
                    ptr[4] = tmp3 + tmp4;
                    ptr[3] = tmp3 - tmp4;
                }
            }
        }

        static inline void putQuadRGB24(uint8 *image, int *Yblk, int Cr, int Cb)
        {
            int Y, R, G, B;

            R = MULR(Cr);
            G = MULG2(Cb, Cr);
            B = MULB(Cb);

            Y = MULY(Yblk[0]);
            image[0 * 3 + 0] = CLAMP_SCALE8(Y + R);
            image[0 * 3 + 1] = CLAMP_SCALE8(Y + G);
            image[0 * 3 + 2] = CLAMP_SCALE8(Y + B);
            Y = MULY(Yblk[1]);
            image[1 * 3 + 0] = CLAMP_SCALE8(Y + R);
            image[1 * 3 + 1] = CLAMP_SCALE8(Y + G);
            image[1 * 3 + 2] = CLAMP_SCALE8(Y + B);
            Y = MULY(Yblk[8]);
            image[16 * 3 + 0] = CLAMP_SCALE8(Y + R);
            image[16 * 3 + 1] = CLAMP_SCALE8(Y + G);
            image[16 * 3 + 2] = CLAMP_SCALE8(Y + B);
            Y = MULY(Yblk[9]);
            image[17 * 3 + 0] = CLAMP_SCALE8(Y + R);
            image[17 * 3 + 1] = CLAMP_SCALE8(Y + G);
            image[17 * 3 + 2] = CLAMP_SCALE8(Y + B);
        }

        inline void YUV2RGB24(int32 *blk, uint8 *image)
        {
            int x, y;
            int *Yblk = blk + 64 * 2;
            int *Crblk = blk;
            int *Cbblk = blk + 64;

            for (y = 0; y < 16; y += 2, Crblk += 4, Cbblk += 4, Yblk += 8, image += 8 * 3 * 3)
            {
                if (y == 8) Yblk += 64;
                for (x = 0; x < 4; x++, image += 6, Crblk++, Cbblk++, Yblk += 2)
                {
                    putQuadRGB24(image, Yblk, *Crblk, *Cbblk);
                    putQuadRGB24(image + 8 * 3, Yblk + 64, *(Crblk + 4), *(Cbblk + 4));
                }
            }
        }

        virtual bool decodeVideo(Color32 *pixels)
        {
            curVideoChunk++;
            while (curVideoChunk >= videoChunksCount)
            {
                if (!nextChunk())
                {
                    return false;
                }
            }

            VideoChunk *chunk = videoChunks + (curVideoChunk % MAX_CHUNKS);

            BitStream bs(chunk->data + 8, chunk->size - 8); // make bitstream without frame header

            int32 qscale = chunk->qscale;

            int32 blocks[64 * 6]; // Cr, Cb, YTL, YTR, YBL, YBR
            for (int32 bX = 0; bX < width / 16; bX++)
            {
                for (int32 bY = 0; bY < height / 16; bY++)
                {
                    memset(blocks, 0, sizeof(blocks));

                    for (int i = 0; i < 6; i++)
                    {
                        int32* block = blocks + i * 64;

                        int16 dc = bs.readU(10);
                        if (dc & 0x200) {
                            dc -= 0x400;
                        }

                        block[0] = SCALER(dc * STR_QTABLE[0], AAN_EXTRA - 3);

                        int32 used_col = 0;

                        int16 skip, ac;
                        int32 index = 0;
                        while (readCode(bs, skip, ac))
                        {
                            index += skip + 1;
                            ASSERT(index < 64);
                            block[STR_ZSCAN[index]] = SCALER(ac * STR_QTABLE[index] * qscale, AAN_EXTRA);

                            used_col |= (STR_ZSCAN[index] > 7) ? 1 << (STR_ZSCAN[index] & 7) : 0;
                        }

                        if (index == 0) used_col = -1;

                        IDCT(block, used_col);
                    }

                    Color24 pix[16 * 16];
                    YUV2RGB24(blocks, (uint8*)pix);

                    int32 i = 0;
                    Color32 *blockPixels = pixels + (width * bY * 16 + bX * 16);
                    for (int y = 0; y < 16; y++)
                    {
                        for (int x = 0; x < 16; x++)
                        {
                            blockPixels[y * width + x] = pix[i++];
                        }
                    }
                }
            }

            chunk->size = 0;

            return true;
        }

        bool getNextAudioStream()
        {
            curAudioChunk++;
            while (curAudioChunk >= audioChunksCount)
            {
                if (!nextChunk())
                {
                    curAudioChunk--;
                    return false;
                }
            }

            AudioChunk *chunk = audioChunks + (curAudioChunk % MAX_CHUNKS);
            ASSERT(chunk->size > 0);
            audioDecoder->processSector(chunk->data);
            return true;
        }

        static bool audioNextBlockCallback(void* userData)
        {
            return ((STR*)userData)->getNextAudioStream();
        }

        virtual int decode(Sound::Frame *frames, int count)
        {
        #ifdef NO_VIDEO
            return 0;
        #else
            if (!audioDecoder) return 0;

            int ret = audioDecoder->decode(frames, count);
            if (ret < count) {
                memset(frames + ret, 0, (count - ret) * sizeof(Sound::Frame));
            }

            return count;
        #endif
        }
    };

// based on https://wiki.multimedia.cx/index.php/Sega_FILM
    struct Cinepak : Decoder {

        struct Chunk {
            int    offset;
            int    size;
            uint32 info[2];
        } *chunks;

        int chunksCount;
        int audioChunkIndex;
        int audioChunkPos;
        Array<Sound::Frame> audioChunkFrames;

        int videoChunkIndex;
        int videoChunkPos;
        Array<uint8> videoChunkData;

        Cinepak(Stream *stream) : Decoder(stream), chunks(NULL), audioChunkIndex(-1), audioChunkPos(0), videoChunkIndex(-1), videoChunkPos(0) {
            ASSERTV(stream->readLE32() == FOURCC("FILM"));
            int sampleOffset = stream->readBE32();
            stream->seek(4); // skip version 1.06
            stream->seek(4); // skip reserved
            ASSERTV(stream->readLE32() == FOURCC("FDSC"));
            ASSERTV(stream->readBE32() == 32);
            ASSERTV(stream->readLE32() == FOURCC("cvid"));
            height = stream->readBE32();
            width  = stream->readBE32();
            ASSERTV(stream->read() == 24);
            channels = stream->read();
            ASSERT(channels == 2);
            ASSERTV(stream->read() == 16);
            ASSERTV(stream->read() == 0);
            freq = stream->readBE16();
            ASSERT(freq == 22254);

            stream->seek(6);
            ASSERTV(stream->readLE32() == FOURCC("STAB"));
            stream->seek(4); // skip STAB length
            fps = stream->readBE32() / 2;
            chunksCount = stream->readBE32();
            chunks = new Chunk[chunksCount];
            for (int i = 0; i < chunksCount; i++) {
                Chunk &c = chunks[i];
                c.offset  = stream->readBE32() + sampleOffset;
                c.size    = stream->readBE32();
                c.info[0] = stream->readBE32();
                c.info[1] = stream->readBE32();
            }
        }

        virtual ~Cinepak() {
            delete[] chunks;  
        }

        virtual bool decodeVideo(Color32 *pixels) {
            if (audioChunkIndex >= chunksCount)
                return false;
            /*
            // TODO: sega cinepak film decoder
            // get next audio chunk
            if (videoChunkPos >= videoChunkData.length) {
                videoChunkPos = 0;

                while (++videoChunkIndex < chunksCount) {
                    if (chunks[videoChunkIndex].info[0] != 0xFFFFFFFF || chunks[videoChunkIndex].info[1] != 1)
                        break;
                }

                if (videoChunkIndex >= chunksCount)
                    return true;

                const Chunk &chunk = chunks[videoChunkIndex];

                {
                    OS_LOCK(Sound::lock);
                    stream->setPos(chunk.offset);
                    videoChunkData.resize(chunk.size);
                    stream->raw(videoChunkData.items, videoChunkData.length);
                }
            }

            // TODO: decode
            Stream data(NULL, videoChunkData.items + videoChunkPos, videoChunkData.length - videoChunkPos);
            union FrameHeader {
                struct { uint32 flags:8, size:24; };
                uint32 value;
            } hdr;

            hdr.value = data.readBE32();
            ASSERT(hdr.size <= videoChunkData.length - videoChunkPos);
            videoChunkPos += hdr.size;
            */
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++) {
                    Color32 c;
                    c.r = c.g = c.b = x ^ y;
                    c.a = 255;
                    pixels[y * width + x] = c;
                }
            
            return true;
        }

        virtual int decode(Sound::Frame *frames, int count) {
            if (audioChunkIndex >= chunksCount) {
                memset(frames, 0, count * sizeof(Sound::Frame));
                return count;
            }

            // get next audio chunk
            if (audioChunkPos >= audioChunkFrames.length) {
                audioChunkPos = 0;

                while (++audioChunkIndex < chunksCount) {
                    if (chunks[audioChunkIndex].info[0] == 0xFFFFFFFF)
                        break;
                }

                if (audioChunkIndex >= chunksCount) {
                    memset(frames, 0, count);
                    return count;
                }

                const Chunk &chunk = chunks[audioChunkIndex];

                audioChunkFrames.resize(chunk.size / sizeof(Sound::Frame));

                stream->setPos(chunk.offset);
                // read LEFT channel samples
                for (int i = 0; i < audioChunkFrames.length; i++)
                    audioChunkFrames[i].L = stream->readBE16();
                // read RIGHT channel samples
                for (int i = 0; i < audioChunkFrames.length; i++)
                    audioChunkFrames[i].R = stream->readBE16();
            }

            for (int i = 0; i < count; i += 2) {
                frames[i + 0] = audioChunkFrames[audioChunkPos];
                frames[i + 1] = audioChunkFrames[audioChunkPos++];

                if (audioChunkPos >= audioChunkFrames.length)
                    return i + 2;
            }

            return count;
        }
    };

    enum Format {
        PC,
        PSX,
        SAT,
    } format;

    Sound::Sample *sample;
    Decoder *decoder;
    Texture *frameTex[2];
    Color32 *frameData;
    float   step, stepTimer, time;
    bool    isPlaying;
    bool    needUpdate;

    static void playAsync(Stream *stream, void *userData) {
        if (stream) {
            Video *video = (Video*)userData;
            video->sample = Sound::play(stream, NULL, 1.0f, 1.0f, Sound::MUSIC);
            Core::resetTime();
        }
    }

    Video(Stream *stream, TR::LevelID id) : sample(NULL), decoder(NULL), stepTimer(0.0f), time(0.0f), isPlaying(false), needUpdate(false) {
        frameTex[0] = frameTex[1] = NULL;

        if (!stream) return;

        uint32 magic = stream->readLE32();
        stream->seek(-4);

        float pitch = 1.0f;

        if (magic == FOURCC("FILM")) {
            format  = SAT;
            decoder = new Cinepak(stream);
            pitch = decoder->freq / 22050.0f; // 22254 / 22050 = 1.00925
        } else if (magic == FOURCC("ARMo")) {
            format  = PC;
            decoder = new Escape(stream);
        } else {
            format  = PSX;
            decoder = new STR(stream);
        }

        frameData = new Color32[decoder->width * decoder->height];
        memset(frameData, 0, decoder->width * decoder->height * sizeof(Color32));

        for (int i = 0; i < 2; i++) {
            frameTex[i] = new Texture(decoder->width, decoder->height, 1, FMT_RGBA, OPT_DYNAMIC, frameData);
        }

        if (!TR::getVideoTrack(id, playAsync, this)) {
            sample = Sound::play(decoder);
            sample->pitch = pitch;
            if (sample) {
                sample->pitch = pitch;
            }
        }

        step      = 1.0f / decoder->fps;
        stepTimer = step;
        time      = 0.0f;
        isPlaying = true;
    }

    virtual ~Video() {
        OS_LOCK(Sound::lock);
        if (sample) {
            if (sample->decoder == decoder) {
                sample->decoder = NULL;
            }
            sample->stop();
        }
        delete decoder;
        delete frameTex[0];
        delete frameTex[1];
        delete[] frameData;
    }

    void update() {
        if (!isPlaying) return;

        stepTimer += Core::deltaTime;
        if (stepTimer < step)
            return;
        stepTimer -= step;
        time += step;
    #ifdef VIDEO_TEST
        int t = Core::getTime();
        while (decoder->decodeVideo(frameData)) {}
        LOG("time: %d\n", Core::getTime() - t);
        isPlaying = false;
    #else
        isPlaying = needUpdate = decoder->decodeVideo(frameData);
    #endif
    }

    void render() { // update GPU texture
        if (!needUpdate) return;
        frameTex[0]->update(frameData);
        swap(frameTex[0], frameTex[1]);
        needUpdate = false;
    }
};

#endif
