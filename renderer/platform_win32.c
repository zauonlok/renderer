#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "platform.h"
#include "image.h"

/* data structures */

typedef struct {
    image_t *framebuffer;
    HDC cdc;
    HBITMAP dib;
    HBITMAP old;
} context_t;

struct window {
    HWND handle;
    int should_close;
    char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
    context_t *context;
};

/* window stuff */

static const char *WINDOW_CLASS_NAME = "Class";
static const char *WINDOW_ENTRY_NAME = "Entry";

static const char ACTION_UP = 0;
static const char ACTION_DOWN = 1;

static void handle_key_message(window_t *window, int virtual_key, char action) {
    keycode_t key;
    switch (virtual_key) {
        case 'A': key = KEY_A;   break;
        case 'D': key = KEY_D;   break;
        case 'S': key = KEY_S;   break;
        case 'W': key = KEY_W;   break;
        default:  key = KEY_NUM; break;
    }
    if (key < KEY_NUM) {
        window->keys[key] = action;
    }
}

static LRESULT CALLBACK process_message(HWND hWnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam) {
    window_t *window = (window_t*)GetProp(hWnd, WINDOW_ENTRY_NAME);
    if (window == NULL) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    } else if (uMsg == WM_CLOSE) {
        window->should_close = 1;
        return 0;
    } else if (uMsg == WM_KEYDOWN) {
        handle_key_message(window, wParam, ACTION_DOWN);
        return 0;
    } else if (uMsg == WM_KEYUP) {
        handle_key_message(window, wParam, ACTION_UP);
        return 0;
    } else if (uMsg == WM_LBUTTONDOWN) {
        window->buttons[BUTTON_L] = ACTION_DOWN;
        return 0;
    } else if (uMsg == WM_RBUTTONDOWN) {
        window->buttons[BUTTON_R] = ACTION_DOWN;
        return 0;
    } else if (uMsg == WM_LBUTTONUP) {
        window->buttons[BUTTON_L] = ACTION_UP;
        return 0;
    } else if (uMsg == WM_RBUTTONUP) {
        window->buttons[BUTTON_R] = ACTION_UP;
        return 0;
    } else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

static void register_class(void) {
    static int initialized = 0;
    if (initialized == 0) {
        ATOM id;
        WNDCLASS wc;
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = process_message;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = GetModuleHandle(NULL);
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = WINDOW_CLASS_NAME;
        id = RegisterClass(&wc);
        assert(id != 0);
        initialized = 1;
    }
}

static HWND create_window(const char *title, int width, int height) {
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect;
    HWND window;

    rect.left   = 0;
    rect.top    = 0;
    rect.right  = width;
    rect.bottom = height;
    AdjustWindowRect(&rect, style, 0);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    window = CreateWindow(WINDOW_CLASS_NAME, title, style,
                          CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                          NULL, NULL, GetModuleHandle(NULL), NULL);
    assert(window != NULL);
    return window;
}

static context_t *create_context(HWND window, int width, int height) {
    BITMAPINFOHEADER bi;
    HDC wdc, cdc;
    HBITMAP dib, old;
    unsigned char *buffer;
    context_t *context;
    image_t *framebuffer;

    wdc = GetDC(window);
    cdc = CreateCompatibleDC(wdc);
    ReleaseDC(window, wdc);

    memset(&bi, 0, sizeof(BITMAPINFOHEADER));
    bi.biSize        = sizeof(BITMAPINFOHEADER);
    bi.biWidth       = width;
    bi.biHeight      = -height;  /* top-down */
    bi.biPlanes      = 1;
    bi.biBitCount    = 32;
    bi.biCompression = BI_RGB;
    dib = CreateDIBSection(cdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
                           (void**)&buffer, NULL, 0);
    assert(dib != NULL);
    old = (HBITMAP)SelectObject(cdc, dib);

    framebuffer = (image_t*)malloc(sizeof(image_t));
    framebuffer->width    = width;
    framebuffer->height   = height;
    framebuffer->channels = 4;
    framebuffer->buffer   = buffer;

    context = (context_t*)malloc(sizeof(context_t));
    context->framebuffer = framebuffer;
    context->cdc         = cdc;
    context->dib         = dib;
    context->old         = old;
    return context;
}

window_t *window_create(const char *title, int width, int height) {
    HWND handle;
    context_t *context;
    window_t *window;

    assert(width > 0 && height > 0);

    register_class();
    handle = create_window(title, width, height);
    context = create_context(handle, width, height);

    window = (window_t*)malloc(sizeof(window_t));
    window->handle       = handle;
    window->should_close = 0;
    window->context      = context;
    memset(window->keys, 0, sizeof(window->keys));
    memset(window->buttons, 0, sizeof(window->buttons));

    SetProp(handle, WINDOW_ENTRY_NAME, window);
    ShowWindow(handle, SW_SHOW);
    return window;
}

void window_destroy(window_t *window) {
    ShowWindow(window->handle, SW_HIDE);
    RemoveProp(window->handle, WINDOW_ENTRY_NAME);

    SelectObject(window->context->cdc, window->context->old);
    DeleteDC(window->context->cdc);
    DeleteObject(window->context->dib);

    DestroyWindow(window->handle);

    free(window->context->framebuffer);
    free(window->context);
    free(window);
}

int window_should_close(window_t *window) {
    return window->should_close;
}

/* private function, implemented in image.c */
void image_blit_bgr(image_t *src, image_t *dst);

void window_draw_image(window_t *window, image_t *image) {
    HDC wdc = GetDC(window->handle);
    context_t *context = window->context;
    image_t *framebuffer = context->framebuffer;

    image_blit_bgr(image, framebuffer);
    BitBlt(wdc, 0, 0, framebuffer->width, framebuffer->height,
           context->cdc, 0, 0, SRCCOPY);
    ReleaseDC(window->handle, wdc);
}

/* input stuff */

void input_poll_events(void) {
    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
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
    POINT pos;
    GetCursorPos(&pos);
    ScreenToClient(window->handle, &pos);
    if (xpos != NULL) {
        *xpos = pos.x;
    }
    if (ypos != NULL) {
        *ypos = pos.y;
    }
}

/* time stuff */

double timer_get_time(void) {
    static double period = -1;
    LARGE_INTEGER counter;
    if (period < 0) {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        period = 1.0 / frequency.QuadPart;
    }
    QueryPerformanceCounter(&counter);
    return counter.QuadPart * period;
}

void timer_sleep_for(int milliseconds) {
    assert(milliseconds > 0);
    Sleep(milliseconds);
}
