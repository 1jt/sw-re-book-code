#include "windows.h"
#include "tchar.h"
#include "stdio.h"
 
int _tmain(int argc, LPTSTR argv[]){
	char szA[100];
	WCHAR szW[100];
	
	sprintf_s(szA, "%S", L"Unicode Str");//��Unicode�ַ���ת��ΪANSI�ַ���
	printf("%s\n", szA);

	swprintf_s(szW, L"%S", "ANSI Str");	//��ANSI�ַ���ת��ΪUnicode�ַ���
	wprintf(L"%s\n",szW);

	while(TRUE);
	return TRUE;
}