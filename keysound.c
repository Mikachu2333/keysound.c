#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

// 链接Windows多媒体库
#pragma comment(lib, "winmm.lib")

// 全局变量：键盘钩子句柄
HHOOK g_hKeyboardHook = NULL;
// 音效文件路径（建议使用WAV格式，延迟最低）
const char* g_soundPath = "keysound.wav";

// 键盘钩子回调函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // 仅处理有效的键盘消息
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
        
        // 仅在按键按下时播放音效（避免重复播放）
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            // 使用PlaySound播放音效，异步模式避免阻塞键盘输入
            PlaySoundA(g_soundPath, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
        }
    }
    
    // 传递钩子给下一个处理程序，保证系统正常工作
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// 隐藏控制台窗口（如果程序以控制台模式启动）
void HideConsoleWindow() {
    HWND hConsole = GetConsoleWindow();
    if (hConsole != NULL) {
        ShowWindow(hConsole, SW_HIDE);
    }
}

int main() {
    // 隐藏控制台窗口
    HideConsoleWindow();
    
    // 安装全局键盘钩子（WH_KEYBOARD_LL是低级键盘钩子，无需DLL注入）
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    
    if (g_hKeyboardHook == NULL) {
        // 如果钩子安装失败，记录错误日志到文件
        FILE* pLog = fopen("hook_error.log", "w");
        if (pLog != NULL) {
            fprintf(pLog, "Failed to install keyboard hook. Error code: %d\n", GetLastError());
            fclose(pLog);
        }
        return 1;
    }
    
    // 消息循环：保持程序运行，处理系统消息
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // 卸载钩子（程序退出时清理）
    UnhookWindowsHookEx(g_hKeyboardHook);
    return 0;
}