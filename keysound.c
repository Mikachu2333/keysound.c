#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

// 退出程序的全局热键ID
#define HOTKEY_QUIT_ID 1001

// 全局变量：键盘钩子句柄
HHOOK g_hKeyboardHook = NULL;
// 音效文件路径（为软件同目录下的keysound.wav）
WCHAR g_soundPath[MAX_PATH] = L"keysound.wav";
static void InitializeSoundPath(void) {
    DWORD len = GetModuleFileNameW(NULL, g_soundPath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        g_soundPath[0] = L'\0';
        return;
    }
    while (len > 0) {
        if (g_soundPath[len - 1] == L'\\' || g_soundPath[len - 1] == L'/') {
            break;
        }
        --len;
    }
    if (len == 0) {
        g_soundPath[0] = L'\0';
        return;
    }
    if (lstrlenW(g_soundPath) + lstrlenW(L"keysound.wav") < MAX_PATH) {
        lstrcpyW(g_soundPath + len, L"keysound.wav");
    } else {
        g_soundPath[0] = L'\0';
    }
}

// 键盘钩子回调函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // 仅处理有效的键盘消息
    if (nCode == HC_ACTION) {
        // 仅在按键按下时播放音效（避免重复播放）
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            // 使用PlaySoundW播放音效，异步模式避免阻塞键盘输入
            PlaySoundW(g_soundPath, NULL,
                       SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
        }
    }

    // 传递钩子给下一个处理程序，保证系统正常工作
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// GUI程序入口WINMAIN，天然隐藏控制台，替换被弃用/不推荐的手段
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    InitializeSoundPath();

    // 安装全局键盘钩子（WH_KEYBOARD_LL是低级键盘钩子，无需DLL注入）
    g_hKeyboardHook =
        SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

    if (g_hKeyboardHook == NULL) {
        HANDLE hLog = CreateFileW(L"hook_error.log", GENERIC_WRITE, 0, NULL,
                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLog != INVALID_HANDLE_VALUE) {
            char errMsg[128];
            DWORD written;
            int len =
                snprintf(errMsg, sizeof(errMsg),
                         "Failed to install keyboard hook. Error code: %lu\n",
                         GetLastError());
            if (len > 0) {
                WriteFile(hLog, errMsg, (DWORD)len, &written, NULL);
            }
            CloseHandle(hLog);
        }
        return 1;
    }
    // 注册退出程序的快捷键（作为无控制台窗口的替代）:
    // Ctrl + Shift + Win + Alt + Tab
    // 另注：MOD_NOREPEAT 取消长按重复触发
    BOOL hotkeyRegistered = RegisterHotKey(
        NULL, HOTKEY_QUIT_ID,
        MOD_CONTROL | MOD_SHIFT | MOD_WIN | MOD_ALT | MOD_NOREPEAT, VK_TAB);
    if (!hotkeyRegistered) {
        HANDLE hLog = CreateFileW(L"hotkey_error.log", GENERIC_WRITE, 0, NULL,
                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLog != INVALID_HANDLE_VALUE) {
            char errMsg[128];
            DWORD written;
            int len =
                snprintf(errMsg, sizeof(errMsg),
                         "Failed to register quit hotkey. Error code: %lu\n",
                         GetLastError());
            if (len > 0) {
                WriteFile(hLog, errMsg, (DWORD)len, &written, NULL);
            }
            CloseHandle(hLog);
        }
        if (g_hKeyboardHook != NULL) {
            UnhookWindowsHookEx(g_hKeyboardHook);
            g_hKeyboardHook = NULL;
        }
        return 1;
    }

    // 消息循环：保持程序运行，处理系统消息
    MSG msg;
    BOOL bRet;
    int exitCode = 0;
    // GetMessageW 返回0表示WM_QUIT停止，返回-1表示错误
    while ((bRet = GetMessageW(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            exitCode = 1;
            break;  // 遇到错误安全退出
        } else {
            if (msg.message == WM_HOTKEY && msg.wParam == HOTKEY_QUIT_ID) {
                exitCode = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    if (bRet == 0) {
        exitCode = (int)msg.wParam;
    }

    // 反注册快捷键
    if (hotkeyRegistered) {
        UnregisterHotKey(NULL, HOTKEY_QUIT_ID);
    }

    // 卸载钩子（程序退出时清理）
    if (g_hKeyboardHook != NULL) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }

    return exitCode;
}
