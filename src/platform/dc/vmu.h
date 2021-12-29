#ifndef VMU_H_INCLUDED
#define VMU_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

bool vmfile_search(const char *fname, int *vm);
void vm_avail();
bool save_to_vmu(int unit, const char *filename, const char *buf, int buf_len);
bool load_from_vmu(int unit, const char *filename, char *buf, int *buf_len);

  //int save_to_vmu(int unit, const char *filename, const char *desc_long, const char *desc_short, void *buf, int buf_len, unsigned char *icon_data);
  //int load_from_vmu(int unit, const char *filename, void *buf, int *buf_len);

#ifdef __cplusplus
}
#endif
#endif
