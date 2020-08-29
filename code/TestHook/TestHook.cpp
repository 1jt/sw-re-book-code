#include "stdio.h"
#include "windows.h"
#include "tchar.h"

typedef void (*PFN_HOOKSTART)();
typedef void (*PFN_HOOKSTOP)();

int _tmain(int argc, TCHAR* argv[]) {
	HMODULE	hDll = NULL;
	if( (hDll = LoadLibraryA("HookDll.dll")) == NULL )// װ��HookDll.dll
		return FALSE;

	// ��ȡ��������HkStart()��HkStop()�ĵ�ַ
	PFN_HOOKSTART HookStart = (PFN_HOOKSTART)GetProcAddress(hDll, "HkStart");
	PFN_HOOKSTOP HookStop = (PFN_HOOKSTOP)GetProcAddress(hDll, "HkStop");

	HookStart(); //��ʼ��ȡ������Ϣ
	_tprintf(_T("press 'q' to quit!\n"));
	while( getchar() != 'q' ); // �ȵ��û�����'q'����ֹ��ȡ
	HookStop(); //��ֹ��ȡ������Ϣ
	FreeLibrary(hDll); //ж��HookDll.dll
	return TRUE;
}
