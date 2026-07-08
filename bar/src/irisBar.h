#ifndef iris_bar_h
#define iris_bar_h

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdint.h>


/* constants */
#define BAR_HEIGHT 24
#define FONT_NAME  "monospace-10"
#define BG_COLOR   "#222222"
#define FG_COLOR   "#ffffff"


typedef struct {
    Display* dpy;
    Window win;

    int32_t screen;
    int32_t screen_width;

    XftFont *font;
    XftColor fg_color;
    XftDraw *xft_draw;

    GC gc;
    unsigned long bg_pixel;
    
    Pixmap pixmap;    

} irisBar;

void init(irisBar* ibar);
void redraw(irisBar* ibar);
void test();




#endif