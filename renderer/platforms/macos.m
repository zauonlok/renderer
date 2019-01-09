#include "../core/platform.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach_time.h>
#import <Cocoa/Cocoa.h>
#include "../core/graphics.h"
#include "../core/image.h"

#define UNUSED(x) ((void)(x))

struct window {
    NSWindow *handle;
    image_t *surface;
    /* states */
    int should_close;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    /* callbacks */
    callbacks_t callbacks;
};

static NSAutoreleasePool *g_autoreleasepool;

/* window related functions */

@interface WindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation WindowDelegate {
    window_t *window;
}

- (instancetype)initWithWindow:(window_t *)aWindow {
    self = [super init];
    if (self != nil) {
        window = aWindow;
    }
    return self;
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    UNUSED(sender);
    window->should_close = 1;
    return NO;
}

@end

/*
 * for virtual-key codes, see
 * https://stackoverflow.com/questions/3202629/
 */
static void handle_key_event(window_t *window, int virtual_key, char pressed) {
    keycode_t key;
    switch (virtual_key) {
        case 0x00: key = KEY_A;   break;
        case 0x02: key = KEY_D;   break;
        case 0x01: key = KEY_S;   break;
        case 0x0D: key = KEY_W;   break;
        default:   key = KEY_NUM; break;
    }
    if (key < KEY_NUM) {
        window->keys[key] = pressed;
        if (window->callbacks.key_callback) {
            window->callbacks.key_callback(window, key, pressed);
        }
    }
}

static void handle_button_event(window_t *window, button_t button,
                                char pressed) {
    window->buttons[button] = pressed;
    if (window->callbacks.button_callback) {
        window->callbacks.button_callback(window, button, pressed);
    }
}

static void handle_scroll_event(window_t *window, double offset) {
    if (window->callbacks.scroll_callback) {
        window->callbacks.scroll_callback(window, offset);
    }
}

@interface ContentView : NSView
@end

@implementation ContentView {
    window_t *window;
}

- (instancetype)initWithWindow:(window_t *)aWindow {
    self = [super init];
    if (self != nil) {
        window = aWindow;
    }
    return self;
}

- (BOOL)acceptsFirstResponder {
    return YES;  /* to receive key-down events */
}

- (void)drawRect:(NSRect)dirtyRect {
    image_t *surface = window->surface;
    NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:&(surface->buffer)
                          pixelsWide:surface->width
                          pixelsHigh:surface->height
                       bitsPerSample:8
                     samplesPerPixel:3
                            hasAlpha:NO
                            isPlanar:NO
                      colorSpaceName:NSCalibratedRGBColorSpace
                         bytesPerRow:surface->width * 4
                        bitsPerPixel:32] autorelease];
    NSImage *nsimage = [[[NSImage alloc] init] autorelease];
    [nsimage addRepresentation:rep];
    [nsimage drawInRect:dirtyRect];
}

- (void)keyDown:(NSEvent *)event {
    handle_key_event(window, [event keyCode], 1);
}

- (void)keyUp:(NSEvent *)event {
    handle_key_event(window, [event keyCode], 0);
}

- (void)mouseDown:(NSEvent *)event {
    UNUSED(event);
    handle_button_event(window, BUTTON_L, 1);
}

- (void)mouseUp:(NSEvent *)event {
    UNUSED(event);
    handle_button_event(window, BUTTON_L, 0);
}

- (void)rightMouseDown:(NSEvent *)event {
    UNUSED(event);
    handle_button_event(window, BUTTON_R, 1);
}

- (void)rightMouseUp:(NSEvent *)event {
    UNUSED(event);
    handle_button_event(window, BUTTON_R, 0);
}

- (void)scrollWheel:(NSEvent *)event {
    double offset = [event scrollingDeltaY];
    if ([event hasPreciseScrollingDeltas]) {
        offset *= 0.1;
    }
    handle_scroll_event(window, offset);
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
    NSRect rect;
    NSUInteger mask;
    NSWindow *handle;
    WindowDelegate *delegate;
    ContentView *view;

    rect = NSMakeRect(0, 0, width, height)
    mask = NSWindowStyleMaskTitled
           | NSWindowStyleMaskClosable
           | NSWindowStyleMaskMiniaturizable;
    handle = [[NSWindow alloc] initWithContentRect:rect
                                         styleMask:mask
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    assert(handle != nil);
    [handle setTitle:[NSString stringWithUTF8String:title]];

    /*
     * the storage semantics of NSWindow.setDelegate is @property(assign),
     * or @property(weak) with ARC, we must not autorelease the delegate
     */
    delegate = [[WindowDelegate alloc] initWithWindow:window];
    assert(delegate != nil);
    [handle setDelegate:delegate];

    view = [[[ContentView alloc] initWithWindow:window] autorelease];
    assert(view != nil);
    [handle setContentView:view];
    [handle makeFirstResponder:view];

    return handle;
}

window_t *window_create(const char *title, int width, int height) {
    window_t *window;
    NSWindow * handle;
    image_t *surface;

    assert(width > 0 && height > 0);

    create_application();
    window = (window_t*)malloc(sizeof(window_t));
    handle = create_window(window, title, width, height);
    surface = image_create(width, height, 4);

    window->handle       = handle;
    window->surface      = surface;
    window->should_close = 0;
    memset(window->keys, 0, sizeof(window->keys));
    memset(window->buttons, 0, sizeof(window->buttons));
    memset(&window->callbacks, 0, sizeof(window->callbacks));

    [handle makeKeyAndOrderFront:nil];
    return window;
}

void window_destroy(window_t *window) {
    [window->handle orderOut:nil];

    [[window->handle delegate] release];
    [window->handle close];

    [g_autoreleasepool drain];
    g_autoreleasepool = [[NSAutoreleasePool alloc] init];

    image_release(window->surface);
    free(window);
}

int window_should_close(window_t *window) {
    return window->should_close;
}

void private_blit_rgb_image(image_t *src, image_t *dst);
void private_blit_rgb_buffer(colorbuffer_t *src, image_t *dst);

void window_draw_image(window_t *window, image_t *image) {
    private_blit_rgb_image(image, window->surface);
    [[window->handle contentView] setNeedsDisplay:YES];  /* invoke drawRect */
}

void window_draw_buffer(window_t *window, colorbuffer_t *buffer) {
    private_blit_rgb_buffer(buffer, window->surface);
    [[window->handle contentView] setNeedsDisplay:YES];  /* invoke drawRect */
}

/* input related functions */

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
    assert(key >= 0 && key < KEY_NUM);
    return window->keys[key];
}

int input_button_pressed(window_t *window, button_t button) {
    assert(button >= 0 && button < BUTTON_NUM);
    return window->buttons[button];
}

void input_query_cursor(window_t *window, double *xpos, double *ypos) {
    NSPoint pos = [window->handle mouseLocationOutsideOfEventStream];
    NSRect rect = [[window->handle contentView] frame];
    if (xpos) {
        *xpos = pos.x;
    }
    if (ypos) {
        *ypos = rect.size.height - 1 - pos.y;
    }
}

void input_set_callbacks(window_t *window, callbacks_t callbacks) {
    window->callbacks = callbacks;
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
