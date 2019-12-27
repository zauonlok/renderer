#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "../core/graphics.h"
#include "../core/image.h"
#include "../core/macro.h"
#include "../core/platform.h"
#include "../core/private.h"

struct window {
    Window handle;
    XImage *ximage;
    image_t *surface;
    /* common data */
    int should_close;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    callbacks_t callbacks;
    void *userdata;
};

/* platform initialization */

static Display *g_display = NULL;
static XContext g_context;

static void open_display(void) {
    g_display = XOpenDisplay(NULL);
    assert(g_display != NULL);
    g_context = XUniqueContext();
}

static void close_display(void) {
    XCloseDisplay(g_display);
    g_display = NULL;
}

static void initialize_path(void) {
    char path[PATH_SIZE];
    ssize_t bytes;
    int error;

    bytes = readlink("/proc/self/exe", path, PATH_SIZE - 1);
    assert(bytes != -1);
    path[bytes] = '\0';
    *strrchr(path, '/') = '\0';

    error = chdir(path);
    assert(error == 0);
    error = chdir("assets");
    assert(error == 0);
    UNUSED_VAR(error);
}

void platform_initialize(void) {
    assert(g_display == NULL);
    open_display();
    initialize_path();
}

void platform_terminate(void) {
    assert(g_display != NULL);
    close_display();
}

/* window related functions */

static Window create_window(const char *title, int width, int height) {
    int screen = XDefaultScreen(g_display);
    unsigned long border = XWhitePixel(g_display, screen);
    unsigned long background = XBlackPixel(g_display, screen);
    Window root = XRootWindow(g_display, screen);
    Window handle;
    XSizeHints *size_hints;
    XClassHint *class_hint;
    Atom delete_window;
    long mask;

    handle = XCreateSimpleWindow(g_display, root, 0, 0, width, height, 0,
                                 border, background);

    /* not resizable */
    size_hints = XAllocSizeHints();
    size_hints->flags = PMinSize | PMaxSize;
    size_hints->min_width = width;
    size_hints->max_width = width;
    size_hints->min_height = height;
    size_hints->max_height = height;
    XSetWMNormalHints(g_display, handle, size_hints);
    XFree(size_hints);

    /* application name */
    class_hint = XAllocClassHint();
    class_hint->res_name = (char*)title;
    class_hint->res_class = (char*)title;
    XSetClassHint(g_display, handle, class_hint);
    XFree(class_hint);

    /* event subscription */
    mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;
    XSelectInput(g_display, handle, mask);
    delete_window = XInternAtom(g_display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(g_display, handle, &delete_window, 1);

    return handle;
}

static void create_surface(int width, int height,
                           image_t **out_surface, XImage **out_ximage) {
    int screen = XDefaultScreen(g_display);
    int depth = XDefaultDepth(g_display, screen);
    Visual *visual = XDefaultVisual(g_display, screen);
    image_t *surface;
    XImage *ximage;

    assert(depth == 24 || depth == 32);
    surface = image_create(width, height, 4, FORMAT_LDR);
    ximage = XCreateImage(g_display, visual, depth, ZPixmap, 0,
                          (char*)surface->ldr_buffer, width, height, 32, 0);

    *out_surface = surface;
    *out_ximage = ximage;
}

window_t *window_create(const char *title, int width, int height) {
    window_t *window;
    Window handle;
    image_t *surface;
    XImage *ximage;

    assert(g_display && width > 0 && height > 0);

    handle = create_window(title, width, height);
    create_surface(width, height, &surface, &ximage);

    window = (window_t*)malloc(sizeof(window_t));
    memset(window, 0, sizeof(window_t));
    window->handle = handle;
    window->ximage = ximage;
    window->surface = surface;

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
    return window->should_close;
}

void window_set_userdata(window_t *window, void *userdata) {
    window->userdata = userdata;
}

void *window_get_userdata(window_t *window) {
    return window->userdata;
}

static void present_surface(window_t *window) {
    int screen = XDefaultScreen(g_display);
    GC gc = XDefaultGC(g_display, screen);
    image_t *surface = window->surface;
    XPutImage(g_display, window->handle, gc, window->ximage,
              0, 0, 0, 0, surface->width, surface->height);
    XFlush(g_display);
}

void window_draw_buffer(window_t *window, framebuffer_t *buffer) {
    private_blit_bgr(buffer, window->surface);
    present_surface(window);
}

/* input related functions */

static void handle_key_event(window_t *window, int virtual_key, char pressed) {
    KeySym *keysyms;
    KeySym keysym;
    keycode_t key;
    int dummy;

    keysyms = XGetKeyboardMapping(g_display, virtual_key, 1, &dummy);
    keysym = keysyms[0];
    XFree(keysyms);

    switch (keysym) {
        case XK_a:     key = KEY_A;     break;
        case XK_d:     key = KEY_D;     break;
        case XK_s:     key = KEY_S;     break;
        case XK_w:     key = KEY_W;     break;
        case XK_space: key = KEY_SPACE; break;
        default:       key = KEY_NUM;   break;
    }
    if (key < KEY_NUM) {
        window->keys[key] = pressed;
        if (window->callbacks.key_callback) {
            window->callbacks.key_callback(window, key, pressed);
        }
    }
}

static void handle_button_event(window_t *window, int xbutton, char pressed) {
    if (xbutton == Button1 || xbutton == Button3) {         /* mouse button */
        button_t button = xbutton == Button1 ? BUTTON_L : BUTTON_R;
        window->buttons[button] = pressed;
        if (window->callbacks.button_callback) {
            window->callbacks.button_callback(window, button, pressed);
        }
    } else if (xbutton == Button4 || xbutton == Button5) {  /* mouse wheel */
        if (window->callbacks.scroll_callback) {
            float offset = xbutton == Button4 ? 1 : -1;
            window->callbacks.scroll_callback(window, offset);
        }
    }
}

static void handle_client_event(window_t *window, XClientMessageEvent *event) {
    static Atom protocols = None;
    static Atom delete_window = None;
    if (protocols == None) {
        protocols = XInternAtom(g_display, "WM_PROTOCOLS", True);
        delete_window = XInternAtom(g_display, "WM_DELETE_WINDOW", True);
        assert(protocols != None);
        assert(delete_window != None);
    }
    if (event->message_type == protocols) {
        Atom protocol = event->data.l[0];
        if (protocol == delete_window) {
            window->should_close = 1;
        }
    }
}

static void process_event(XEvent *event) {
    Window handle;
    window_t *window;
    int error;

    handle = event->xany.window;
    error = XFindContext(g_display, handle, g_context, (XPointer*)&window);
    if (error != 0) {
        return;
    }

    if (event->type == ClientMessage) {
        handle_client_event(window, &event->xclient);
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
    assert(key >= 0 && key < KEY_NUM);
    return window->keys[key];
}

int input_button_pressed(window_t *window, button_t button) {
    assert(button >= 0 && button < BUTTON_NUM);
    return window->buttons[button];
}

void input_query_cursor(window_t *window, float *xpos, float *ypos) {
    Window root, child;
    int root_x, root_y, window_x, window_y;
    unsigned int mask;
    XQueryPointer(g_display, window->handle, &root, &child,
                  &root_x, &root_y, &window_x, &window_y, &mask);
    *xpos = (float)window_x;
    *ypos = (float)window_y;
}

void input_set_callbacks(window_t *window, callbacks_t callbacks) {
    window->callbacks = callbacks;
}

/* misc platform functions */

static double get_native_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

float platform_get_time(void) {
    static double initial = -1;
    if (initial < 0) {
        initial = get_native_time();
    }
    return (float)(get_native_time() - initial);
}
