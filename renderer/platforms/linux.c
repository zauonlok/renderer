#include "../core/platform.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include "../core/graphics.h"
#include "../core/image.h"

struct window {
    Window handle;
    int closing;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    image_t *surface;
    XImage *ximage;
};

/* window related functions */

static Display *g_display = NULL;
static XContext g_context;

static void open_display(void) {
    if (g_display == NULL) {
        g_display = XOpenDisplay(NULL);
        assert(g_display != NULL);
        g_context = XUniqueContext();
    }
}

static Window create_window(const char *title, int width, int height) {
    Atom wm_delete_window = XInternAtom(g_display, "WM_DELETE_WINDOW", True);
    int screen = XDefaultScreen(g_display);
    unsigned long border = XWhitePixel(g_display, screen);
    unsigned long background = XBlackPixel(g_display, screen);
    Window root = XRootWindow(g_display, screen);
    Window window;
    XSizeHints *size_hints;
    XClassHint *class_hint;
    long mask;

    window = XCreateSimpleWindow(g_display, root, 0, 0, width, height, 0,
                                 border, background);

    /* not resizable */
    size_hints = XAllocSizeHints();
    size_hints->flags      = PMinSize | PMaxSize;
    size_hints->min_width  = width;
    size_hints->max_width  = width;
    size_hints->min_height = height;
    size_hints->max_height = height;
    XSetWMNormalHints(g_display, window, size_hints);
    XFree(size_hints);

    /* application name */
    class_hint = XAllocClassHint();
    class_hint->res_name  = (char*)title;
    class_hint->res_class = (char*)title;
    XSetClassHint(g_display, window, class_hint);
    XFree(class_hint);

    /* event subscription */
    mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;
    XSelectInput(g_display, window, mask);
    XSetWMProtocols(g_display, window, &wm_delete_window, 1);

    return window;
}

static void create_surface(int width, int height,
                           image_t **out_surface, XImage **out_ximage) {
    int screen = XDefaultScreen(g_display);
    int depth = XDefaultDepth(g_display, screen);
    Visual *visual = XDefaultVisual(g_display, screen);
    image_t *surface = image_create(width, height, 4);
    XImage *ximage;

    assert(depth == 24 || depth == 32);
    ximage = XCreateImage(g_display, visual, depth, ZPixmap, 0,
                          (char*)surface->buffer, width, height, 32, 0);

    *out_surface = surface;
    *out_ximage = ximage;
}

window_t *window_create(const char *title, int width, int height) {
    window_t *window;
    Window handle;
    image_t *surface;
    XImage *ximage;

    assert(width > 0 && height > 0);

    open_display();
    handle = create_window(title, width, height);
    create_surface(width, height, &surface, &ximage);

    window = (window_t*)malloc(sizeof(window_t));
    window->handle  = handle;
    window->closing = 0;
    window->surface = surface;
    window->ximage  = ximage;
    memset(window->keys, 0, sizeof(window->keys));
    memset(window->buttons, 0, sizeof(window->buttons));

    XSaveContext(g_display, handle, g_context, (XPointer)window);
    XMapWindow(g_display, handle);
    XFlush(g_display);
    return window;
}

void window_destroy(window_t *window) {
    XUnmapWindow(g_display, window->handle);
    XDeleteContext(g_display, window->handle, g_context);

    window->ximage->data = NULL;
    XDestroyImage(window->ximage);
    XDestroyWindow(g_display, window->handle);
    XFlush(g_display);

    image_release(window->surface);
    free(window);
}

int window_should_close(window_t *window) {
    return window->closing;
}

void window_draw_image(window_t *window, image_t *image) {
    int screen = XDefaultScreen(g_display);
    GC gc = XDefaultGC(g_display, screen);
    image_t *surface = window->surface;
    image_blit_bgr(image, surface);
    XPutImage(g_display, window->handle, gc, window->ximage,
              0, 0, 0, 0, surface->width, surface->height);
    XFlush(g_display);
}

void window_draw_buffer(window_t *window, colorbuffer_t *buffer) {
    int screen = XDefaultScreen(g_display);
    GC gc = XDefaultGC(g_display, screen);
    image_t *surface = window->surface;
    colorbuffer_blit_bgr(buffer, surface);
    XPutImage(g_display, window->handle, gc, window->ximage,
              0, 0, 0, 0, surface->width, surface->height);
    XFlush(g_display);
}

/* input related functions */

static const char ACTION_UP = 0;
static const char ACTION_DOWN = 1;

static void handle_key_event(window_t *window, int virtual_key, char action) {
    KeySym keysym, *keysyms;
    keycode_t key;
    int dummy;

    keysyms = XGetKeyboardMapping(g_display, virtual_key, 1, &dummy);
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
    window_t *window;
    int error = XFindContext(g_display, event->xany.window,
                             g_context, (XPointer*)&window);
    if (error != 0) {
        return;
    }

    if (event->type == ClientMessage) {
        Atom wm_protocols = XInternAtom(g_display, "WM_PROTOCOLS", True);
        if (event->xclient.message_type == wm_protocols) {
            Atom protocol = event->xclient.data.l[0];
            Atom wm_delete_window = XInternAtom(g_display, "WM_DELETE_WINDOW",
                                                True);
            if (protocol == wm_delete_window) {
                window->closing = 1;
            }
        }
    } else if (event->type == KeyPress) {
        handle_key_event(window, event->xkey.keycode, ACTION_DOWN);
    } else if (event->type == KeyRelease) {
        handle_key_event(window, event->xkey.keycode, ACTION_UP);
    } else if (event->type == ButtonPress) {
        handle_button_event(window, event->xbutton.button, ACTION_DOWN);
    } else if (event->type == ButtonRelease) {
        handle_button_event(window, event->xbutton.button, ACTION_UP);
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
    assert(key >= 0 && key < KEY_NUM);
    return window->keys[key];
}

int input_button_pressed(window_t *window, button_t button) {
    assert(button >= 0 && button < BUTTON_NUM);
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

/* time related functions */

double timer_get_time(void) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
#endif
}
