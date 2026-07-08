#ifndef iris_wm_h
#define iris_wm_h


#include <X11/Xlib.h>
#include <stdint.h>

typedef struct  {
    Display* dpy;
    Window root;
    Window* windows;

    int32_t window_count;
    int32_t window_capacity;
    int32_t focused_index;

    int32_t screen_width;
    int32_t screen_height;

} irisWM;

void add_window(irisWM *wm,Window w);
void destroy_window(irisWM* wm, Window w);
void focus_window(irisWM* wm, int32_t index);

void setup_keybindings(irisWM* wm);
void on_key_press(irisWM* wm,XKeyEvent* kev);


#endif