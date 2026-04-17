#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

// 全局变量：键盘钩子句柄
HHOOK g_hKeyboardHook = NULL;
// 音效文件路径（建议使用WAV格式，延迟最低）
const WCHAR* g_soundPath = L"keysound.wav";

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

    // 消息循环：保持程序运行，处理系统消息
    MSG msg;
    BOOL bRet;
    // GetMessageW 返回0表示WM_QUIT停止，返回-1表示错误
    while ((bRet = GetMessageW(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            break;  // 遇到错误安全退出
        } else {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // 卸载钩子（程序退出时清理）
    if (g_hKeyboardHook != NULL) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }

    return (int)msg.wParam;
}
