#ifndef _VMFILE_H_
#define _VMFILE_H_

struct VMFILE
{
	int _fd;
	char *_ptr;
	int _cnt;
	char *_base;
	char _vm;
	char filename[32];
};

VMFILE *vm_fileopen(const char *fname, const char *mode);
void vm_fclose(VMFILE *fp);
int vm_fread(void *buf, int size, int n, VMFILE *fp);
size_t vm_fwrite(const void *buf, int size, int n, VMFILE *fp);
int vm_fseek(VMFILE *fp, int offs, int whence);
int vm_fsize(VMFILE *fp);

#endif
