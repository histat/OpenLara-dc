#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <zlib.h>
#include "vmu.h"
#include "vm_file.h"
#include <ronin/ronin.h>

#include "icon_data_2bpp.h"

static unsigned char lara_icon[32+512];
static unsigned char lcd_icon[(48/8)*32];

static void setlcd(struct vmsinfo *info, void *bit)
{
  unsigned int param[50];
  
  param[0] = MAPLE_FUNC_LCD<<24;
  param[1] = 0;
  memcpy(param+2, bit, 48*4);
  maple_docmd(info->port, info->dev, MAPLE_COMMAND_BWRITE, 50, param);
}

/*
static void clearlcd(struct vmsinfo *info)
{
  unsigned int bit[50];
  
  memset(bit, 0, sizeof(bit));
  setlcd(info, bit);
}
*/

static void conv_lcd_icon(unsigned char *bit, const unsigned char *in)
{
  int i,x,j;
  unsigned int *src = (unsigned int *)in;
  unsigned char *dst = bit + (48/8) * 32;

  for (i=0; i<32; i++) {
    unsigned char v;
    unsigned int b = *src++;
    v = 0;
    *--dst = 0xff;
    for (j= 0; j<4; j++) {
      for (x=0; x<8; x++) {
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
  int i,x,j;
  const unsigned char *src = in;
  unsigned char *dst = ((unsigned char *)bit) + 32;
  unsigned short *pal = (unsigned short *)bit;

  pal[0] = 0xf000;
  pal[15] = 0xffff;

  for (i=0; i<32; i++) {
    unsigned char v;
    unsigned char b;
    for (j= 0; j<4; j++) {
      b = *src++;
      for (x=0; x<4; x++) {
	v = (b & 0x80)?0x00:0xf0;
	v |= (b & 0x40)?0x00:0x0f;
	b <<= 2;
	dst[x] = v;
      }
      dst += 4;
    }
  }
}

#define MAX_VMU_SIZE (128 * 1024)

int vm_file;
static bool vmu_avail[4*2];

bool vmfile_search(const char *fname, int *vm)
{
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;
  static bool inited = false;
  
  if(!inited) {
    memset(vmu_avail, false, sizeof(vmu_avail));
    
    int mask = getimask();
    setimask(15);
    struct mapledev *pad=locked_get_pads();
    for (int i=0; i<4; ++i, ++pad) {
      if (pad[i].present & (1<<0)) {
	vmu_avail[i] = true;
      }
      if (pad[i].present & (1<<1)) {
	vmu_avail[i+4] = true;
      }
    }
    setimask(mask);
  
    conv_lcd_icon(lcd_icon, icon_data_2bpp);  

    conv_icon(lara_icon, icon_data_2bpp);

    inited = true;
  }

  for (int x=0; x<4; x++) {
    for (int y=0; y<2; y++) {
      
      if (vmu_avail[x+y*4]) {
	int res = x*6 + y + 1;
	if (vmsfs_check_unit(res, 0, &info))
	  if (vmsfs_get_superblock(&info, &super))
	    if (vmsfs_open_file(&super, fname, &file)) {
#ifndef NOSERIAL
	      printf("%s Found on %c%d\n", fname, 'A'+res/6,res%6);	
#endif	
	      *vm = res;
	      return true;
	    }
      }
    }
  }
  return false;
}

bool vmfile_exists(const char *fname)
{
    return vmfile_search(fname, &vm_file);
}

bool save_to_vmu(int unit, const char *filename, const char *buf, int buf_len)
{
  struct vms_file_header header;
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;
  char new_filename[16];
  int free_cnt;
  time_t long_time;
  struct tm *now_time;
  struct timestamp stamp;
  unsigned char compressed_buf[MAX_VMU_SIZE];
  int compressed_len;

  memset(compressed_buf, 0, sizeof(compressed_buf));
  compressed_len = buf_len + 512;
  
  if (compress((Bytef*)compressed_buf, (uLongf*)&compressed_len,
	       (Bytef*)buf, buf_len) != Z_OK) {
    return false;
  }
  if (!vmsfs_check_unit(unit, 0, &info)) {
    return false;
  }
  if (!vmsfs_get_superblock(&info, &super)) {
    return false;
  }
  free_cnt = vmsfs_count_free(&super);

  strncpy(new_filename, filename, sizeof(new_filename));
  if (vmsfs_open_file(&super, new_filename, &file))
    free_cnt += file.blks;
  
  if (((128+512+compressed_len+511)/512) > free_cnt) {
    return false;
  }
  
  memset(&header, 0, sizeof(header));
  strncpy(header.shortdesc, "OpenLara",sizeof(header.shortdesc));
  strncpy(header.longdesc, "OpenLara/SAVE", sizeof(header.longdesc));
  strncpy(header.id, "OpenLara", sizeof(header.id));
  header.numicons = 1;
  memcpy(header.palette, lara_icon, sizeof(header.palette));

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

  if (!vmsfs_create_file(&super, new_filename, &header, lara_icon+sizeof(header.palette), NULL, compressed_buf, compressed_len, &stamp)) {
#ifndef NOSERIAL
    fprintf(stderr,"%s",vmsfs_describe_error());
#endif
    return false;
  }

  setlcd(&info, lcd_icon);
  
  return true;
}

bool load_from_vmu(int unit, const char *filename, char *buf, int *buf_len)
{
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;
  unsigned char compressed_buf[MAX_VMU_SIZE];
  unsigned int compressed_len;
  
  if (!vmsfs_check_unit(unit, 0, &info)) {
    return false;
  }
  if (!vmsfs_get_superblock(&info, &super)) {
    return false;
  }
  if (!vmsfs_open_file(&super, filename, &file)) {
    return false;
  }
  
  memset(compressed_buf, 0, sizeof(compressed_buf));
  compressed_len = file.size;
  
  if (!vmsfs_read_file(&file, (Bytef*)compressed_buf, compressed_len)) {
    return false;
  }
  
  if (!*buf_len)
    *buf_len = MAX_VMU_SIZE;
  
  if (uncompress((Bytef*)buf, (uLongf*)buf_len,(const Bytef*)compressed_buf, (uLong)compressed_len) != Z_OK) {
    return false;
  }

  setlcd(&info, lcd_icon);

  return true;
}

