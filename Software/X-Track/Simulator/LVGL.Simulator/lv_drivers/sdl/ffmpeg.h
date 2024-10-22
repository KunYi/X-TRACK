
#ifndef __FFMPEG_H_
#define __FFMPEG_H_

void ffmpeg_init(int width, int height, const char* filename);
void ffmpeg_close(void);
void capture(int width, int height, monitor_t *m);

#endif
