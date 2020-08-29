#include "windows.h"

typedef BOOL (WINAPI *PFWRITEFILE)(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytesToWrite, LPDWORD lpBytesWrittern, LPOVERLAPPED lpOverlapped);

FARPROC g_pOrgFunc = NULL;

BOOL WINAPI MyWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytesToWrite, LPDWORD lpBytesWrittern, LPOVERLAPPED lpOverlapped) {
	DWORD i = 0;
	LPSTR lpString=(LPSTR)lpBuffer;
	for(i = 0; i < nBytesToWrite; i++){
		if( 0x61 <= lpString[i] && lpString[i] <= 0x7A ){
			lpString[i] = 0x61 + (lpString[i]-0x60)%0x1A;
		}
	}
	return ((PFWRITEFILE)g_pOrgFunc)(hFile, lpBuffer, nBytesToWrite, lpBytesWrittern, lpOverlapped);
}

BOOL hook_iat(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew) {
	LPCSTR szLibName;
	PIMAGE_THUNK_DATA pThunk;
	DWORD dwOldProtect;

	//pAddr���浱ǰPE�ļ���ImageBase��ַ
	HMODULE hMod = GetModuleHandle(NULL);
	PBYTE pAddr = (PBYTE)hMod;

	//pAddrָ��NTͷ��ʼ(ƫ������DOSͷ�����4�ֽڶ���)
	pAddr += *((DWORD*)&pAddr[0x3C]);

	//dwRVA���DataDirectory[1]�ĵ�һ��4�ֽ�����, ��Import Table��RVA
	DWORD dwRVA = *((DWORD*)&pAddr[0x80]);

	//pImportDescָ��IMAGE_IMPORT_DESCRIPTOR�������ʼ
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hMod+dwRVA);

	//����IMAGE_IMPORT_DESCRIPTOR����
	for( ; pImportDesc->Name; pImportDesc++ ){
		szLibName = (LPCSTR)((DWORD)hMod + pImportDesc->Name);
		if( !_stricmp(szLibName, szDllName) ) { 
			//�ҵ�����szDllName��Ӧ��IAT����pThunkָ��
			pThunk = (PIMAGE_THUNK_DATA)((DWORD)hMod + pImportDesc->FirstThunk);

			for( ; pThunk->u1.Function; pThunk++ ){
				//��IAT���ҵ�pfnOrg��Ӧ�ĵ�ַ��
				if( pThunk->u1.Function == (DWORD)pfnOrg ){
					// �����ڴ����ԣ������޸�IAT
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
					//�޸�IAT��pfnOrg������ַΪpfnNew������ַ
                    pThunk->u1.Function = (DWORD)pfnNew;
					// ���ڴ����ԸĻ�
                    VirtualProtect((LPVOID)&pThunk->u1.Function, 4,	dwOldProtect, &dwOldProtect);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch( fdwReason ){
		case DLL_PROCESS_ATTACH : 
            //����ȡ��Ŀ��API�ĵ�ַ
           	g_pOrgFunc = GetProcAddress(GetModuleHandle(L"kernel32.dll"),"WriteFile");
            // ��ȡ��kernel32.WriteFile()��MyDll4.MyWriteFile()��ȡ
			hook_iat("kernel32.dll", g_pOrgFunc, (PROC)MyWriteFile);
			break;
		case DLL_PROCESS_DETACH :
            //�ѹ�����kernel32.WriteFile()��ԭ��ַ�ָ�IAT
            hook_iat("kernel32.dll", (PROC)MyWriteFile, g_pOrgFunc);
			break;
	}
	return TRUE;
}
