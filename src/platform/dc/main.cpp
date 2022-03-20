#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "game.h"

#include <ronin/ronin.h>
#include <ronin/soundcommon.h>

#include "private.h"
#include "icon_data_2bpp.h"

extern "C" {
  void dcExit();
  void *memcpy4s(void *s1, const void *s2, unsigned int n);
}

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

//int32 fps;
int32 frameIndex = 0;
int32 fpsCounter = 0;
uint32 curSoundBuffer = 0;

#define SND_FRAME_SIZE  4
//#define SND_FRAMES      1152
#define SND_FRAMES      1024

Sound::Frame  sndBuf[SND_FRAMES * 2 + 32] __attribute__((aligned(2)));

uint32* soundBuffer;

void sndInit() {

  stop_sound();
  do_sound_command(CMD_SET_BUFFER(3));
  do_sound_command(CMD_SET_STEREO(1));
  do_sound_command(CMD_SET_FREQ_EXP(FREQ_44100_EXP));

  memset(sndBuf, 0, SND_FRAMES * SND_FRAME_SIZE);

  soundBuffer = (uint32 *)sndBuf;
}

void soundFill()
{
  if (read_sound_int(&SOUNDSTATUS->mode) != MODE_PLAY)
    start_sound();

  int ring_buffer_samples = read_sound_int(&SOUNDSTATUS->ring_length);
  int n = read_sound_int(&SOUNDSTATUS->samplepos);

  if ((n-=fillpos)<0)
    n += ring_buffer_samples; //n = n + fillpos + (ring_buffer_samples - fillpos)

  if(n < 22)
    return;

  if (curSoundBuffer == 1) {
  }
  //sndFill(soundBuffer + curSoundBuffer * SND_SAMPLES, SND_SAMPLES);
  Sound::fill((Sound::Frame*)soundBuffer + curSoundBuffer * SND_FRAMES, SND_FRAMES);
  #if 1
  uint32 *srcBuffer = soundBuffer + curSoundBuffer * SND_FRAMES;

  n = SND_FRAMES;

  if (fillpos+n > ring_buffer_samples) {
    int r = ring_buffer_samples - fillpos;
    memcpy4s(RING_BUF+fillpos, srcBuffer, SAMPLES_TO_BYTES(r));
    fillpos = 0;
    n -= r;
    memcpy4s(RING_BUF, srcBuffer+r, SAMPLES_TO_BYTES(n));
  } else {
    memcpy4s(RING_BUF+fillpos, srcBuffer, SAMPLES_TO_BYTES(n));
  }

 if ((fillpos += n) >= ring_buffer_samples)
    fillpos = 0;

#endif

  curSoundBuffer ^= 1;
}

void vblank()
{
  frameIndex++;
  soundFill();
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

void osCacheWrite(Stream *stream) {

    LOG("cache write : %s\n", stream->name);

    if (strcmp(stream->name, "settings") == 0) {
      osWriteSlot(stream);
      return;
    }

    if (stream->callback)
      stream->callback(NULL, stream->userData);

    delete stream;
}

void osCacheRead(Stream *stream) {
    LOG("cache read : %s\n", stream->name);

    if (strcmp(stream->name, "settings") == 0) {
      osReadSlot(stream);
      return;
    }

    int fd = open(stream->name, O_RDONLY);
    if (fd>=0) {
        int size = (int)file_size(fd);
        char *data = new char[size];
        read(fd, data, size);
        close(fd);
        if (stream->callback)
            stream->callback(new Stream(stream->name, data, size), stream->userData);
        delete[] data;
    } else
        if (stream->callback)
            stream->callback(NULL, stream->userData);
    delete stream;
}

// memory card

#define MAX_VMU_SIZE 1*1024

static unsigned char lara_icon[32+512];
static unsigned char lcd_icon[(48/8)*32];

static int last_vm = -1;

enum vmuresult {
    VMU_OK,
    VMU_NO,
    VMU_NORES,
    VMU_NOSPACE,
    VMU_NOFILE,
    VMU_READFAILE,
    VMU_WRITEFAILE,
    VMU_NOFILESPACE
};

vmuresult vmu_errno;

static void setlcd(struct vmsinfo *info, void *bit)
{
  unsigned int param[50];

  param[0] = MAPLE_FUNC_LCD<<24;
  param[1] = 0;
  memcpy(param+2, bit, 48*4);
  maple_docmd(info->port, info->dev, MAPLE_COMMAND_BWRITE, 50, param);
}

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

bool saveVMU(int unit, const char *filename, const char *buf, int size, unsigned char *icon, unsigned char *lcd)
{
  struct vms_file_header header;
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;
  char new_filename[16];
  unsigned int free_cnt;
  time_t long_time;
  struct tm *now_time;
  struct timestamp stamp;

  if (!vmsfs_check_unit(unit, 0, &info)) {
    vmu_errno = VMU_NO;
    return false;
  }
  if (!vmsfs_get_superblock(&info, &super)) {
    vmu_errno = VMU_NORES;
    return false;
  }
  free_cnt = vmsfs_count_free(&super);

  strncpy(new_filename, filename, sizeof(new_filename));
  if (vmsfs_open_file(&super, new_filename, &file))
    free_cnt += file.blks;

  if (((128+512+size+511)/512) > free_cnt) {
    vmu_errno = VMU_NOSPACE;
    return false;
  }

  memset(&header, 0, sizeof(header));
  strncpy(header.shortdesc, "Save Data",sizeof(header.shortdesc));
  strncpy(header.longdesc, "OpenLara", sizeof(header.longdesc));
  strncpy(header.id, "OpenLara", sizeof(header.id));
  header.numicons = 1;
  memcpy(header.palette, icon, sizeof(header.palette));

  time(&long_time);
  now_time = localtime(&long_time);
  if (now_time != NULL) {
    stamp.year = now_time->tm_year + 1900;
    stamp.month = now_time->tm_mon + 1;
    stamp.wkday = now_time->tm_wday;
    stamp.day = now_time->tm_mday;
    stamp.hour = now_time->tm_hour;
    stamp.minute = now_time->tm_min;
    stamp.second = now_time->tm_sec;
  }

  vmu_errno = VMU_OK;

  vmsfs_beep(&info, 1);

  if (!vmsfs_create_file(&super, new_filename, &header, icon+sizeof(header.palette), NULL, buf, size, &stamp)) {
#ifndef NOSERIAL
    fprintf(stderr,"%s",vmsfs_describe_error());
#endif
    vmsfs_beep(&info, 0);
    vmu_errno = VMU_WRITEFAILE;
    return false;
  }
  vmsfs_beep(&info, 0);

  setlcd(&info, lcd);

  return true;
}

bool loadVMU(int unit, const char *filename, char *&buf, int &size, unsigned char *lcd)
{
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;

  if (!vmsfs_check_unit(unit, 0, &info)) {
    vmu_errno = VMU_NO;
    return false;
  }
  if (!vmsfs_get_superblock(&info, &super)) {
    vmu_errno = VMU_NORES;
    return false;
  }
  if (!vmsfs_open_file(&super, filename, &file)) {
    vmu_errno = VMU_NOFILE;
    return false;
  }

  size = file.size;

  buf = new char[size];

  if (!vmsfs_read_file(&file, (unsigned char *)buf, size)) {
    vmu_errno = VMU_READFAILE;
    return false;
  }

  setlcd(&info, lcd);

  vmu_errno = VMU_OK;

  return true;
}

void osReadSlot(Stream *stream) {
  LOG("read slot : %s\n", stream->name);

  int size;
  char *data;

  if(last_vm >= 0 && loadVMU(last_vm, stream->name, data, size, lcd_icon)) {
    if (stream->callback)
      stream->callback(new Stream(stream->name, data, size), stream->userData);

    delete[] data;
    delete stream;
    return;
  }

  last_vm = -1;

  for (int i=0; i<24; i++)
     if(loadVMU(i, stream->name, data, size, lcd_icon)) {
      last_vm = i;
      break;
  }

  if (last_vm >= 0) {
    if (stream->callback)
      stream->callback(new Stream(stream->name, data, size), stream->userData);
    delete[] data;
  } else
      if (stream->callback)
          stream->callback(NULL, stream->userData);

    delete stream;

    LOG("%s %d:\n",__FUNCTION__, vmu_errno);
}

void osWriteSlot(Stream *stream) {
  LOG("write slot : %s 0x%x\n", stream->name, stream->size);

  if(stream->size > MAX_VMU_SIZE) {
    vmu_errno = VMU_NOFILESPACE;
    if (stream->callback)
      stream->callback(NULL, stream->userData);

    delete stream;
    return;
  }

  if (last_vm >= 0 && saveVMU(last_vm, stream->name, stream->data, stream->size, lara_icon, lcd_icon)) {
     if (stream->callback)
      stream->callback(new Stream(stream->name, stream->data, stream->size), stream->userData);

    delete stream;
    return;
  }

  last_vm = -1;

  for (int i=0; i<24; i++)
    if (saveVMU(i, stream->name, stream->data, stream->size, lara_icon, lcd_icon)) {
      last_vm = i;
      break;
    }

  if (last_vm >= 0) {
     if (stream->callback)
      stream->callback(new Stream(stream->name, stream->data, stream->size), stream->userData);
  } else
      if (stream->callback)
        stream->callback(NULL, stream->userData);

  delete stream;

  LOG("%s %d:\n",__FUNCTION__, vmu_errno);
}

// Input
bool osJoyReady(int index) {
  return index == 0;
}

#define X_MAX(a,b)       ((a) > (b) ? (a) : (b))
#define CART_RUMBLE_TICKS     6

int32 cartRumbleTick = 0;

struct rumbinfo rumblepack;
void rumbleInit()
{
  rumble_check_unit(1, &rumblepack);
}

void rumbleSet(bool enable)
{
    if (enable) {
        cartRumbleTick = CART_RUMBLE_TICKS;
    } else {
        cartRumbleTick = 0;
    }

    if(cartRumbleTick)
      rumble_set(&rumblepack, 1);
    else
      rumble_set(&rumblepack, 0);
}

void rumbleUpdate(int32 frames)
{
    if (!cartRumbleTick)
        return;

    cartRumbleTick -= frames;

    if (cartRumbleTick <= 0) {
        rumbleSet(false);
    }
}

void osJoyVibrate(int index, float L, float R) {
    rumbleSet(X_MAX(L, R) > 0);
}

void joyInit() {
}

void joyUpdate() {

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

  vblank();

  int JoyCnt = 0;

  struct mapledev *pad = locked_get_pads();
  for (int i = 0; i < 4; i++, pad++) {
    if ( pad->func & MAPLE_FUNC_CONTROLLER && JoyCnt < 2) {
      int Buttons = ~pad->cond.controller.buttons & 0x0fff;
      Buttons |= ((pad->cond.controller.ltrigger > 30) ? DC_PAD::JOY_LTRIGGER:0);
      Buttons |= ((pad->cond.controller.rtrigger > 30) ? DC_PAD::JOY_RTRIGGER:0);
      int joyx = pad->cond.controller.joyx;
      int joyy = pad->cond.controller.joyy;
      
      if (Buttons == 0x0606)
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

    //Game::init("PSXDATA/GYM.PSX");
    //Game::init("PSXDATA/LEVEL1.PSX");
    //Game::init("PSXDATA/LEVEL2.PSX");
    //Game::init("DEMODATA/LEVEL2.DEM");
    //Game::init("PSXDATA/CUT1.PSX");

    int32 lastFrameIndex = -1;

    while (!Core::isQuit) {
      joyUpdate();

      int32 frame = frameIndex / 2;
      int32 delta = frame - lastFrameIndex;

      lastFrameIndex = frame;

      if (delta != 0)
        rumbleUpdate(delta);


      if (Game::update()) {
	        ta_begin_frame();
	        primitive_buffer_begin();
	        Game::render();
	        primitive_buffer_flush();
	        ta_end_dlist();
      }

      //fpsCounter++;
      if (frameIndex >= 60)
      {
          frameIndex -= 60;
          lastFrameIndex -= 30;
          //fps = fpsCounter;

          //fpsCounter = 0;
      }
    }

    sndFree();
    Game::deinit();
    
    dcExit();
    return 0;
}

