#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "vm_file.h"
#include "vmu.h"

#define MAX_FILES (2)
#define MAX_SIZE 128*1024

static struct {
	int flag;
	VMFILE _iob;
	char _buffer[MAX_SIZE];
} fh[MAX_FILES];


enum {
  O_READ = 1,
  O_WRITE = 2,
};

// ---

VMFILE *vm_fileopen(const char *fname, const char *mode)
{
  VMFILE *ret;
  int size = 0;
  int cnt = 0;
  int vm = 0;
  int i;
  
  for (i=0; i<MAX_FILES; i++)
    if(fh[i].flag == 0)
      break;
  
  if (i>=MAX_FILES)
    return NULL;
  
  memset(fh[i]._buffer, 0, MAX_SIZE);
  char *_buffer = fh[i]._buffer;
  
  if (!strncmp(mode, "rb", 2)) {
    
    if (!vmfile_search(fname, &vm)) {
#ifndef NOSERIAL
      printf("Can't open %s", fname);
#endif
      goto _exit;
    }
    if (!load_from_vmu(vm, fname, _buffer, &size)) {
#ifndef NOSERIAL
      printf("load failed rb %s", fname);
#endif
      
      goto _exit;
    }

    fh[i].flag = O_READ;

    cnt = size;
    
  } else if (!strncmp(mode, "wb", 2)) {
    
    if (!vmfile_search(fname, &vm)) {
#ifndef NOSERIAL
      printf("Create %s", fname);
#endif
    }

    fh[i].flag = O_WRITE;

  } else if (!strncmp(mode, "r+", 2)) {

    if (!vmfile_search(fname, &vm)) {
#ifndef NOSERIAL
      printf("Can't open %s", fname);
#endif
      goto _exit;
    }
    
    if (!load_from_vmu(vm, fname, _buffer, &size)) {
      goto _exit;
    }
    
    fh[i].flag = O_READ;
    cnt = size;
    
  } else {
#ifndef NOSERIAL
    fprintf(stderr,"ERROR: VMU");
#endif
    goto _exit;
  }
  
  ret = &fh[i]._iob;

  memset(ret->filename, 0, 32);
  strncpy(ret->filename, fname, 32);
  
  ret->_fd = i;
  ret->_base = _buffer;
  ret->_ptr = ret->_base;
  ret->_cnt = cnt;
  ret->_vm = vm;
  
  return ret;
_exit:
  fh[i].flag = 0;
  return NULL;
}

void vm_fclose(VMFILE *fp)
{
  int i;
  
  if (fh[fp->_fd].flag != O_WRITE)
    goto _exit;
  
  if (fp->_vm > 0)
    if (save_to_vmu(fp->_vm, fp->filename, fp->_base, fp->_cnt)) {
#ifndef NOSERIAL
      printf("SUCCESS port %c%d", 'A'+ fp->_vm/6, fp->_vm%6);
#endif
      goto _exit;
    }
  
  for (i=0; i<24; i++)
    if (save_to_vmu(i, fp->filename, fp->_base, fp->_cnt)) {
#ifndef NOSERIAL
      printf("SUCCESS port %c%d", 'A'+ i/6, i%6);
#endif
      break;
    }
  
  if (i>=24) {
#ifndef NOSERIAL			
    fprintf(stderr,"no available vmu\n");
#endif
  }
_exit:
  fh[fp->_fd].flag = 0;
  return;
}

int vm_fread(void *buf, int size, int n, VMFILE *fp)
{
  memcpy(buf, fp->_ptr, size * n);
  
  fp->_ptr += size * n;
  
  return size * n;
}

size_t vm_fwrite(const void *buf, int size, int n, VMFILE *fp)
{
  memcpy(fp->_ptr, buf, size * n);
  
  fp->_ptr += size * n;
  fp->_cnt += size * n;
  
  if (fp->_cnt > MAX_SIZE) {
#ifndef NOSERIAL
    fprintf(stderr,"*** 0x%x > 0x%x in %s***", fp->_cnt, MAX_SIZE,__func__);
#endif
    return 0;
  }
  
  return size * n;
}


int vm_fseek(VMFILE *fp, int offs, int whence)
{
  fp->_ptr = fp->_base + offs;
  return 0;
}

int vm_fsize(VMFILE *fp)
{
  return fp->_cnt;
}
