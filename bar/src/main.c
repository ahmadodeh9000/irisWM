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

    clean_ibar(&ibar);

    return 0;

}