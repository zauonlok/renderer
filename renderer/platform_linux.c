#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include "platform.h"
#include "image.h"

/* data structures */

typedef struct context context_t;

struct window {
    Window handle;
    int should_close;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    context_t *context;
};

struct context {
    image_t *framebuffer;
    XImage *ximage;
};

/* window stuff */

static Display *g_display = NULL;
static int g_screen;
static Atom g_WM_PROTOCOLS;
static Atom g_WM_DELETE_WINDOW;
static XContext g_context;

static void open_display(void) {
    if (g_display == NULL) {
        g_display = XOpenDisplay(NULL);
        assert(g_display != NULL);
        g_screen = XDefaultScreen(g_display);
        g_WM_PROTOCOLS = XInternAtom(g_display, "WM_PROTOCOLS", True);
        g_WM_DELETE_WINDOW = XInternAtom(g_display, "WM_DELETE_WINDOW", True);
        g_context = XUniqueContext();
    }
}

static Window create_window(const char *title, int width, int height) {
    Window root, window;
    unsigned long black, white;
    XTextProperty property;
    XSizeHints *size_hints;
    XClassHint *class_hints;
    long mask;

    root = XRootWindow(g_display, g_screen);
    black = XBlackPixel(g_display, g_screen);
    white = XWhitePixel(g_display, g_screen);
    window = XCreateSimpleWindow(g_display, root, 0, 0, width, height, 0,
                                 white, black);

    /* not resizable */
    size_hints = XAllocSizeHints();
    size_hints->flags      = PMinSize | PMaxSize;
    size_hints->min_width  = width;
    size_hints->max_width  = width;
    size_hints->min_height = height;
    size_hints->max_height = height;
    XSetWMNormalHints(g_display, window, size_hints);
    XFree(size_hints);

    /* title bar name */
    XStringListToTextProperty((char**)&title, 1, &property);
    XSetWMName(g_display, window, &property);
    XSetWMIconName(g_display, window, &property);

    /* application name */
    class_hints = XAllocClassHint();
    class_hints->res_name  = (char*)title;
    class_hints->res_class = (char*)title;
    XSetClassHint(g_display, window, class_hints);
    XFree(class_hints);

    /* event subscription */
    mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;
    XSelectInput(g_display, window, mask);
    XSetWMProtocols(g_display, window, &g_WM_DELETE_WINDOW, 1);

    return window;
}

static context_t *create_context(int width, int height) {
    Visual *visual = XDefaultVisual(g_display, g_screen);
    int depth = XDefaultDepth(g_display, g_screen);
    image_t *framebuffer = image_create(width, height, 4);
    unsigned char *buffer = framebuffer->buffer;
    XImage *ximage;
    context_t *context;

    assert(depth == 24 || depth == 32);
    ximage = XCreateImage(g_display, visual, depth, ZPixmap, 0,
                          (char*)buffer, width, height, 32, 0);

    context = (context_t*)malloc(sizeof(context_t));
    context->framebuffer = framebuffer;
    context->ximage      = ximage;
    return context;
}

window_t *window_create(const char *title, int width, int height) {
    Window handle;
    context_t *context;
    window_t *window;

    assert(width > 0 && height > 0);

    open_display();
    handle = create_window(title, width, height);
    context = create_context(width, height);

    window = (window_t*)malloc(sizeof(window_t));
    window->handle       = handle;
    window->should_close = 0;
    window->context      = context;
    memset(window->keys, 0, sizeof(window->keys));
    memset(window->buttons, 0, sizeof(window->buttons));

    XSaveContext(g_display, handle, g_context, (XPointer)window);
    XMapWindow(g_display, handle);
    XFlush(g_display);
    return window;
}

void window_destroy(window_t *window) {
    context_t *context = window->context;

    XUnmapWindow(g_display, window->handle);
    XDeleteContext(g_display, window->handle, g_context);

    context->ximage->data = NULL;
    XDestroyImage(context->ximage);

    XDestroyWindow(g_display, window->handle);
    XFlush(g_display);

    image_release(context->framebuffer);
    free(context);
    free(window);
}

int window_should_close(window_t *window) {
    return window->should_close;
}

void image_blit_bgr(image_t *src, image_t *dst);  /* implemented in image.c */

void window_draw_image(window_t *window, image_t *image) {
    GC gc = XDefaultGC(g_display, g_screen);
    context_t *context = window->context;
    image_t *framebuffer = context->framebuffer;

    image_blit_bgr(image, framebuffer);

    XPutImage(g_display, window->handle, gc, context->ximage,
              0, 0, 0, 0, framebuffer->width, framebuffer->height);
    XFlush(g_display);
}

/* input stuff */

static void handle_key_event(window_t *window, int keycode, char action) {
    KeySym keysym, *keysyms;
    keycode_t key;
    int dummy;

    keysyms = XGetKeyboardMapping(g_display, keycode, 1, &dummy);
    keysym = keysyms[0];
    XFree(keysyms);

    switch (keysym) {
        case XK_a: key = KEY_A;   break;
        case XK_d: key = KEY_D;   break;
        case XK_s: key = KEY_S;   break;
        case XK_w: key = KEY_W;   break;
        default:   key = KEY_NUM; break;
    }
    if (key < KEY_NUM) {
        window->keys[key] = action;
    }
}

static void handle_button_event(window_t *window, int button, char action) {
    if (button == Button1) {
        window->buttons[BUTTON_L] = action;
    } else if (button == Button3) {
        window->buttons[BUTTON_R] = action;
    }
}

static void process_event(XEvent *event) {
    Window handle;
    window_t *window;
    int status;

    handle = event->xany.window;
    status = XFindContext(g_display, handle, g_context, (XPointer*)&window);
    if (status != 0) {
        return;
    }

    if (event->type == ClientMessage) {
        if (event->xclient.message_type == g_WM_PROTOCOLS) {
            Atom protocol = event->xclient.data.l[0];
            if (protocol == g_WM_DELETE_WINDOW) {
                window->should_close = 1;
            }
        }
    } else if (event->type == KeyPress) {
        handle_key_event(window, event->xkey.keycode, 1);
    } else if (event->type == KeyRelease) {
        handle_key_event(window, event->xkey.keycode, 0);
    } else if (event->type == ButtonPress) {
        handle_button_event(window, event->xbutton.button, 1);
    } else if (event->type == ButtonRelease) {
        handle_button_event(window, event->xbutton.button, 0);
    }
}

void input_poll_events(void) {
    int count = XPending(g_display);
    while (count > 0) {
        XEvent event;
        XNextEvent(g_display, &event);
        process_event(&event);
        count -= 1;
    }
    XFlush(g_display);
}

int input_key_pressed(window_t *window, keycode_t key) {
    assert(key < KEY_NUM);
    return window->keys[key];
}

int input_button_pressed(window_t *window, button_t button) {
    assert(button < BUTTON_NUM);
    return window->buttons[button];
}

void input_query_cursor(window_t *window, int *xpos, int *ypos) {
    Window root, child;
    int root_x, root_y, window_x, window_y;
    unsigned int mask;

    XQueryPointer(g_display, window->handle, &root, &child,
                  &root_x, &root_y, &window_x, &window_y, &mask);
    if (xpos != NULL) {
        *xpos = window_x;
    }
    if (ypos != NULL) {
        *ypos = window_y;
    }
}

/* time stuff */

double time_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

void time_sleep_for(int milliseconds) {
    struct timespec ts;
    assert(milliseconds > 0);
    ts.tv_sec  = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
