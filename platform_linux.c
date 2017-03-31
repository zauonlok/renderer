#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "platform.h"
#include "image.h"
#include "error.h"

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
    int width;
    int height;
    unsigned char *buffer;
    XImage *image;
};

/* window stuff */

static Display *g_display = NULL;
static int g_screen;
static Atom WM_PROTOCOLS;
static Atom WM_DELETE_WINDOW;
static XContext g_context;

static void open_display(void) {
    if (g_display == NULL) {
        g_display = XOpenDisplay(NULL);
        FORCE(g_display != NULL, "XOpenDisplay");
        g_screen = XDefaultScreen(g_display);
        WM_PROTOCOLS = XInternAtom(g_display, "WM_PROTOCOLS", True);
        WM_DELETE_WINDOW = XInternAtom(g_display, "WM_DELETE_WINDOW", True);
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
    XSetWMProtocols(g_display, window, &WM_DELETE_WINDOW, 1);

    XMapWindow(g_display, window);
    XFlush(g_display);
    return window;
}

static context_t *create_context(int width, int height) {
    Visual *visual = XDefaultVisual(g_display, g_screen);
    int depth = XDefaultDepth(g_display, g_screen);
    int bytes_per_pixel = 4;
    int buffer_size = width * height * bytes_per_pixel;
    unsigned char *buffer = (unsigned char*)malloc(buffer_size);
    XImage *image;
    context_t *context;

    FORCE(depth == 24 || depth == 32, "create_context: depth");
    image = XCreateImage(g_display, visual, depth, ZPixmap, 0, (char*)buffer,
                         width, height, 32, 0);

    context = (context_t*)malloc(sizeof(context_t));
    context->width    = width;
    context->height   = height;
    context->buffer   = buffer;
    context->image    = image;
    return context;
}

window_t *window_create(const char *title, int width, int height) {
    Window handle;
    context_t *context;
    window_t *window;

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
    return window;
}

void window_destroy(window_t *window) {
    context_t *context = window->context;
    XUnmapWindow(g_display, window->handle);
    XDeleteContext(g_display, window->handle, g_context);
    context->image->data = NULL;
    XDestroyImage(context->image);
    XDestroyWindow(g_display, window->handle);
    XFlush(g_display);
    free(context->buffer);
    free(context);
    free(window);
}

int window_should_close(window_t *window) {
    return window->should_close;
}

void window_draw_image(window_t *window, image_t *image) {
    GC gc = XDefaultGC(g_display, g_screen);
    context_t *context = window->context;
    int bytes_per_pixel = 4;
    int bytes_per_row = context->width * bytes_per_pixel;
    int buffer_size = context->height * bytes_per_row;
    int channels = image->channels;
    int r, c;

    if (channels != 1 && channels != 3 && channels != 4) {
        FATAL("window_draw_image: channels");
    }
    memset(context->buffer, 0, buffer_size);
    for (r = 0; r < context->height && r < image->height; r++) {
        for (c = 0; c < context->width && c < image->width; c++) {
            int context_index = r * bytes_per_row + c * bytes_per_pixel;
            unsigned char *context_pixel = &(context->buffer[context_index]);
            unsigned char *image_pixel = image_pixel_ptr(image, r, c);
            if (channels == 1) {
                context_pixel[0] = image_pixel[0];
                context_pixel[1] = image_pixel[0];
                context_pixel[2] = image_pixel[0];
            } else {
                context_pixel[0] = image_pixel[0];
                context_pixel[1] = image_pixel[1];
                context_pixel[2] = image_pixel[2];
            }
        }
    }

    XPutImage(g_display, window->handle, gc, context->image,
              0, 0, 0, 0, context->width, context->height);
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
        if (event->xclient.message_type == WM_PROTOCOLS) {
            Atom protocol = event->xclient.data.l[0];
            if (protocol == WM_DELETE_WINDOW) {
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
    return window->keys[key];
}

int input_button_pressed(window_t *window, button_t button) {
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

double input_get_time(void) {
#if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
#endif
}
