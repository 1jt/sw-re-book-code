#include "windows.h"

typedef BOOL (WINAPI *PFWRITEFILE)(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytesToWrite, LPDWORD lpBytesWrittern, LPOVERLAPPED lpOverlapped);

BYTE g_pOrgBytes[5] = {0,};

BOOL hook_by_code(LPCSTR szDllName, 		//����Ҫ��ȡ��API��DLL����
					LPCSTR szFuncName, 		//Ҫ��ȡ��API����
					PROC pfnNew, 			//�û��ṩ�Ĺ�ȡ�����ĵ�ַ
					PBYTE pOrgBytes) {		//�洢ԭ5�ֽ�ָ��Ļ������������ѹ�
	DWORD dwOldProtect;
	BYTE pBuf[5] = {0xE9, 0, };

	//��ȡҪ��ȡ��API��ַ
	FARPROC pfnOrg = (FARPROC)GetProcAddress(GetModuleHandleA(szDllName), szFuncName);
	PBYTE pByte = (PBYTE)pfnOrg;

	if( pByte[0] == 0xE9 ) //Ŀ�꺯���Ѿ�����ȡ��
		return FALSE;

	//����ǰ5�ֽڵĶ�дȨ��
	VirtualProtect((LPVOID)pfnOrg, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	//����kernel32.WriteFile��ԭʼǰ5�ֽ�ָ�pOrgBytes
	memcpy(pOrgBytes, pfnOrg, 5);

	//����JMP������Ե�ַ��ʹpBuf��5�ֽ�ΪJMP������Ե�ַ
	DWORD dwAddress = (DWORD)pfnNew - (DWORD)pfnOrg - 5;
	memcpy(&pBuf[1], &dwAddress, 4);

	//��5�ֽ�JMPָ���滻kernel32.WriteFile��ԭʼ��ʼ5�ֽ�
	memcpy(pfnOrg, pBuf, 5);

	//�ָ�ǰ5�ֽڵĶ�дȨ��
	VirtualProtect((LPVOID)pfnOrg, 5, dwOldProtect, &dwOldProtect);
	return TRUE;
}

BOOL unhook_by_code(LPCSTR szDllName, LPCSTR szFuncName, PBYTE pOrgBytes) {
    DWORD dwOldProtect;

    //��ȡҪ�ѹ���API��ַ
    FARPROC pFunc = GetProcAddress(GetModuleHandleA(szDllName), szFuncName);
    PBYTE pByte = (PBYTE)pFunc;

    //����ѹ��������һ�����ֽڲ���JMPָ��ĺ�������ֱ�ӷ���
    if( pByte[0] != 0xE9 )
        return FALSE;

    // ����ǰ5�ֽڵĶ�дȨ��
    VirtualProtect((LPVOID)pFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

    //��kernel32.WriteFile��ԭʼǰ5�ֽ��滻��ǰ����ǰ5�ֽڵ�JMPָ��
    memcpy(pFunc, pOrgBytes, 5);

    //�ָ�ǰ5�ֽڵĶ�дȨ��
    VirtualProtect((LPVOID)pFunc, 5, dwOldProtect, &dwOldProtect);
    return TRUE;
}

BOOL WINAPI MyWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytesToWrite, LPDWORD lpBytesWrittern, LPOVERLAPPED lpOverlapped){
	unhook_by_code("kernel32.dll", "WriteFile", g_pOrgBytes);

	FARPROC pFunc = GetProcAddress(GetModuleHandleA("kernel32.dll"), "WriteFile");	//���Ŀ��API��ַ
	DWORD i = 0;
	LPSTR lpString=(LPSTR)lpBuffer;
	for(i = 0; i < nBytesToWrite; i++){
		if( 0x61 <= lpString[i] && lpString[i] <= 0x7A ){
			lpString[i] = 0x61 + (lpString[i]-0x60)%0x1A;
		}
	}
	BOOL status = ((PFWRITEFILE)pFunc)(hFile, lpBuffer, nBytesToWrite, lpBytesWrittern, lpOverlapped);

	hook_by_code("kernel32.dll", "WriteFile", (PROC)MyWriteFile, g_pOrgBytes);
	return status;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, 
					LPVOID lpvReserved){
    switch( fdwReason ){
        case DLL_PROCESS_ATTACH : 
        	hook_by_code("kernel32.dll", "WriteFile", (PROC)MyWriteFile, g_pOrgBytes);
        	break;
        case DLL_PROCESS_DETACH :
	        unhook_by_code("kernel32.dll", "WriteFile", g_pOrgBytes);
    	    break;
    }
    return TRUE;
}
