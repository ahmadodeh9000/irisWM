#include "iriswm.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/keysym.h>

static void tile(irisWM* wm);

void add_window(irisWM* wm,Window w) {
    
    if (wm->window_count >= wm->window_capacity) {
        wm->window_capacity *= 2;
        wm->windows = realloc(wm->windows,sizeof(Window) * wm->window_capacity);
    }

    wm->windows[wm->window_count++] = w;
    wm->focused_index = wm->window_count - 1; // the focused window is the newest window


    /* for debugging */
    printf("[irisWM]: [+] Window %ld added (total: %d)\n",w,wm->window_count);
    tile(wm);


}


void destroy_window(irisWM *wm, Window w) {
    
    int32_t i;
    for (i = 0; i < wm->window_count; ++i) {
        if (wm->windows[i] == w) break;
    }

    if (i == wm->window_count) return;                     

    for (int32_t j = i; j < wm->window_count - 1; ++j) {
        wm->windows[j] = wm->windows[j + 1];
    }

    wm->window_count--;

    if (wm->focused_index >= wm->window_count && wm->window_count > 0) {
        wm->focused_index = wm->window_count - 1;
    }

    printf("[irisWM]: [-] Window %ld removed (total: %d)\n",w,wm->window_count);
    tile(wm);

}


void focus_window(irisWM* wm, int32_t index) {
    if (index < 0 || index >= wm->window_count) return;
    wm->focused_index = index;

    Window focused_window = wm->windows[index];

    XSetInputFocus(wm->dpy,focused_window,RevertToPointerRoot,CurrentTime);
    XRaiseWindow(wm->dpy,focused_window);

    printf("irisWM: [$] Focused window %ld\n",focused_window);
}


static void tile(irisWM* wm) {
    if (wm->window_count == 0) return;

    if (wm->window_count == 1) {
        XMoveResizeWindow(wm->dpy,wm->windows[0],0,0,wm->screen_width,wm->screen_height);
        return;
    }

    int32_t master_width = (wm->screen_width * 60) / 100;
    int32_t stack_width   = wm->screen_width - master_width;
    int32_t stack_count   = wm->window_count - 1;
    int32_t stack_height  = wm->screen_height / stack_count;


    XMoveResizeWindow(wm->dpy,wm->windows[0],0,0,master_width,wm->screen_height);
        for (int32_t i = 1; i < wm->window_count; i++) {
        int32_t y = (i - 1) * stack_height;
        XMoveResizeWindow(wm->dpy, wm->windows[i], master_width, y, stack_width, stack_height);
    }

    
}



/******************************************
 *                                        *
 *          KEYS HANDLING                 *
 *                                        *
 ******************************************/

// constants 
#define MOD Mod4Mask    // superkey (Windows button for e.g)



// key actions
static void spawn_terminal(irisWM* wm);
static void focus_next(irisWM* wm);
static void focus_prev(irisWM* wm);


static void spawn_terminal(irisWM* wm) {

    if (NULL == wm) return;
    pid_t pid = fork();

    if (0 == pid) {
        execvp("xterm", (char *[]){"xterm", NULL});
        exit(1);
    }

    printf("[!] Spawned xterm (pid=%d)\n", pid);
}


static void close_focused(irisWM* wm) {
    if (wm->window_count == 0) return;

    Window w = wm->windows[wm->focused_index];
    XKillClient(wm->dpy,w);

}


static void focus_next(irisWM* wm) {
    if (0 == wm->window_count) return;

    int32_t next = (wm->focused_index + 1) % wm->window_count;
    focus_window(wm,next);
}

static void focus_prev(irisWM* wm) {
    if (0 == wm->window_count) return;

    int32_t prev = (wm->focused_index - 1 + wm->window_count) % wm->window_count;
    focus_window(wm,prev);
}



void setup_keybindings(irisWM* wm) {
    XUngrabKey(wm->dpy,AnyKey,AnyModifier,wm->root);

    struct {
        KeySym keysym;
        void (*handler)(irisWM*);
    } keys[] = {
        { XK_Return, spawn_terminal },  // windows + enter
        { XK_q,      close_focused },   // windows + q
        { XK_j,      focus_next },      // windows + j
        { XK_k,      focus_prev },      // windows + k
    };

    int32_t nkeys = sizeof(keys) / sizeof(keys[0]);
    

    unsigned int lock_masks[] = { 0, LockMask, Mod2Mask, LockMask | Mod2Mask };
    for (int i = 0; i < nkeys; i++) {
        KeyCode kc = XKeysymToKeycode(wm->dpy, keys[i].keysym);
        if (kc == 0) continue;
        for (int j = 0; j < 4; j++) {
            XGrabKey(wm->dpy, kc, MOD | lock_masks[j], wm->root, True, GrabModeAsync, GrabModeAsync);
        }
    }
    
    printf("[irisWM] Keybindings set up\n");


}


void on_key_press(irisWM* wm,XKeyEvent* kev) {
    KeySym ks = XLookupKeysym(kev,0);
    switch (ks) {
        case XK_Return: spawn_terminal(wm);   break;
        case XK_q:      close_focused(wm);  break;
        case XK_j:      focus_next(wm);     break;
        case XK_k:      focus_prev(wm);     break;
    }
}


