#include <stdio.h>
#include <string.h>

#include "X11/Xlib.h"

int main()
{
    // Инициализируем XOpenDisplay
    Display* display = XOpenDisplay(0);
    if (display == NULL) {
        printf("Ошибка при инициализации XOpenDisplay\n");
        return 1;
    }
    printf("XOpenDisplay инициализирован успешно\n");

    // Получение номера экрана по умолчанию
    int screen = DefaultScreen(display);

    // Создаем Graphics context
    GC gc = DefaultGC(display, screen);

    // Получаем ссылку на родительское окно
    Window parent_window = DefaultRootWindow(display);

    unsigned int border_color = BlackPixel(display, screen);
    unsigned int background_color = WhitePixel(display, screen);

    // Создаем окно
    Window window = XCreateSimpleWindow(display, parent_window, 0, 0, 640, 480, 1, border_color, background_color);
    if (!window) {
        printf("Ошибка при выполнении XCreateSimpleWindow\n");
        return 1;
    }

    long event_mask = ExposureMask
        | KeyPressMask
        | KeyReleaseMask
        | ButtonPressMask
        | ButtonReleaseMask
        | FocusChangeMask;

    // Select window events
    XSelectInput(display, window, event_mask);

    // Make window visible
    XMapWindow(display, window);

    // Set window title
    XStoreName(display, window, "XServer window");

    // Get WM_DELETE_WINDOW atom
    Atom wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", True);

    // Subscribe WM_DELETE_WINDOW message
    XSetWMProtocols(display, window, &wm_delete, 1);

    char msg[1024] = "Window message\0";

    for (;;)
    {
        // Get events from event loop
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == KeyPress)
        {
            XClearWindow(display, window);
            XDrawString(display, window, gc, 10, 20, msg, strlen(msg));
        }

        // Close button
        if (event.type == ClientMessage) {
            if (event.xclient.data.l[0] == wm_delete) {
                break;
            }
        }
    }

    XCloseDisplay(display);
    return 0;
}