#ifndef __MONITOR_H__
#define __MONITOR_H__

typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * texture;
    volatile bool sdl_refr_qry;
#if SDL_DOUBLE_BUFFERED
    uint32_t * tft_fb_act;
#else
    uint32_t * tft_fb;
#endif
} monitor_t;

#endif
