#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mach/mach_time.h>
#import <Cocoa/Cocoa.h>
#include "platform.h"
#include "image.h"

/* data structures */

typedef struct {
    image_t *framebuffer;
} context_t;

struct window {
    NSWindow *handle;
    int should_close;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    context_t *context;
};

/* memory management */

static NSAutoreleasePool *g_autoreleasepool;

/* window stuff */

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
    (void)sender;
    window->should_close = 1;
    return NO;
}

@end

static const char ACTION_UP = 0;
static const char ACTION_DOWN = 1;

static void handle_key_event(window_t *window, int virtual_key, char action) {
    /* for virtual keys, see https://stackoverflow.com/questions/3202629/ */
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
    return YES;  /* pixels will be drawn opaquely */
}

- (BOOL)acceptsFirstResponder {
    return YES;  /* to receive key-down events */
}

- (void)drawRect:(NSRect)dirtyRect {
    image_t *framebuffer = window->context->framebuffer;
    NSImage *nsimage;
    NSBitmapImageRep *rep;

    rep = [[[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:&(framebuffer->buffer)
                          pixelsWide:framebuffer->width
                          pixelsHigh:framebuffer->height
                       bitsPerSample:8
                     samplesPerPixel:3
                            hasAlpha:NO
                            isPlanar:NO
                      colorSpaceName:NSCalibratedRGBColorSpace
                         bytesPerRow:framebuffer->width * 4
                        bitsPerPixel:32] autorelease];
    assert(rep != nil);
    nsimage = [[[NSImage alloc] init] autorelease];
    [nsimage addRepresentation:rep];

    [nsimage drawInRect:dirtyRect];
}

- (void)keyDown:(NSEvent *)event {
    handle_key_event(window, [event keyCode], ACTION_DOWN);
}

- (void)keyUp:(NSEvent *)event {
    handle_key_event(window, [event keyCode], ACTION_UP);
}

- (void)mouseDown:(NSEvent *)event {
    (void)event;
    window->buttons[BUTTON_L] = ACTION_DOWN;
}

- (void)mouseUp:(NSEvent *)event {
    (void)event;
    window->buttons[BUTTON_L] = ACTION_UP;
}

- (void)rightMouseDown:(NSEvent *)event {
    (void)event;
    window->buttons[BUTTON_R] = ACTION_DOWN;
}

- (void)rightMouseUp:(NSEvent *)event {
    (void)event;
    window->buttons[BUTTON_R] = ACTION_UP;
}

@end

static void create_menubar(void) {
    NSMenu *menu_bar, *app_menu;
    NSMenuItem *app_menu_item, *quit_menu_item;
    NSString *app_name, *quit_title;

    menu_bar = [[[NSMenu alloc] init] autorelease];
    [NSApp setMainMenu:menu_bar];

    app_menu_item = [[[NSMenuItem alloc] init] autorelease];
    [menu_bar addItem:app_menu_item];

    app_menu = [[[NSMenu alloc] init] autorelease];
    [app_menu_item setSubmenu:app_menu];

    app_name = [[NSProcessInfo processInfo] processName];
    quit_title = [@"Quit " stringByAppendingString:app_name];
    quit_menu_item = [[[NSMenuItem alloc] initWithTitle:quit_title
                                                 action:@selector(terminate:)
                                          keyEquivalent:@"q"] autorelease];
    [app_menu addItem:quit_menu_item];
}

static void change_directory(void) {
    NSArray<NSString*> *arguments = [[NSProcessInfo processInfo] arguments];
    NSString *program_path = [arguments objectAtIndex:0];
    NSString *program_dir = [program_path stringByDeletingLastPathComponent];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:program_dir];
}

static void create_application(void) {
    if (NSApp == nil) {
        g_autoreleasepool = [[NSAutoreleasePool alloc] init];
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        create_menubar();
        [NSApp finishLaunching];
        change_directory();
    }
}

static NSWindow *create_window(window_t *window, const char *title,
                               int width, int height) {
    NSRect rect = NSMakeRect(0, 0, width, height);
    NSUInteger mask;
    NSWindow *handle;
    WindowDelegate *delegate;
    ContentView *view;

    mask = NSWindowStyleMaskTitled
           | NSWindowStyleMaskClosable
           | NSWindowStyleMaskMiniaturizable;
    handle = [[NSWindow alloc] initWithContentRect:rect
                                         styleMask:mask
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    assert(handle != nil);
    [handle setTitle:[NSString stringWithUTF8String:title]];

    delegate = [[WindowDelegate alloc] initWithWindow:window];
    assert(delegate != nil);
    [handle setDelegate:delegate];  /* setDelegate has @property(assign),
                                       do not autorelease the delegate */

    view = [[[ContentView alloc] initWithWindow:window] autorelease];
    assert(view != nil);
    [handle setContentView:view];
    [handle makeFirstResponder:view];

    return handle;
}

static context_t *create_context(int width, int height) {
    context_t *context = (context_t*)malloc(sizeof(context_t));
    context->framebuffer = image_create(width, height, 4);
    return context;
}

window_t *window_create(const char *title, int width, int height) {
    window_t *window = (window_t*)malloc(sizeof(window_t));
    NSWindow * handle;
    context_t *context;

    assert(width > 0 && height > 0);

    create_application();
    handle = create_window(window, title, width, height);
    context = create_context(width, height);

    window->handle       = handle;
    window->should_close = 0;
    window->context      = context;
    memset(window->keys, 0, sizeof(window->keys));
    memset(window->buttons, 0, sizeof(window->buttons));

    [handle makeKeyAndOrderFront:nil];
    return window;
}

void window_destroy(window_t *window) {
    WindowDelegate *delegate = [window->handle delegate];
    context_t *context = window->context;

    [window->handle orderOut:nil];

    [window->handle setDelegate:nil];
    [delegate release];
    [window->handle close];

    [g_autoreleasepool drain];
    g_autoreleasepool = [[NSAutoreleasePool alloc] init];

    image_release(context->framebuffer);
    free(context);
    free(window);
}

int window_should_close(window_t *window) {
    return window->should_close;
}

/* private function, implemented in image.c */
void image_blit_rgb(image_t *src, image_t *dst);

void window_draw_image(window_t *window, image_t *image) {
    image_blit_rgb(image, window->context->framebuffer);
    [[window->handle contentView] setNeedsDisplay:YES];  /* invoke drawRect */
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
    [g_autoreleasepool drain];
    g_autoreleasepool = [[NSAutoreleasePool alloc] init];
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
    NSPoint pos = [window->handle mouseLocationOutsideOfEventStream];
    NSRect rect = [[window->handle contentView] frame];
    if (xpos) {
        *xpos = (int)(pos.x + 0.5);
    }
    if (ypos) {
        *ypos = (int)(rect.size.height - 1 - pos.y + 0.5);
    }
}

/* time stuff */

double timer_get_time(void) {
    static double period = -1;
    if (period < 0) {
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        period = (double)info.numer / (double)info.denom / 1e9;
    }
    return mach_absolute_time() * period;
}

void timer_sleep_for(int milliseconds) {
    struct timespec ts;
    assert(milliseconds > 0);
    ts.tv_sec  = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
