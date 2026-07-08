/*  *   *   *   *   *   *   *   *   *   *   *
*   A Simple Status Bar in C + Xlib         *
*   made by Ahmad Odeh                      *
*                                           *       
*   *   *   *   *   *   *   *   *   *   *   */


#include "irisBar.h"


static irisBar ibar;

int main() {
    init(&ibar);

    for (;;) {

        while (XPending(ibar.dpy)) {
            XEvent event;
            XNextEvent(ibar.dpy,&event);

            if (Expose == event.type) {
                redraw(&ibar);
            }
        }

        redraw(&ibar);

    }

    XftDrawDestroy(ibar.xft_draw);
    XftFontClose(ibar.dpy, ibar.font);
    XFreePixmap(ibar.dpy, ibar.pixmap);
    XCloseDisplay(ibar.dpy);

    return 0;

}