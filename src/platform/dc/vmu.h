#ifndef VMU_H_INCLUDED
#define VMU_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void conv_lcd_icon(unsigned char *bit, const unsigned char *in);
void conv_icon(unsigned char *bit, const unsigned char *in);

bool save_to_vmu(int unit, const char *filename, const char *buf, int buf_len, unsigned char *icon, unsigned char *lcd);
bool load_from_vmu(int unit, const char *filename, char *buf, int *buf_len, unsigned char *lcd);

#ifdef __cplusplus
}
#endif
#endif
