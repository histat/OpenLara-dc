#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "icon_data_2bpp.h"

#include "private.h"
#include "audio.h"
#include "watchdog.h"

#define ENABLE_MUTEX

#ifdef ENABLE_LANG
namespace DC_FLASH {

int GetLocalLanguage() {

  flashrom_syscfg_t data;

  if (flashrom_get_syscfg(&data) == 0)
    return data.language;
  else
    return -1;
}

}

#endif


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
    LOG("mutex: init");
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
  if(ret != 0) {
    LOG("mutex lock");
  }
}

void osMutexUnlock(void *obj) {
  int ret = mutex_unlock((mutex_t *)obj);
  if(ret != 0) {
    LOG("mutex unlock");
  }
}
#else

void* osMutexInit() {
  return NULL;
}

void osMutexFree(void *obj) {}

void osMutexLock(void *obj) {}

void osMutexUnlock(void *obj) {}
#endif

void* osRWLockInit() {
    return osMutexInit();
}

void osRWLockFree(void *obj) {
    osMutexFree(obj);
}

void osRWLockRead(void *obj) {
    osMutexLock(obj);
}

void osRWUnlockRead(void *obj) {
    osMutexUnlock(obj);
}

void osRWLockWrite(void *obj) {
    osMutexLock(obj);
}

void osRWUnlockWrite(void *obj) {
    osMutexUnlock(obj);
}


#define SAMPLERATE 44100

#define SND_FRAME_SIZE  4
#define SND_FRAMES   2352
Sound::Frame sndBuf[SND_FRAMES + 32] __attribute__((aligned(2)));

static kthread_t * sndthd = NULL;

static void* sndFill(void *arg);

void sndInit() {

  audio_init();

  memset(sndBuf, 0, SND_FRAMES * SND_FRAME_SIZE);

  audio_register_ringbuffer(AUDIO_FORMAT_16BIT, SAMPLERATE, SND_FRAMES);

  sndthd = thd_create(0, &sndFill, NULL);
}

static void* sndFill(void *arg) {
    (void)arg;
    while (1) {
      uint32_t *samples = (uint32_t *)sndBuf;
      Sound::fill(sndBuf, SND_FRAMES);

      int numsamples = SND_FRAMES;

      int sleep_ms = (int)(1000.0 * ((float)SND_FRAMES / (float)SAMPLERATE) * (1.0 / 4.0));

      while (numsamples > 0) {
	unsigned int actual_written = audio_write_stereo_data(samples, numsamples);

	if (actual_written < numsamples) {
	  numsamples -= actual_written;
	  samples += actual_written;
	  thd_sleep(sleep_ms);
	} else {
	  numsamples = 0;
	}
      }
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
  return (int)(timer_ms_gettime64() - osStartTime);
}

void osCacheWrite(Stream *stream) {
    LOG("cache write : %s\n", stream->name);

    if (stream->callback)
        stream->callback(NULL, stream->userData);
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

  free(buf);

  delete stream;
}

void osWriteSlot(Stream *stream) {
  LOG("write slot : %s 0x%x\n", stream->name, stream->size);
  maple_device_t *dev;
  int free_bytes;
  vmu_pkg_t   pkg;
  uint8 *buf;
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

  pkg.data = (const uint8*)stream->data;

  pkg.data_len = stream->size;

  ret = vmu_pkg_build(&pkg, &buf, &bufSize);

  if (ret < 0) {
    if (stream->callback)
      stream->callback(NULL, stream->userData);

    delete stream;
    return;
  }

  vmufs_write(dev, stream->name, buf, bufSize, VMUFS_OVERWRITE);

  free(buf);

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

static int JoyCnt = 0;

static purupuru_effect_t effect;
#define JOY_MIN_UPDATE_FX_TIME   500

bool osJoyReady(int index) {
     return (joyDevice[index].dev != NULL ? true: false);
}

void rumbleInit()
{
}

void osJoyVibrate(int index, float L, float R) {
	joyDevice[index].vL = L;
	joyDevice[index].vR = R;
}

void joyInit()
{
  for (int i = 0; i < INPUT_JOY_COUNT; i++) {
    JoyDevice &joy = joyDevice[i];
    joy.dev = NULL;
    joy.time = -1;
  }
}

void joyUpdate() {

  maple_device_t *dev;

  JoyCnt = 0;

  for (int i = 0; i < INPUT_JOY_COUNT; i++) {

    JoyDevice &joy = joyDevice[JoyCnt];

    dev = maple_enum_dev(i, 0);
    joy.dev = dev;

    if (dev && (dev->info.functions & MAPLE_FUNC_CONTROLLER)) {

      cont_state_t *st = (cont_state_t *)maple_dev_status(dev);

#if 0
      if (dev->info.functions & MAPLE_FUNC_LCD) {
	vmu_draw_lcd(dev, lcd_icon);
      }
#endif

      Input::setJoyDown(JoyCnt, jkUp, st->buttons & CONT_DPAD_UP);
      Input::setJoyDown(JoyCnt, jkDown, st->buttons & CONT_DPAD_DOWN);
      Input::setJoyDown(JoyCnt, jkLeft, st->buttons & CONT_DPAD_LEFT);
      Input::setJoyDown(JoyCnt, jkRight, st->buttons & CONT_DPAD_RIGHT);
  
      Input::setJoyDown(JoyCnt, jkA,  (st->buttons & CONT_A));
      Input::setJoyDown(JoyCnt, jkB,  (st->buttons & CONT_B));
      Input::setJoyDown(JoyCnt, jkX,  (st->buttons & CONT_X));
      Input::setJoyDown(JoyCnt, jkY,  (st->buttons & CONT_Y));
      Input::setJoyDown(JoyCnt, jkLB, (st->ltrig > 30));
      Input::setJoyDown(JoyCnt, jkRB, (st->rtrig > 30));
  
      Input::setJoyDown(JoyCnt, jkSelect, (st->buttons & CONT_START));
      if ((st->buttons & CONT_C) != 0)
	Input::setJoyDown(JoyCnt, jkLB, (st->buttons & CONT_C));
      if ((st->buttons & CONT_Z) != 0)
	Input::setJoyDown(JoyCnt, jkRB, (st->buttons & CONT_Z));
  
      int joyx = st->joyx+128;
      int joyy = st->joyy+128;
      int joy2x = st->joy2x+128;
      int joy2y = st->joy2y+128;

      vec2 stick = vec2(float(joyx), float(joyy)) / 128.0f - 1.0f;
      if (fabs(joyx) < 0.2f && fabs(joyy) < 0.2f)
	stick = vec2(0.0f);
      Input::setJoyPos(JoyCnt, jkL, stick);

      stick = vec2(float(joy2x), float(joy2y)) / 128.0f - 1.0f;
      if (fabs(joy2x) < 0.2f && fabs(joy2y) < 0.2f)
	stick = vec2(0.0f);
      Input::setJoyPos(JoyCnt, jkR, stick);

      JoyCnt++;

      if ((joy.vL != joy.oL || joy.vR != joy.oR) && Core::getTime() >= joy.time) {
	joy.oL = joy.vL;
	joy.oR = joy.vR;
	joy.time = Core::getTime() + JOY_MIN_UPDATE_FX_TIME;
      }
    }
  }
}

#ifdef ENABLE_LANG
int checkLanguage(int arg) {
  int id;
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

#ifndef NOSERIAL
    wdResume();
#endif

    while (!Core::isQuit) {
#ifndef NOSERIAL
      wdPet();
#endif
      joyUpdate();

      if (Game::update()) {
	Game::render();
      }
    }

    sndFree();
    Game::deinit();

    return 0;
}

