#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "vm_file.h"
#include "vmu.h"

#define MAX_FILES (2)
#define MAX_SIZE 128*1024

static struct {
	int used;
	VMFILE _iob;
	char _buffer[MAX_SIZE];
} fh[MAX_FILES];


// ---

VMFILE *vm_fileopen(const char *fname, const char *mode)
{
  VMFILE *ret;
  int size = 0;
  int cnt = 0;
  int vm = 0;
  int i;
  
  for (i=0; i<MAX_FILES; i++)
    if(fh[i].used == 0)
      break;
  
  if (i>=MAX_FILES)
    return NULL;
  
  fh[i].used = 1;
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
    
  } else if (!strncmp(mode, "wb", 2)) {
    
    if (!vmfile_search(fname, &vm)) {
#ifndef NOSERIAL
      printf("Create %s", fname);
#endif
    }

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
  fh[i].used = 0;
  return NULL;
}

void vm_fclose(VMFILE *fp)
{
  int i;
  
  if (!fp->_cnt)
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
  fh[fp->_fd].used = 0;
  return;
}

int vm_fread(void *buf, int size, int n, VMFILE *fp)
{
  memcpy(buf, fp->_ptr, size);
  
  fp->_ptr += size;
  
  return size;
}

size_t vm_fwrite(const void *buf, int size, int n, VMFILE *fp)
{
  memcpy(fp->_ptr, buf, size);
  
  fp->_ptr += size * n;
  fp->_cnt += size * n;
  
  if (fp->_cnt > MAX_SIZE) {
#ifndef NOSERIAL
    fprintf(stderr,"*** 0x%x > 0x%x in %s***", fp->_cnt, MAX_SIZE,__func__);
#endif
    return 0;
  }
  
  return size;
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

#if 0
int vm_fprintf(VMFILE *fp , const char *format , ... )
{
	va_list	ap;
	char buf[512];
	int len;

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	len = strlen(buf);
	
	vm_fwrite(buf, len, 1, fp);

	return len;
}
/*
void vm_remove(const char *fname)
{
	int vm;
	if (!vmfile_search(fname, &vm))
		return;
	
	delete_file_vmu(vm, fname);
}

int vm_rename(const char *oldpath, const char *newpath)
{
	if (!rename_vmu_file(oldpath, newpath)) {
		return -1;
	}
	
	return 0;
}
*/

int vm_fgetc(VMFILE *fp )
{
	uint8_t value;
	vm_fread(&value, 1, 1, fp);

	return value;
	
}

int vm_fputc(int c, VMFILE *fp )
{
	uint8_t value;
	value = c;
	vm_fwrite(&value, 1, 1, fp);

	return 1;
}

int vm_feof( VMFILE *fp )
{
	char *p = fp->_ptr;

	if ((int)p[0] == -1)
		return -1;
	
	return 0;
}

uint16_t vm_fgeti(VMFILE *fp)
{
	uint16_t value;
	vm_fread(&value, 2, 1, fp);
	return value;
}

uint32_t vm_fgetl(VMFILE *fp)
{
	uint32_t value;
	vm_fread(&value, 4, 1, fp);
	return value;
}

void vm_fputi(uint16_t word, VMFILE *fp)
{
	vm_fwrite(&word, 2, 1, fp);
}

void vm_fputl(uint32_t word, VMFILE *fp)
{
	vm_fwrite(&word, 4, 1, fp);
}

// write a string to a file-- does NOT null-terminate it
void vm_fputstringnonull(const char *buf, VMFILE *fp)
{
	if (buf[0])
		vm_fprintf(fp, "%s", buf);
}

// reads strlen(str) bytes from file fp, and returns true if they match "str"
bool vm_fverifystring(VMFILE *fp, const char *str)
{
int i;
char result = 1;
int stringlength = strlen(str);

	for(i=0;i<stringlength;i++)
	{
		if (vm_fgetc(fp) != str[i]) result = 0;
	}
	
	return result;
}


/*
void c------------------------------() {}
*/

static int boolbyte, boolmask_r, boolmask_w;

// prepare for a boolean read operation
void vm_fresetboolean(void)
{
	boolmask_r = 256;
	boolmask_w = 1;
	boolbyte = 0;
}


// read a boolean value (a single bit) from a file
char vm_fbooleanread(VMFILE *fp)
{
	char value;

	if (boolmask_r == 256)
	{
		boolbyte = vm_fgetc(fp);
		boolmask_r = 1;
	}
	
	value = (boolbyte & boolmask_r) ? 1:0;
	boolmask_r <<= 1;
	return value;
}

void vm_fbooleanwrite(char bit, VMFILE *fp)
{
	if (boolmask_w == 256)
	{
		vm_fputc(boolbyte, fp);
		boolmask_w = 1;
		boolbyte = 0;
	}
	
	if (bit)
	{
		boolbyte |= boolmask_w;
	}
	
	boolmask_w <<= 1;
}


void vm_fbooleanflush(VMFILE *fp)
{
	vm_fputc(boolbyte, fp);
	boolmask_w = 1;
}


// ----
/*
VMFileBuffer::VMFileBuffer()
{
	fMaxSize = 0;
	fFP = NULL;

	memset(fbuffer, 0, sizeof(fbuffer));
	//printf("%s", __func__);
}

void VMFileBuffer::SetBufferSize(int maxsize)
{
	fMaxSize = maxsize;
	//printf("%s %d", __func__, fMaxSize);
}

void VMFileBuffer::SetFile(VMFILE *fp)
{
	fFP = fp;
	_pos = 0;
}
*/
/*
void c------------------------------() {}
*/

/*
void VMFileBuffer::Write8(uint8_t data)
{
	memcpy((uint8_t*)&fbuffer[_pos], (uint8_t*)&data, 1);
	_pos += 1;
	
	CheckFlush(fMaxSize);
}

void VMFileBuffer::Write16(uint16_t data)
{
	memcpy((uint8_t*)&fbuffer[_pos], (uint8_t*)&data, 2);
	_pos += 2;
	
	CheckFlush(fMaxSize);
}

void VMFileBuffer::Write32(uint32_t data)
{
	memcpy((uint8_t*)&fbuffer[_pos], (uint8_t*)&data, 4);
	_pos += 4;
	
	CheckFlush(fMaxSize);
}
*/
/*
void c------------------------------() {}
*/
/*
void VMFileBuffer::CheckFlush(int maxsize)
{
	if (_pos >= maxsize)
	{
		if (fFP)
		{
			//stat("CheckFlush wrote %d bytes", _pos);
			
			vm_fwrite(fbuffer, _pos, 1, fFP);
			_pos = 0;
		}
		else
		{
			staterr("CheckFlush: no file");
		}
	}
}

void VMFileBuffer::Flush()
{
	CheckFlush(0);
}

void VMFileBuffer::Dump()
{
}
*/
#endif
