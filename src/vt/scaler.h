#ifndef _SCALER_H_
#define _SCALER_H_

void Super2xSaI(unsigned char *src, unsigned int src_pitch, int src_bytes_per_pixel, unsigned char *dst, unsigned int dst_pitch, int dst_bytes_per_pixel, int width, int height, int pal[256]);

#endif // _SCALER_H_
