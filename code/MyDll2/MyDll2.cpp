#include "windows.h"
#include "tchar.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    TCHAR szCmd[MAX_PATH]  = {0,};
    TCHAR szPath[MAX_PATH] = {0,};
    TCHAR *p = NULL;
    STARTUPINFO si = {0,};
    PROCESS_INFORMATION pi = {0,};

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    switch( fdwReason ) {
    case DLL_PROCESS_ATTACH: 
		//��õ�ǰDLL��װ�ص��Ľ��̵Ŀ�ִ���ļ���·����szPath��
        if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
            break;
        if( !(p = _tcsrchr(szPath, _T('\\') )) )
            break;
        if( lstrcmpi(p+1, _T("notepad.exe")) )
            break;
		//��ǰDLL�����ص��Ľ��̵Ŀ�ִ���ļ�Ϊnotepad.exe������IE����www.xidian.edu.cn
        wsprintf(szCmd, _T("%s %s"), _T("c:\\Program Files\\Internet Explorer\\iexplore.exe"), _T("http://www.xidian.edu.cn"));
        if( !CreateProcess(NULL, (LPTSTR)(LPCTSTR)szCmd, 
                            NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi) )
            break;
        if( pi.hProcess != NULL )
            CloseHandle(pi.hProcess);
        break;
    }
    return TRUE;
}
