#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "platform.h"
#include "image.h"
#include "error.h"

typedef struct context {
    int width;
    int height;
    int channels;
    int pitch;
    unsigned char *buffer;
    HDC cdc;
    HBITMAP dib;
    HBITMAP old;
} context_t;

struct window {
    HWND handle;
    context_t *context;
    bool should_close;
};

static const char *WINDOW_CLASS_NAME = "Renderer";

static LRESULT CALLBACK window_proc(HWND hWnd, UINT uMsg,
                                    WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CLOSE) {
        window_t *window = (window_t*)GetProp(hWnd, "Ninja");
        window->should_close = true;
        return 0;
    } else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

static void register_class() {
    static bool initialized = false;
    if (initialized == false) {
        WNDCLASS wc;
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = window_proc;
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
    }
}

static HWND create_window(const char *title, int width, int height) {
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect = {0, 0, width, height};
    HWND window;

    AdjustWindowRect(&rect, style, false);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    window = CreateWindow(WINDOW_CLASS_NAME, title, style,
                          CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                          NULL, NULL, GetModuleHandle(NULL), NULL);
    if (window == NULL) {
        FATAL("CreateWindow");
    }

    ShowWindow(window, SW_SHOW);
    return window;
}

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
    bi.biBitCount    = 32;
    bi.biCompression = BI_RGB;
    dib = CreateDIBSection(cdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
                           &buffer, NULL, 0);
    if (dib == NULL) {
        FATAL("CreateDIBSection");
    }
    old = (HBITMAP)SelectObject(cdc, dib);

    context = (context_t*)malloc(sizeof(context_t));
    context->width    = width;
    context->height   = height;
    context->channels = 4;
    context->pitch    = width * 4;
    context->buffer   = buffer;
    context->cdc      = cdc;
    context->dib      = dib;
    context->old      = old;
    return context;
}

window_t *platform_create_window(const char *title, int width, int height) {
    HWND handle;
    context_t *context;
    window_t *window;

    register_class();
    handle = create_window(title, width, height);
    context = create_context(handle, width, height);

    window = (window_t*)malloc(sizeof(window_t));
    window->handle       = handle;
    window->context      = context;
    window->should_close = false;
    SetProp(handle, "Ninja", window);
    return window;
}

bool platform_window_should_close(window_t *window) {
    return window->should_close;
}

void platform_draw_image(window_t *window, image_t *image) {
    context_t *context = window->context;
    HDC hdc;
    int row, col, chn;

    memset(context->buffer, 0, context->height * context->pitch);
    for (row = 0; row < context->height; row++) {
        if (row >= image->height) {
            break;
        }
        for (col = 0; col < context->width; col++) {
            int ctx_index = row * context->pitch + col * context->channels;
            int img_index = row * image->pitch + col * image->channels;
            if (col >= image->width) {
                break;
            }
            for (chn = 0; chn < context->channels; chn++) {
                if (chn >= image->channels) {
                    break;
                }
                int ctx_comp = ctx_index + chn;
                int img_comp = img_index + chn;
                context->buffer[ctx_comp] = image->buffer[img_comp];
            }
        }
    }

    hdc = GetDC(window->handle);
    BitBlt(hdc, 0, 0, context->width, context->height,
           context->cdc, 0, 0, SRCCOPY);
    ReleaseDC(window->handle, hdc);
}

void platform_destroy_window(window_t *window) {
    SelectObject(window->context->cdc, window->context->old);
    DeleteDC(window->context->cdc);
    DeleteObject(window->context->dib);
    DestroyWindow(window->handle);

    free(window->context);
    free(window);
}

void platform_poll_events(void) {
    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

static int translate_keycode(keycode_t key) {
    static bool initialized = false;
    static int keycodes[KEY_NUM];
    if (initialized == false) {
        keycodes[KEY_A] = 'A';
        keycodes[KEY_D] = 'D';
        keycodes[KEY_S] = 'S';
        keycodes[KEY_W] = 'W';
        initialized = true;
    }
    return keycodes[key];
}

bool platform_is_key_down(window_t *window, keycode_t key) {
    int virtual_key = translate_keycode(key);
    if (GetAsyncKeyState(virtual_key) & (1 << 31)) {
        return true;
    } else {
        return false;
    }
}

static int translate_button(button_t button) {
    static bool initialized = false;
    static int buttons[BUTTON_NUM];
    if (initialized == false) {
        buttons[BUTTON_L] = VK_LBUTTON;
        buttons[BUTTON_R] = VK_RBUTTON;
        initialized = true;
    }
    return buttons[button];
}

bool platform_is_button_down(window_t *window, button_t button) {
    int virtual_key = translate_button(button);
    if (GetAsyncKeyState(virtual_key) & (1 << 31)) {
        return true;
    } else {
        return false;
    }
}

void platform_get_cursor_pos(window_t *window, double *xpos, double *ypos) {
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

double platform_get_time(void) {
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
