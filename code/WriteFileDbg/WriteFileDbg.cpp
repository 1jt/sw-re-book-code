#include "windows.h"
#include "stdio.h"

LPVOID pWriteFile = NULL;
HANDLE hProcess;
HANDLE hThread;
BYTE INT3 = 0xCC;
BYTE OriginalByte = 0;

BOOL OnCreateProcessDebugEvent(LPDEBUG_EVENT pde){
	// ���WriteFile()�ĺ�����ַ
	pWriteFile = GetProcAddress(GetModuleHandleA("kernel32.dll"), "WriteFile");

	hProcess = ((CREATE_PROCESS_DEBUG_INFO)pde->u.CreateProcessInfo).hProcess;
	hThread = ((CREATE_PROCESS_DEBUG_INFO)pde->u.CreateProcessInfo).hThread;

	// ��WriteFile()�ĵ�һ�ֽڱ��浽OriginalByte�У���0xCCд��WriteFile()��һ�ֽ�
	ReadProcessMemory(hProcess, pWriteFile, &OriginalByte, sizeof(BYTE), NULL);
	WriteProcessMemory(hProcess, pWriteFile, &INT3, sizeof(BYTE), NULL);
	return TRUE;
}

BOOL OnExceptionDebugEvent(LPDEBUG_EVENT pde){
	CONTEXT ctx;
	DWORD nBytesToWrite, dataAddr, i;
	PEXCEPTION_RECORD per = &pde->u.Exception.ExceptionRecord;

	if( EXCEPTION_BREAKPOINT == per->ExceptionCode ){ //�������쳣�¼��Ƕϵ��쳣INT3
		if( pWriteFile == per->ExceptionAddress ){ //�ϵ��ַ��WriteFile()����ʼ��ַ
			//��WriteFile()��ʼ��0xCC�Ļ�ԭʼ����
			WriteProcessMemory(hProcess, pWriteFile, &OriginalByte, sizeof(BYTE), NULL);

			//����߳�������
			ctx.ContextFlags = CONTEXT_CONTROL;
			GetThreadContext(hThread, &ctx);

			//��ȡWriteFile()�ĵ�2,3����,��ϣ��д������ݵ���ʼ��ַ���ֽ���
			ReadProcessMemory(hProcess, (LPVOID)(ctx.Esp+0x8), &dataAddr, sizeof(DWORD), NULL);
			ReadProcessMemory(hProcess, (LPVOID)(ctx.Esp+0xC), &nBytesToWrite, sizeof(DWORD), NULL);

			//����һ����ʱ�ռ������޸�Ҫд���ļ�������
			PBYTE buf = (PBYTE)malloc(nBytesToWrite+1);
			memset(buf, 0, nBytesToWrite+1);

			//���ڶ���������ָ���Ҫд����������ݶ�ȡ����ʱ�ռ�,�����޸�
			ReadProcessMemory(hProcess, (LPVOID)dataAddr, buf, nBytesToWrite, NULL);

			for( i = 0; i < nBytesToWrite; i++ ){ //�޸�Ҫд�������
				if( 0x61 <= buf[i] && buf[i] <= 0x7A )
					buf[i] = 0x61 + (buf[i] - 0x61 + 0x1) % 0x1A;
			}

			//���޸ĺ����ʱ�ռ�����д�ص��ڶ���������ָ������ݵ�ַ
			WriteProcessMemory(hProcess, (LPVOID)dataAddr, buf, nBytesToWrite, NULL);

			free(buf); //�ͷ���ʱ�ռ�

			//���߳������ĵ�EIP��ΪWriteFile()����ʼ��ַ
			ctx.Eip = (DWORD)pWriteFile;
			SetThreadContext(hThread, &ctx);

			//���������Խ��̵�ִ��(�����Խ��̴������ctx.Eipλ�ü�WriteFile()��ʼ��ַ������)
			ContinueDebugEvent(pde->dwProcessId, pde->dwThreadId, DBG_CONTINUE);
			Sleep(0);

			// �ٴι�ȡWriteFile()
			WriteProcessMemory(hProcess, pWriteFile, &INT3, sizeof(BYTE), NULL);
			return TRUE;
		}
	}
	return FALSE;
}

int main(int argc, char* argv[]){
	DEBUG_EVENT de;

	if( argc != 2 )
		return 1;
	DWORD dwPID = atoi(argv[1]);
	if( !DebugActiveProcess(dwPID) )
		return 1;

	// �������ȴ��������߷��͵����¼�
	while( WaitForDebugEvent(&de, INFINITE) ) {
		if( CREATE_PROCESS_DEBUG_EVENT == de.dwDebugEventCode ) {
			OnCreateProcessDebugEvent(&de); //�������Խ��̴����򱻹�����������ʱִ��
		}
		else if( EXCEPTION_DEBUG_EVENT == de.dwDebugEventCode ) {
			if( OnExceptionDebugEvent(&de) ) //���쳣�¼�����ʱִ��
				continue;
		}
		else if( EXIT_PROCESS_DEBUG_EVENT == de.dwDebugEventCode ) { //�����Խ�����ֹ
			break; //��������֮��ֹ
		}
		//ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE); //�ٴ����б�������
		ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE); //�ٴ����б�������
	}
	return 0;
}