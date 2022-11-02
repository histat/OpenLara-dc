#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "game.h"
#include "icon_data_2bpp.h"

#include <kos.h>
#include "private.h"
#include "watchdog.h"


#ifdef ENABLE_LANG
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
#endif

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

// multi-threading
#ifdef ENABLE_MUTEX
void* osMutexInit() {
  mutex_t *mutex;
  mutex = (mutex_t *)malloc(sizeof(mutex_t));
  if(!mutex) {
    LOG("mutex: malloc");
    return NULL;
  }

  int ret = mutex_init(mutex, MUTEX_TYPE_RECURSIVE);
  if(ret != 0) {
    LOG("mutex: init %d", ret);
    return NULL;
  }

  return (void*)mutex;
}

void osMutexFree(void *obj) {

  mutex_destroy((mutex_t *)obj);
  free(obj);
}

void osMutexLock(void *obj) {
  int ret = mutex_lock((mutex_t *)obj);
}

void osMutexUnlock(void *obj) {
  int ret = mutex_unlock((mutex_t *)obj);
}
#else

void* osMutexInit() {
  return NULL;
}

void osMutexFree(void *obj) {}

void osMutexLock(void *obj) {}

void osMutexUnlock(void *obj) {}
#endif

#define SAMPLERATE 44100

#define SND_FRAME_SIZE  4
#define SND_FRAMES   2352
Sound::Frame sndBuf[SND_FRAMES] __attribute__((aligned(2)));

static kthread_t * sndthd = NULL;

static void* sndFill(void *arg);

void sndInit() {

  audio_init();

  memset(sndBuf, 0, SND_FRAMES * SND_FRAME_SIZE);

  audio_register_ringbuffer(AUDIO_FORMAT_16BIT, SAMPLERATE, SND_FRAMES);

  sndthd = thd_create(0, &sndFill, NULL);
}

static void* sndFill(void *arg) {
    while (1) {
      Sound::fill(sndBuf, SND_FRAMES);
      audio_write_stereo_data(sndBuf, SND_FRAMES);

      thd_sleep(20);
    }
    return NULL;
}

void sndFree() {

  thd_destroy(sndthd);

  audio_unregister_ringbuffer();
}

// timing
int osStartTime = 0;

int osGetTimeMS() {
#if 0
  timeval t;
  gettimeofday(&t, NULL);
  return int((t.tv_sec - osStartTime) * 1000 + t.tv_usec / 1000);
#else
  return (int)(timer_ms_gettime64() - osStartTime);
#endif
}

void osCacheWrite(Stream *stream) {
    LOG("cache write : %s\n", stream->name);

  if (stream->callback)    
        stream->callback(new Stream(stream->name, stream->data, stream->size), stream->userData);
    delete stream;
}

void osCacheRead(Stream *stream) {
    LOG("cache read : %s\n", stream->name);

    if (stream->callback)
      stream->callback(NULL, stream->userData);
    delete stream;
}

// memory card

static unsigned char lara_icon[32+512];
static unsigned char lcd_icon[(48/8)*32];

static void conv_lcd_icon(unsigned char *bit, const unsigned char *in)
{
  unsigned int *src = (unsigned int *)in;
  unsigned char *dst = bit + (48/8) * 32;

  for (int i=0; i<32; i++) {
    unsigned char v;
    unsigned int b = *src++;
    v = 0;
    *--dst = 0xff;
    for (int j= 0; j<4; j++) {
      for (int x=0; x<8; x++) {
	      v <<= 1;
	      v |= (b & 1)?1:0;
	      b >>= 1;
      }
      *--dst = v;
    }
    *--dst = 0xff;
  }
}

static void conv_icon(unsigned char *bit, const unsigned char *in)
{
  const unsigned char *src = in;
  unsigned char *dst = ((unsigned char *)bit) + 32;
  unsigned short *pal = (unsigned short *)bit;

  pal[0] = 0xf000;
  pal[15] = 0xffff;

  for (int i=0; i<32; i++) {
    unsigned char v;
    unsigned char b;
    for (int j= 0; j<4; j++) {
      b = *src++;
      for (int x=0; x<4; x++) {
	      v = (b & 0x80)?0x00:0xf0;
	      v |= (b & 0x40)?0x00:0x0f;
	      b <<= 2;
	      dst[x] = v;
      }
      dst += 4;
    }
  }
}


void osReadSlot(Stream *stream) {
  LOG("read slot : %s\n", stream->name);

  maple_device_t *dev;
  vmu_pkg_t pkg;
  
  char *data;
  int len;

  uint8 *buf;
  int size;

  dev = maple_enum_type(0, MAPLE_FUNC_MEMCARD);

  if (!dev) {
    return;
  }

  if (dev->info.functions & MAPLE_FUNC_LCD) {
    vmu_draw_lcd(dev, lcd_icon);
  }

  if (vmufs_read(dev, stream->name, (void **)&buf, &size) < 0) {
    if (stream->callback)
      stream->callback(NULL, stream->userData);
  
    delete stream;
    return;
  }

  vmu_pkg_parse(buf, &pkg);
  
  data = (char *) pkg.data;
  len = pkg.data_len;

  if (stream->callback)
    stream->callback(new Stream(stream->name, data, len), stream->userData);

  if (buf)
    free(buf);

  delete[] data;
  delete stream;
}

void osWriteSlot(Stream *stream) {
  LOG("write slot : %s 0x%x\n", stream->name, stream->size);
  maple_device_t *dev;
  int free_bytes;
  vmu_pkg_t   pkg;
  int hdr_size;
  //uint8 *buf;
  uint8 *data;
  int bufSize;
  int ret;
  char filename[64];

  free_bytes = 0;

  dev = maple_enum_type(0, MAPLE_FUNC_MEMCARD);

  if (dev) {
    
    free_bytes = vmufs_free_blocks(dev);
  }

  if (free_bytes < ((stream->size+128+512+511)/512)) {
    if (stream->callback)
      stream->callback(NULL, stream->userData);

    delete stream;

    return;
  }

  if (dev->info.functions & MAPLE_FUNC_LCD) {
    vmu_draw_lcd(dev, lcd_icon);
  }
  
  memset(&pkg, 0, sizeof(struct vmu_pkg));
  strncpy(pkg.desc_short, "OpenLara", 16);
  strncpy(pkg.desc_long, "Save Data", 32);
  strncpy(pkg.app_id, "OpenLara", 16);
  pkg.icon_cnt = 1;
  pkg.icon_anim_speed = 0;
  pkg.eyecatch_type = VMUPKG_EC_NONE;
  memcpy(&pkg.icon_pal, &lara_icon, 32);
  pkg.icon_data =  lara_icon + 32;
  pkg.eyecatch_data = NULL;

  hdr_size = sizeof(struct vmu_hdr) + 512;
  pkg.data = (const uint8*)stream->data;

  pkg.data_len = stream->size;

  ret = vmu_pkg_build(&pkg, (uint8 **)&data, &bufSize);
  if (ret < 0) {
    free(data);
    if (stream->callback)
      stream->callback(NULL, stream->userData);

    delete stream;
    return;
  }

  vmufs_write(dev, stream->name, data, bufSize, VMUFS_OVERWRITE);
  free(data);

  if (stream->callback)
    stream->callback(new Stream(stream->name, stream->data, stream->size), stream->userData);

  delete stream;
}

// Input

#define INPUT_JOY_COUNT 4

struct JoyDevice {
    maple_device_t *dev;
    int               time;
    float             vL, vR;
    float             oL, oR;
} joyDevice[INPUT_JOY_COUNT];

static purupuru_effect_t effect;
#define JOY_MIN_UPDATE_FX_TIME   50

bool osJoyReady(int index) {
	return joyDevice[index].dev != NULL;
}

void rumbleInit()
{
	effect.duration = 0x00;
	effect.effect2 = 0x00;
	effect.effect1 = 0x00;
	effect.special = PURUPURU_SPECIAL_MOTOR1;
}

void joyRumble(int index) {

  //maple_device_t *dev;

    JoyDevice &joy = joyDevice[index];
    float intensity;
    if (joy.dev && (joy.vL != joy.oL || joy.vR != joy.oR) && Core::getTime() >= joy.time) {
      intensity = max(joy.vL, joy.vR);
      if (intensity > 10) {
	  effect.duration = 0xFF;
	  effect.effect1 = PURUPURU_EFFECT1_INTENSITY(6) | PURUPURU_EFFECT1_PULSE;
	  effect.effect2 = PURUPURU_EFFECT2_UINTENSITY(3);
	  effect.special = PURUPURU_SPECIAL_MOTOR1;
      } else {
	  effect.duration = 0xFF;
	  effect.effect1 = PURUPURU_EFFECT1_INTENSITY(2) | PURUPURU_EFFECT1_PULSE;
	  effect.effect2 = PURUPURU_EFFECT2_UINTENSITY(2);
	  effect.special = PURUPURU_SPECIAL_MOTOR1;
      }

      purupuru_rumble(joy.dev, &effect);

      joy.oL = joy.vL;
      joy.oR = joy.vR;
      joy.time = Core::getTime() + JOY_MIN_UPDATE_FX_TIME;
    }
}

void osJoyVibrate(int index, float L, float R) {
	joyDevice[index].vL = L;
	joyDevice[index].vR = R;
}

void joyInit()
{
}

void joyUpdate() {

  int JoyCnt = 0;
  maple_device_t *dev;

  for (int i = 0; i < INPUT_JOY_COUNT; i++) {
    dev = maple_enum_dev(i, 0);

    if (dev && (dev->info.functions & MAPLE_FUNC_CONTROLLER)) {

      cont_state_t *st = (cont_state_t *)maple_dev_status(dev);
      joyDevice[JoyCnt].dev = dev;
#if 0
      if (dev->info.functions & MAPLE_FUNC_LCD) {
	vmu_draw_lcd(dev, lcd_icon);
      }
#endif

      int Buttons = st->buttons & 0x0fff;
      Buttons |= ((st->ltrig > 30) ? DC_PAD::JOY_LTRIGGER:0);
      Buttons |= ((st->rtrig > 30) ? DC_PAD::JOY_RTRIGGER:0);

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
  
      int joyx = st->joyx+128;
      int joyy = st->joyy+128;
      int joy2x = st->joy2x+128;
      int joy2y = st->joy2y+128;

      vec2 stick = vec2(float(joyx), float(joyy)) / 128.0f - 1.0f;
      if (FABS(joyx) < 0.2f && FABS(joyy) < 0.2f)
	stick = vec2(0.0f);
      Input::setJoyPos(JoyCnt, jkL, stick);

      stick = vec2(float(joy2x), float(joy2y)) / 128.0f - 1.0f;
      if (FABS(joy2x) < 0.2f && FABS(joy2y) < 0.2f)
	stick = vec2(0.0f);
      Input::setJoyPos(JoyCnt, jkR, stick);
      JoyCnt++;
    }
  }
}

#ifdef ENABLE_LANG
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
#endif

int main()
{
    dc_init_hardware();

    osStartTime = Core::getTime();

    //printf("start time %d\n", osStartTime);
    cacheDir[0] = saveDir[0] = contentDir[0] = 0;

    strcat(contentDir, "/cd/");

    conv_icon(lara_icon, icon_data_2bpp);
    conv_lcd_icon(lcd_icon, icon_data_2bpp);

    sndInit();
    joyInit();
    rumbleInit();

#ifdef ENABLE_LANG
    int id = DC_FLASH::GetLocalLanguage();
    Core::defLang = checkLanguage(id);
#endif

    Core::width  = 640;
    Core::height = 480;

    Game::init();

    //Game::init("DATA/LEVEL1.PHD");
    //Game::init("DATA/LEVEL2.PHD");
    //Game::init("DATA/GYM.PHD");
#ifndef NOSERIAL
    wdResume();
#endif

    while (!Core::isQuit) {
#ifndef NOSERIAL
      wdPet();
#endif
      joyUpdate();

      if (Game::update()) {
	pvr_wait_ready();
	pvr_scene_begin();
	primitive_buffer_begin();
	Game::render();
	primitive_buffer_flush();
	pvr_scene_finish();
      }
    }

    sndFree();
    Game::deinit();

    return 0;
}

