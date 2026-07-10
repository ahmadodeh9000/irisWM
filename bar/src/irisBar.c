#include "irisBar.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>

#include <sys/stat.h>
#include <dirent.h>


/*  *   *   *   *   *   *   *   *   *   *
*                                       *
*                                       *
*         System getters fuctions       *
*                                       *
*                                       *
*  *   *    *   *   *   *   *   *   *   */


#define WIFI_MAX_QUALITY 70
#define WIFI_BARS 8
#define RIGHT_PADDING 10

static int32_t get_wifi_quality(void);
static int32_t is_wireless_iface(const char *iface);
static void detect_connection_type(char *type_buf, size_t len);
static void get_wifi_bar(int32_t quality, char *buf, size_t len);


static const char* get_username(void);
void get_time_str(char *buf, size_t len);
void draw_text(irisBar* bar,const char *text, int x, int y);


void draw_text(irisBar* bar,const char *text, int x, int y) {
    XftDrawStringUtf8(bar->xft_draw, &bar->fg_color, bar->font,
                       x, y, (const FcChar8 *)text, strlen(text));
}


static int32_t is_wireless_iface(const char *iface) {
    char path[320];
    snprintf(path, sizeof(path), "/sys/class/net/%s/wireless", iface);

    struct stat st;
    if (stat(path, &st) == 0) {
        return 1;   // wireless subfolder exists -> it's wifi
    }
    return 0;       // no wireless folder -> ethernet or other
}


static void detect_connection_type(char *type_buf, size_t len) {
    DIR *d = opendir("/sys/class/net");
    if (!d) {
        snprintf(type_buf, len, "unknown");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;          // skip . and ..
        if (strcmp(entry->d_name, "lo") == 0) continue; // skip loopback

        // Check if this interface is "up" (has carrier)
        char carrier_path[300];
        snprintf(carrier_path, sizeof(carrier_path), "/sys/class/net/%s/carrier", entry->d_name);

        FILE *fp = fopen(carrier_path, "r");
        if (!fp) continue;

        char carrier_val[4] = {0};
        fread(carrier_val, 1, sizeof(carrier_val) - 1, fp);
        fclose(fp);

        if (carrier_val[0] != '1') continue;  // not connected/up

        // Found an active interface -> check its type
        if (is_wireless_iface(entry->d_name)) {
            snprintf(type_buf, len, "wifi (%s)", entry->d_name);
        } else {
            snprintf(type_buf, len, "ethernet (%s)", entry->d_name);
        }

        closedir(d);
        return;
    }

    closedir(d);
    snprintf(type_buf, len, "disconnected");
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
    if (quality > WIFI_MAX_QUALITY) quality = WIFI_MAX_QUALITY;

    int filled = (quality * WIFI_BARS + (WIFI_MAX_QUALITY / 2)) / WIFI_MAX_QUALITY;

    // "[########]" = 1 + 8 + 1 + null = 11 bytes minimum
    if (len < 11) return;

    buf[0] = '[';
    for (int i = 0; i < WIFI_BARS; i++)
        buf[i + 1] = (i < filled) ? '#' : '*';
    buf[WIFI_BARS + 1] = ']';
    buf[WIFI_BARS + 2] = '\0';
}



// get the user name

static const char* get_username() {
    struct passwd* pw;
    uid_t uid;

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

    char conn_buf[364];
    char wifi_buf[16];
    char wifi_display[32];
    char time_buf[64];

    detect_connection_type(conn_buf, sizeof(conn_buf));
    get_time_str(time_buf,sizeof(time_buf));

    int baseline_y = (BAR_HEIGHT + ibar->font->ascent - ibar->font->descent) / 2;

    // Left
    draw_text(ibar,ibar->username,0 , baseline_y);
    
    // Center
    int center_x = ibar->screen_width / 2 - 40;
    if (strncmp(conn_buf, "wifi", 4) == 0) {
        get_wifi_bar(get_wifi_quality(), wifi_buf, sizeof(wifi_buf));
        snprintf(wifi_display, sizeof(wifi_display), "wifi: %s", wifi_buf);
        draw_text(ibar, wifi_display, center_x, baseline_y);
    } else {
        draw_text(ibar, conn_buf, center_x, baseline_y);  // e.g. "ethernet (enp0s3)" or "disconnected"
    }
 
    // Right
    XGlyphInfo extents;
    XftTextExtentsUtf8(ibar->dpy, ibar->font, (const FcChar8 *)time_buf, strlen(time_buf), &extents);
    int time_x = ibar->screen_width - extents.xOff - RIGHT_PADDING;
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
    ibar->username       = get_username();

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


void clean_ibar(irisBar* ibar) {
    XftDrawDestroy(ibar->xft_draw);
    XftFontClose(ibar->dpy, ibar->font);
    XFreePixmap(ibar->dpy, ibar->pixmap);
    XFreeGC(ibar->dpy,ibar->gc);
    XDestroyWindow(ibar->dpy,ibar->win);
    XCloseDisplay(ibar->dpy);
}
