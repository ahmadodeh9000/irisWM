#include "iriswm.h"

#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>

static irisWM iwm;

static void init();
static void event_loop();


int main() {
    init();
    event_loop();
    return 0;
}



void init(){
    


    if (!(iwm.dpy = XOpenDisplay(NULL))) {
        fprintf(stderr,"[ERROR] : failed to connect to X11 display\n");
        exit(1);
    }


    iwm.root                = DefaultRootWindow(iwm.dpy);
    iwm.screen_width        = DisplayWidth(iwm.dpy,DefaultScreen(iwm.dpy));
    iwm.screen_height       = DisplayHeight(iwm.dpy,DefaultScreen(iwm.dpy));
    iwm.window_count        = 0;
    iwm.focused_index       = 0;
    iwm.window_capacity     = 1;
    iwm.windows             = malloc(sizeof(Window) * iwm.window_capacity);

// this is for detection
/***************************************************************************************************/
    Atom net_supporting_wm_check = XInternAtom(iwm.dpy, "_NET_SUPPORTING_WM_CHECK", False);
    Atom net_wm_name = XInternAtom(iwm.dpy, "_NET_WM_NAME", False);
    Atom utf8_string = XInternAtom(iwm.dpy, "UTF8_STRING", False);

    Window check_win = XCreateSimpleWindow(iwm.dpy, iwm.root, 0, 0, 1, 1, 0, 0, 0);

    XChangeProperty(iwm.dpy, iwm.root, net_supporting_wm_check, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&check_win, 1);

    XChangeProperty(iwm.dpy, check_win, net_supporting_wm_check, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&check_win, 1);

    XChangeProperty(iwm.dpy, check_win, net_wm_name, utf8_string, 8,
                    PropModeReplace, (unsigned char *)"irisWM", 6);
/***************************************************************************************************/


    printf("[irisWM]: Connected to X11 : %dx%d\n",iwm.screen_width,iwm.screen_height);

    

    XSelectInput(iwm.dpy,iwm.root,SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);

    setup_keybindings(&iwm);

}


static void event_loop() {
    XEvent event;


    for (;;) {
        XNextEvent(iwm.dpy,&event);

        switch(event.type) {


            case MapRequest: {
                Window w = event.xmaprequest.window;
                add_window(&iwm, w);

                XMapWindow(iwm.dpy, event.xmaprequest.window);
                break;
            }

            case KeyPress: {
                on_key_press(&iwm,(XKeyEvent*) &event);
                break;
            }

            case ConfigureRequest: {
                XConfigureRequestEvent *ev = &event.xconfigurerequest;

                XWindowChanges wc;

                wc.x = ev->x;
                wc.y = ev->y;
                wc.width = ev->width;
                wc.height = ev->height;
                wc.border_width = ev->border_width;
                wc.sibling = ev->above;
                wc.stack_mode = ev->detail;

                XConfigureWindow(
                    iwm.dpy,
                    ev->window,
                    ev->value_mask,
                    &wc
                );

                break;
            }

            case DestroyNotify: {
                destroy_window(&iwm,event.xdestroywindow.window);
                break;
            }


        }



    }

    XCloseDisplay(iwm.dpy);
    free(iwm.windows);


}



