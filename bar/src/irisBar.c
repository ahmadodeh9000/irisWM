#include "irisBar.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>


/*  *   *   *   *   *   *   *   *   *   *
*                                       *
*                                       *
*         System getters fuctions       *
*                                       *
*                                       *
*  *   *    *   *   *   *   *   *   *   */



static int32_t get_wifi_quality();
static void get_wifi_bar(int32_t quality, char *buf, size_t len); 
static char* get_username();
void get_time_str(char *buf, size_t len);
void draw_text(irisBar* bar,const char *text, int x, int y);


void draw_text(irisBar* bar,const char *text, int x, int y) {
    XftDrawStringUtf8(bar->xft_draw, &bar->fg_color, bar->font,
                       x, y, (const FcChar8 *)text, strlen(text));
}




/* wifi */
int get_wifi_quality(void) {
    FILE *fp = fopen("/proc/net/wireless", "r");
    if (!fp)
        return -1;

    char line[256];

    // Skip the first two header lines
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    char iface[32];
    int status;
    float quality;

    if (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, " %31[^:]: %d %f",
                   iface,
                   &status,
                   &quality) == 3) {
            fclose(fp);
            return (int)quality;
        }
    }

    fclose(fp);
    return -1;
}

static void get_wifi_bar(int32_t quality, char *buf, size_t len) {
    if (quality < 0)  quality = 0;
    if (quality > 70) quality = 70;

    int filled = (quality * 8 + 35) / 70;

    // "[########]" = 1 + 8 + 1 + null = 11 bytes minimum
    if (len < 11) return;

    buf[0] = '[';
    for (int i = 0; i < 8; i++)
        buf[i + 1] = (i < filled) ? '#' : '*';
    buf[9] = ']';
    buf[10] = '\0';
}



// get the user name

static char* get_username() {
    register struct passwd* pw;
    register uid_t uid;

    uid     = getuid();
    pw      = getpwuid(uid);
    
    
    if (!pw) {
        fprintf(
            stderr,
            "[irisBar]: failed to get the user name !\n"
        );
        exit(1);
    }

    return pw->pw_name;
}


// time


void get_time_str(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm_info);
}




// draw


void redraw(irisBar* ibar) {

    XSetForeground(ibar->dpy, ibar->gc, ibar->bg_pixel);
    XFillRectangle(ibar->dpy, ibar->pixmap, ibar->gc, 0, 0, ibar->screen_width, BAR_HEIGHT);
    //XClearWindow(ibar->dpy,ibar->win);

    char wifi_buf[16];
    char wifi_display[32];
    char time_buf[64];

    get_wifi_bar(get_wifi_quality(),wifi_buf,sizeof(wifi_buf));
    get_time_str(time_buf,sizeof(time_buf));

    int baseline_y = (BAR_HEIGHT + ibar->font->ascent - ibar->font->descent) / 2;

    // Left
    draw_text(ibar,get_username(), 0, baseline_y);
 
    // Center
    int wifi_x = ibar->screen_width / 2 - 40;
    snprintf(wifi_display, sizeof(wifi_display), "wifi: %s", wifi_buf);
    draw_text(ibar,wifi_display, wifi_x, baseline_y);
 
    // Right
    int time_x = ibar->screen_width - 120;
    draw_text(ibar,time_buf, time_x, baseline_y);

    XCopyArea(ibar->dpy, ibar->pixmap, ibar->win, ibar->gc, 0, 0, ibar->screen_width, BAR_HEIGHT, 0, 0);
 
    XFlush(ibar->dpy);


}




// init
void init(irisBar* ibar) {
    if (!(ibar->dpy = XOpenDisplay(NULL))) {
        fprintf(stderr,"[irisBar]: failed to connet to Display !\n");
        exit(1);
    }

    ibar->screen         = DefaultScreen(ibar->dpy);
    ibar->screen_width   = DisplayWidth(ibar->dpy,ibar->screen);

    Colormap cmap = DefaultColormap(ibar->dpy, ibar->screen);
    XColor bg;
    XParseColor(ibar->dpy, cmap, BG_COLOR, &bg);
    XAllocColor(ibar->dpy, cmap, &bg);
    ibar->bg_pixel = bg.pixel;




    /* Create the bar window at the top of the screen */
    ibar->win = XCreateSimpleWindow(
        ibar->dpy, DefaultRootWindow(ibar->dpy),
        0, 0,                       // x, y
        ibar->screen_width, BAR_HEIGHT,
        0,                          // border width
        0,                          // border color
        ibar->bg_pixel               // background color
    );

    ibar->pixmap = XCreatePixmap(ibar->dpy, ibar->win, ibar->screen_width, BAR_HEIGHT,
                            DefaultDepth(ibar->dpy, ibar->screen));


    // Tell the WM we don't want to tile/manage irisBar like a normal window
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(ibar->dpy, ibar->win, CWOverrideRedirect, &attrs);
 
    XSelectInput(ibar->dpy, ibar->win, ExposureMask);
    XMapWindow(ibar->dpy, ibar->win);
 
    // Graphics context for basic drawing (rects, etc-> if needed later)
    ibar->gc = XCreateGC(ibar->dpy, ibar->win, 0, NULL);
    
    // setting the font 
    ibar->font = XftFontOpenName(ibar->dpy,ibar->screen,FONT_NAME);

    if (!ibar->font) {
        fprintf(
            stderr,
            "[irisBar]: failed to load font \n"
        );
        exit(1);
    }

     
    XftColorAllocName(ibar->dpy, DefaultVisual(ibar->dpy, ibar->screen), cmap, FG_COLOR, &ibar->fg_color);
    ibar->xft_draw = XftDrawCreate(ibar->dpy, ibar->pixmap, DefaultVisual(ibar->dpy, ibar->screen), cmap);
 
    printf("[irisBar]: initialized, width=%d\n", ibar->screen_width);

}