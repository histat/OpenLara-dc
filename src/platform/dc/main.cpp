#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "game.h"


#include <ronin/ronin.h>
#include <ronin/soundcommon.h>

#include "private.h"
#include "vm_file.h"

extern "C" {
  void dcExit();
  void *memcpy4s(void *s1, const void *s2, unsigned int n);
}


namespace DC_FLASH {

  int syscall_info_flash(int sect, int *info)
  {
    return (*(int (**)(int, void*, int, int))0x8c0000b8)(sect,info,0,0);  
  }

  int syscall_read_flash(int offs, void *buf, int cnt)
  {
    return (*(int (**)(int, void*, int, int))0x8c0000b8)(offs,buf,cnt,1);
  }

  int flash_crc(const char *buf, int size)
  {
    int i, c, n = -1;
    for(i=0; i<size; i++) {
      n ^= (buf[i]<<8);
      for(c=0; c<8; c++)
	if(n & 0x8000)
	  n = (n << 1) ^ 4129;
	else
	  n <<= 1;
    }
    return (unsigned short)~n;
  }

  int flash_read_sector(int partition, int sec, char *dst)
  {
    int s, r, n, b, bmb, got=0;
    int info[2];
    char buf[64];
    char bm[64];

    if((r = syscall_info_flash(partition, info))<0)
      return r;
    
    if((r = syscall_read_flash(info[0], buf, 64))<0)
      return r;

    if(memcmp(buf, "KATANA_FLASH", 12) ||
       buf[16] != partition || buf[17] != 0)
      return -2;

    n = (info[1]>>6)-1-((info[1] + 0x7fff)>>15);
    bmb = n+1;
    for(b = 0; b < n; b++) {
      if(!(b&511)) {
	if((r = syscall_read_flash(info[0] + (bmb++ << 6), bm, 64))<0)
	  return r;
      }
      if(!(bm[(b>>3)&63] & (0x80>>(b&7))))
	if((r = syscall_read_flash(info[0] + ((b+1) << 6), buf, 64))<0)
	  return r;
	else if((s=*(unsigned short *)(buf+0)) == sec &&
		flash_crc(buf, 62) == *(unsigned short *)(buf+62)) {
	  memcpy(dst+(s-sec)*60, buf+2, 60);
	  got=1;
	}
    }

    return got;
  }

  int GetLocalLanguage() {

    char data[60];
    if (flash_read_sector(2, 5, data) == 1)
      return data[5];
    else 
      return -1;
  }

}

namespace DC_PAD 
{
  enum {
    JOY_BTN_C		= 0x01,
    JOY_BTN_B		= 0x02,
    JOY_BTN_A		= 0x04,
    JOY_BTN_START	= 0x08,
    JOY_DPAD_UP		= 0x010,
    JOY_DPAD_DOWN	= 0x020,
    JOY_DPAD_LEFT	= 0x040,
    JOY_DPAD_RIGHT	= 0x080,
    JOY_BTN_Z		= 0x100,
    JOY_BTN_Y		= 0x200,
    JOY_BTN_X		= 0x400,
    JOY_BTN_D		= 0x800,
    JOY_LTRIGGER = 0x1000,
    JOY_RTRIGGER = 0x2000,
  };
  
}

namespace DC_MUTEX {

  struct Mutex {
    Mutex() {}
    ~Mutex() {}
  };
}

// multi-threading
void* osMutexInit() {
  //return new DC_MUTEX::Mutex();
  return NULL;
}

void osMutexFree(void *obj) {
  //DC_MUTEX::Mutex *o = (DC_MUTEX::Mutex*)obj; 
  //delete (DC_MUTEX::Mutex *)o;
}

void osMutexLock(void *obj) {
}

void osMutexUnlock(void *obj) {
}

// sound
#define SND_FRAME_SIZE  4
#define SND_FRAMES      1024

Sound::Frame     sndBuf[SND_FRAMES] __attribute__((aligned(2)));

static void write_samples(int samples, int length)
{
  int r = length - fillpos;
  int n = samples;

  if (fillpos+n > length) {
    memcpy4s(RING_BUF+fillpos, &sndBuf[0], SAMPLES_TO_BYTES(r));
    
    fillpos = 0;
    n -= r;
    memcpy4s(RING_BUF, &sndBuf[r], SAMPLES_TO_BYTES(n));
  } else {
    memcpy4s(RING_BUF+fillpos, &sndBuf[0], SAMPLES_TO_BYTES(n));
  }

  if ((fillpos += n) >= length)
    fillpos = 0;

}

static void sndUpdate()
{
  int ring_buffer_samples = read_sound_int(&SOUNDSTATUS->ring_length);
  int n = read_sound_int(&SOUNDSTATUS->samplepos);

  
  if ((n-=fillpos)<0)
    n += ring_buffer_samples;

  //n = n + fillpos + (ring_buffer_samples - fillpos)

  if (n < 100)
    return;

  //printf("playpos %ld ", n);
  Sound::fill(sndBuf, SND_FRAMES);

  write_samples(SND_FRAMES, ring_buffer_samples);
}

void sndInit() {
  stop_sound();
  do_sound_command(CMD_SET_BUFFER(3));
  do_sound_command(CMD_SET_STEREO(1));
  do_sound_command(CMD_SET_FREQ_EXP(FREQ_44100_EXP));

  memset(sndBuf, 0, SND_FRAMES * SND_FRAME_SIZE);

  fillpos = 0;

 if (read_sound_int(&SOUNDSTATUS->mode) != MODE_PLAY)
    start_sound();

}

void sndFree() {
  stop_sound();
}


// timing
int osStartTime = 0;

int osGetTimeMS() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return int((t.tv_sec - osStartTime) * 1000 + t.tv_usec / 1000);
}

//#define ENABLE_LANG
#define SAVE_SUPPORT

void osCacheWrite(Stream *stream) {
  LOG("cache stored: %s 0x%x\n", stream->name, stream->size);
  
  if (stream->callback)
    stream->callback(NULL, stream->userData);

  delete stream;
}

void osCacheRead(Stream *stream) {
  LOG("cache loaded: %s 0x%x\n", stream->name, stream->size);

  if (stream->callback)
    stream->callback(NULL, stream->userData);

  delete stream;
}

// memory card
void osReadSlot(Stream *stream) {
  LOG("read slot : %s\n", stream->name);

#ifdef SAVE_SUPPORT

  VMFILE *f;
  f = vm_fileopen(stream->name, "rb");
  
  if (f) {
    int size = (int)vm_fsize(f);
    char *data = new char[size];
    vm_fread(data, 1, size, f);
    vm_fclose(f);
    if (stream->callback)
      stream->callback(new Stream(stream->name, data, size), stream->userData);
    delete[] data;
    } else
        if (stream->callback)
            stream->callback(NULL, stream->userData);

    delete stream;

#else
    if (stream->callback)
      stream->callback(NULL, stream->userData);

  delete stream;
#endif
}

void osWriteSlot(Stream *stream) {
  LOG("write slot : %s\n", stream->name);

#ifdef SAVE_SUPPORT
  VMFILE *f;
  f = vm_fileopen(stream->name, "wb");
  
  if (f) {
    vm_fwrite(stream->data, 1, stream->size, f);
    vm_fclose(f);
    if (stream->callback)
      stream->callback(new Stream(stream->name, stream->data, stream->size), stream->userData);
    } else
        if (stream->callback)
            stream->callback(NULL, stream->userData);

    delete stream;
#else

    if (stream->callback)
      stream->callback(NULL, stream->userData);

  delete stream;
#endif
}

// Input
bool osJoyReady(int index) {
  return index == 0;
}

void osJoyVibrate(int index, float L, float R) {
    // TODO
}

void joyInit() {
}

void joyUpdate() {

  struct timeval tm;

  static unsigned int tick = 0;
  unsigned int t = Timer();
  if ((t - tick) < 0) {
    return;
  }

  tick += USEC_TO_TIMER(17000);
  if ((t - tick) >= 0) {
    tick += t+USEC_TO_TIMER(17000);
  }

  int mask = getimask();
  setimask(15);

  sndUpdate();

  int JoyCnt = 0;

  struct mapledev *pad = locked_get_pads();
  for(int i = 0; i < 4; i++, pad++) {
    if( pad->func & MAPLE_FUNC_CONTROLLER && JoyCnt < 2) {
      int Buttons = ~pad->cond.controller.buttons & 0x0fff;
      Buttons |= ((pad->cond.controller.ltrigger > 30) ? DC_PAD::JOY_LTRIGGER:0);
      Buttons |= ((pad->cond.controller.rtrigger > 30) ? DC_PAD::JOY_RTRIGGER:0);
      int joyx = pad->cond.controller.joyx;
      int joyy = pad->cond.controller.joyy;
      
      if(Buttons == 0x0606)
	      Core::quit();
      
      Input::setJoyDown(JoyCnt, jkUp, Buttons & DC_PAD::JOY_DPAD_UP);
      Input::setJoyDown(JoyCnt, jkDown, Buttons & DC_PAD::JOY_DPAD_DOWN);
      Input::setJoyDown(JoyCnt, jkLeft, Buttons & DC_PAD::JOY_DPAD_LEFT);
      Input::setJoyDown(JoyCnt, jkRight, Buttons & DC_PAD::JOY_DPAD_RIGHT);
      
      Input::setJoyDown(JoyCnt, jkA,  (Buttons & DC_PAD::JOY_BTN_A));
      Input::setJoyDown(JoyCnt, jkB,  (Buttons & DC_PAD::JOY_BTN_B));
      Input::setJoyDown(JoyCnt, jkX,  (Buttons & DC_PAD::JOY_BTN_X));
      Input::setJoyDown(JoyCnt, jkY,  (Buttons & DC_PAD::JOY_BTN_Y));
      Input::setJoyDown(JoyCnt, jkLB, (Buttons & DC_PAD::JOY_LTRIGGER));
      Input::setJoyDown(JoyCnt, jkRB, (Buttons & DC_PAD::JOY_RTRIGGER));
      if ((Buttons & DC_PAD::JOY_LTRIGGER) != 0)
	        Input::setJoyDown(JoyCnt, jkStart, (Buttons & DC_PAD::JOY_BTN_START));
      else
	        Input::setJoyDown(JoyCnt, jkSelect, (Buttons & DC_PAD::JOY_BTN_START));
      if ((Buttons & DC_PAD::JOY_BTN_C) != 0)
	        Input::setJoyDown(JoyCnt, jkLB, (Buttons & DC_PAD::JOY_BTN_C));
      if ((Buttons & DC_PAD::JOY_BTN_Z) != 0)
	        Input::setJoyDown(JoyCnt, jkRB, (Buttons & DC_PAD::JOY_BTN_Z));
      
      vec2 stick = vec2(float(joyx), float(joyy)) / 128.0f - 1.0f;
      if (fabsf(joyx) < 0.2f && fabsf(joyy) < 0.2f)
        	stick = vec2(0.0f);
      
      Input::setJoyPos(JoyCnt, jkL, stick);
      
      JoyCnt++;
    }
  }

  setimask(mask);
}

int checkLanguage(int arg) {
  uint16 id;
  int str = STR_LANG_EN;

  if (arg < 0 ) return 0;

  id = arg;

  switch (id) {
        case 1: str = STR_LANG_EN; break;
        case 3: str = STR_LANG_FR; break;
        case 2: str = STR_LANG_DE; break;
        case 4: str = STR_LANG_ES; break;
        case 5: str = STR_LANG_IT; break;
  }
  return str - STR_LANG_EN;
}

int main()
{
    dc_init_hardware();

    osStartTime = Core::getTime();

    //printf("start time %d\n", osStartTime);
    cacheDir[0] = saveDir[0] = contentDir[0] = 0;

    sndInit();
    joyInit();

#ifdef ENABLE_LANG
    int id = DC_FLASH::GetLocalLanguage();
    Core::defLang = checkLanguage(id);
#endif

    Core::width  = 640;
    Core::height = 480;

    Game::init();

    //Game::init("PSXDATA/GYM.PSX");
    //Game::init("PSXDATA/LEVEL1.PSX");
    //Game::init("PSXDATA/LEVEL2.PSX");
    //Game::init("DEMODATA/LEVEL2.DEM");
    //Game::init("PSXDATA/CUT1.PSX");

    while (!Core::isQuit) {
      joyUpdate();

      if (Game::update()) {
	        ta_begin_frame();
	        primitive_buffer_begin();
	        Game::render();
	        primitive_buffer_flush();
	        ta_end_dlist();
      }
    }

    sndFree();
    Game::deinit();
    
    dcExit();
    return 0;
}

