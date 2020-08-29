#include "windows.h"
#include "tchar.h"

HINSTANCE g_hInstance = NULL;
HHOOK g_hHook = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved){
	switch( dwReason ){
        case DLL_PROCESS_ATTACH:
			g_hInstance = hinstDLL;
			break;
	}
	return TRUE;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam){
	TCHAR szPath[MAX_PATH] = {0,};
	TCHAR *p = NULL;

	if( nCode >= 0 ) {
		if( !(lParam & 0x80000000) ){ //lParam�ĵ�31λ��0��������1���ͷż���
			GetModuleFileName(NULL, szPath, MAX_PATH);
			p = _tcsrchr(szPath, _T('\\'));
            //��װ�ص�ǰDLL�Ľ���Ϊnotepad.exe������Ϣ���ᴫ�ݸ���һ������
			if( !lstrcmpi(p + 1, _T("notepad.exe")) )
				return 1;
		}
	}
    // ��ǰ���̲���notepad.exe������Ϣ���ݸ���һ������
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) void HkStart() {
		g_hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, g_hInstance, 0);
	}

	__declspec(dllexport) void HkStop() {
		if( g_hHook ) {
			UnhookWindowsHookEx(g_hHook);
			g_hHook = NULL;
		}
	}
#ifdef __cplusplus
}
#endif
