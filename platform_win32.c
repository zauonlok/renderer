#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "platform.h"
#include "image.h"
#include "error.h"

/* data structures */

typedef struct context context_t;

struct window {
    HWND handle;
    int should_close;
    char keycodes[KEY_NUM];
    char buttons[BUTTON_NUM];
    context_t *context;
};

struct context {
    int width;
    int height;
    int channels;
    unsigned char *buffer;
    HDC cdc;
    HBITMAP dib;
    HBITMAP old;
};

/* window stuff */

static const char *WINDOW_CLASS_NAME = "Class";
static const char *WINDOW_ENTRY_NAME = "Entry";

static void handle_key_msg(window_t *window, int virtual_key, char action) {
    keycode_t key;
    switch (virtual_key) {
        case 'A': key = KEY_A;   break;
        case 'D': key = KEY_D;   break;
        case 'S': key = KEY_S;   break;
        case 'W': key = KEY_W;   break;
        default:  key = KEY_NUM; break;
    }
    if (key < KEY_NUM) {
        window->keycodes[key] = action;
    }
}

static LRESULT CALLBACK process_message(HWND hWnd, UINT uMsg,
                                        WPARAM wParam, LPARAM lParam) {
    window_t *window = (window_t*)GetProp(hWnd, WINDOW_ENTRY_NAME);
    if (uMsg == WM_CLOSE) {
        window->should_close = 1;
        return 0;
    } else if (uMsg == WM_KEYDOWN) {
        handle_key_msg(window, wParam, 1);
        return 0;
    } else if (uMsg == WM_KEYUP) {
        handle_key_msg(window, wParam, 0);
        return 0;
    } else if (uMsg == WM_LBUTTONDOWN) {
        window->buttons[BUTTON_L] = 1;
        return 0;
    } else if (uMsg == WM_RBUTTONDOWN) {
        window->buttons[BUTTON_R] = 1;
        return 0;
    } else if (uMsg == WM_LBUTTONUP) {
        window->buttons[BUTTON_L] = 0;
        return 0;
    } else if (uMsg == WM_RBUTTONUP) {
        window->buttons[BUTTON_R] = 0;
        return 0;
    } else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

static void register_class(void) {
    static int initialized = 0;
    if (initialized == 0) {
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
        if (RegisterClass(&wc) == 0) {
            FATAL("RegisterClass");
        }
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
    FORCE(window != NULL, "CreateWindow");
    ShowWindow(window, SW_SHOW);
    return window;
}

static const int BYTES_PER_PIXEL = 4;

static context_t *create_context(HWND window, int width, int height) {
    BITMAPINFOHEADER bi;
    HDC wdc, cdc;
    HBITMAP dib, old;
    unsigned char *buffer;
    context_t *context;

    wdc = GetDC(window);
    cdc = CreateCompatibleDC(wdc);
    ReleaseDC(window, wdc);

    memset(&bi, 0, sizeof(BITMAPINFOHEADER));
    bi.biSize        = sizeof(BITMAPINFOHEADER);
    bi.biWidth       = width;
    bi.biHeight      = -height;
    bi.biPlanes      = 1;
    bi.biBitCount    = (WORD)(BYTES_PER_PIXEL * 8);
    bi.biCompression = BI_RGB;
    dib = CreateDIBSection(cdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
                           (void**)&buffer, NULL, 0);
    FORCE(dib != NULL, "CreateDIBSection");
    old = (HBITMAP)SelectObject(cdc, dib);

    context = (context_t*)malloc(sizeof(context_t));
    context->width    = width;
    context->height   = height;
    context->channels = BYTES_PER_PIXEL;
    context->buffer   = buffer;
    context->cdc      = cdc;
    context->dib      = dib;
    context->old      = old;
    return context;
}

window_t *window_create(const char *title, int width, int height) {
    HWND handle;
    context_t *context;
    window_t *window;

    register_class();
    handle = create_window(title, width, height);
    context = create_context(handle, width, height);

    window = (window_t*)malloc(sizeof(window_t));
    window->handle       = handle;
    window->should_close = 0;
    window->context      = context;
    memset(window->keycodes, 0, sizeof(window->keycodes));
    memset(window->buttons, 0, sizeof(window->buttons));
    SetProp(handle, WINDOW_ENTRY_NAME, window);
    return window;
}

void window_destroy(window_t *window) {
    RemoveProp(window->handle, WINDOW_ENTRY_NAME);
    SelectObject(window->context->cdc, window->context->old);
    DeleteDC(window->context->cdc);
    DeleteObject(window->context->dib);
    DestroyWindow(window->handle);
    free(window->context);
    free(window);
}

int window_should_close(window_t *window) {
    return window->should_close;
}

void window_draw_image(window_t *window, image_t *image) {
    context_t *context = window->context;
    int bytes_per_row = context->width * BYTES_PER_PIXEL;
    int buffer_size = context->height * bytes_per_row;
    HDC wdc;
    int r, c, k;

    memset(context->buffer, 0, buffer_size);
    for (r = 0; r < context->height && r < image->height; r++) {
        for (c = 0; c < context->width && c < image->width; c++) {
            int context_index = r * bytes_per_row + c * BYTES_PER_PIXEL;
            unsigned char *context_pixel = &(context->buffer[context_index]);
            unsigned char *image_pixel = image_pixel_ptr(image, r, c);
            for (k = 0; k < context->channels && k < image->channels; k++) {
                context_pixel[k] = image_pixel[k];
            }
        }
    }

    wdc = GetDC(window->handle);
    BitBlt(wdc, 0, 0, context->width, context->height,
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
    return window->keycodes[key];
}

int input_button_pressed(window_t *window, button_t button) {
    return window->buttons[button];
}

void input_query_cursor(window_t *window, int *row, int *col) {
    POINT pos;
    GetCursorPos(&pos);
    ScreenToClient(window->handle, &pos);
    if (row != NULL) {
        *row = pos.y;
    }
    if (col != NULL) {
        *col = pos.x;
    }
}

double input_get_time(void) {
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
