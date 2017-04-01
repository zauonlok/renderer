#include <stdlib.h>
#include <string.h>
#include <mach/mach_time.h>
#include <Cocoa/Cocoa.h>
#include "platform.h"
#include "image.h"
#include "error.h"

/* data structures */

typedef struct context context_t;

struct window {
    NSWindow *handle;
    int should_close;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    context_t *context;
};

struct context {
    int width;
    int height;
    unsigned char *buffer;
};

/* window stuff */

NSAutoreleasePool *g_pool;

@interface ApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation ApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [NSApp stop:nil];
}

@end

@interface WindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation WindowDelegate {
    window_t *window;
}

- (instancetype)initWithWindow:(window_t *)window_ {
    self = [super init];
    if (self != nil) {
        window = window_;
    }
    return self;
}

- (BOOL)windowShouldClose:(id)sender {
    window->should_close = 1;
    return NO;
}

@end

static void handle_key_event(window_t *window, int virtual_key, char action) {
    keycode_t key;
    switch (virtual_key) {
        case 0x00: key = KEY_A;   break;
        case 0x02: key = KEY_D;   break;
        case 0x01: key = KEY_S;   break;
        case 0x0D: key = KEY_W;   break;
        default:   key = KEY_NUM; break;
    }
    if (key < KEY_NUM) {
        window->keys[key] = action;
    }
}

@interface ContentView : NSView
@end

@implementation ContentView {
    window_t *window;
}

- (instancetype)initWithWindow:(window_t *)window_ {
    self = [super init];
    if (self != nil) {
        window = window_;
    }
    return self;
}

- (BOOL)isOpaque {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)drawRect:(NSRect)rect {
    context_t *context = window->context;
    NSImage *image;
    NSBitmapImageRep *rep;

    rep = [[[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:&(context->buffer)
                          pixelsWide:context->width
                          pixelsHigh:context->height
                       bitsPerSample:8
                     samplesPerPixel:3
                            hasAlpha:NO
                            isPlanar:NO
                      colorSpaceName:NSCalibratedRGBColorSpace
                         bytesPerRow:context->width * 4
                        bitsPerPixel:32] autorelease];
    FORCE(rep != nil, "NSBitmapImageRep");
    image = [[[NSImage alloc] init] autorelease];
    [image addRepresentation:rep];
    [image drawInRect: rect];
}

- (void)keyDown:(NSEvent *)event {
    handle_key_event(window, [event keyCode], 1);
}

- (void)keyUp:(NSEvent *)event {
    handle_key_event(window, [event keyCode], 0);
}

- (void)mouseDown:(NSEvent *)event {
    window->buttons[BUTTON_L] = 1;
}

- (void)mouseUp:(NSEvent *)event {
    window->buttons[BUTTON_L] = 0;
}

- (void)rightMouseDown:(NSEvent *)event {
    window->buttons[BUTTON_R] = 1;
}

- (void)rightMouseUp:(NSEvent *)event {
    window->buttons[BUTTON_R] = 0;
}

@end

static void create_menubar() {
    NSMenu *menu_bar, *app_menu;
    NSMenuItem *app_menu_item, *quit_menu_item;

    menu_bar = [[[NSMenu alloc] init] autorelease];
    [NSApp setMainMenu:menu_bar];
    app_menu_item = [[[NSMenuItem alloc] init] autorelease];
    [menu_bar addItem:app_menu_item];

    app_menu = [[[NSMenu alloc] init] autorelease];
    [app_menu_item setSubmenu:app_menu];
    quit_menu_item = [[[NSMenuItem alloc] initWithTitle:@"Quit Viewer"
                                                 action:@selector(terminate:)
                                          keyEquivalent:@"q"] autorelease];
    [app_menu addItem:quit_menu_item];
}

static void create_application() {
    ApplicationDelegate *delegate;
    if (NSApp) {
        return;
    }
    g_pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    /* menubar setup must go between sharedApplication and finishLaunching */
    create_menubar();
    delegate = [[[ApplicationDelegate alloc] init] autorelease];
    FORCE(delegate != nil, "ApplicationDelegate");
    [NSApp setDelegate:delegate];
    [NSApp run];
}

static NSWindow *create_window(window_t *window, const char *title,
                               int width, int height) {
    NSRect rect;
    NSUInteger mask;
    NSWindow *handle;
    WindowDelegate *delegate;

    rect = NSMakeRect(0, 0, width, height);
    mask = NSWindowStyleMaskTitled
           | NSWindowStyleMaskClosable
           | NSWindowStyleMaskMiniaturizable;
    handle = [[NSWindow alloc] initWithContentRect:rect
                                         styleMask:mask
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    FORCE(handle != nil, "NSWindow");
    [handle setTitle:[NSString stringWithUTF8String:title]];
    delegate = [[[WindowDelegate alloc] initWithWindow:window] autorelease];
    FORCE(delegate != nil, "WindowDelegate");
    [handle setDelegate:delegate];
    return handle;
}

static context_t *create_context(window_t *window, NSWindow *handle,
                                 int width, int height) {
    int bytes_per_pixel = 4;
    int buffer_size = width * height * bytes_per_pixel;
    unsigned char *buffer = (unsigned char*)malloc(buffer_size);
    ContentView *view;
    context_t *context;

    view = [[[ContentView alloc] initWithWindow:window] autorelease];
    FORCE(view != nil, "ContentView");
    [handle setContentView:view];
    [handle makeFirstResponder:view];

    context = (context_t*)malloc(sizeof(context_t));
    context->width  = width;
    context->height = height;
    context->buffer = buffer;
    return context;
}

window_t *window_create(const char *title, int width, int height) {
    window_t *window = (window_t*)malloc(sizeof(window_t));
    NSWindow * handle;
    context_t *context;

    create_application();
    handle = create_window(window, title, width, height);
    context = create_context(window, handle, width, height);

    window->handle       = handle;
    window->should_close = 0;
    window->context      = context;
    memset(window->keys, 0, sizeof(window->keys));
    memset(window->buttons, 0, sizeof(window->buttons));

    [NSApp activateIgnoringOtherApps:YES];
    [handle makeKeyAndOrderFront:nil];
    return window;
}

void window_destroy(window_t *window) {
    [window->handle orderOut:nil];
    [window->handle close];
    [g_pool drain];
    g_pool = [[NSAutoreleasePool alloc] init];
    free(window->context);
    free(window);
}

int window_should_close(window_t *window) {
    return window->should_close;
}

void window_draw_image(window_t *window, image_t *image) {
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
                context_pixel[2] = image_pixel[0];
                context_pixel[1] = image_pixel[1];
                context_pixel[0] = image_pixel[2];
            }
        }
    }

    [[window->handle contentView] setNeedsDisplay:YES];
}

/* input stuff */

void input_poll_events(void) {
    while (1) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantPast]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (event == nil) {
            break;
        }
        [NSApp sendEvent:event];
    }
    [g_pool drain];
    g_pool = [[NSAutoreleasePool alloc] init];
}

int input_key_pressed(window_t *window, keycode_t key) {
    return window->keys[key];
}

int input_button_pressed(window_t *window, button_t button) {
    return window->buttons[button];
}

void input_query_cursor(window_t *window, int *xpos, int *ypos) {
    NSRect rect = [[window->handle contentView] frame];
    NSPoint pos = [window->handle mouseLocationOutsideOfEventStream];
    if (xpos) {
        *xpos = (int)(pos.x + 0.5);
    }
    if (ypos) {
        *ypos = (int)(rect.size.height - pos.y - 1 + 0.5);
    }
}

double input_get_time(void) {
    static double period = -1;
    if (period < 0) {
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        period = (double)info.numer / (double)info.denom / 1e9;
    }
    return mach_absolute_time() * period;
}
