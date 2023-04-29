// dllmain.cpp : Définit le point d'entrée de l'application DLL.
//Construit d'apres un exemple de 
//https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

#include "pch.h"
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

#define BUFSIZE 4096 
#define titre "Création d'un processus enfant avec entrée et sortie redirigées"

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;
HANDLE g_hInputFile = NULL;
   PROCESS_INFORMATION piProcInfo; 
   STARTUPINFO siStartInfo;

void CreerProcessusEnfant(char* Instruction);
void EcrireVersPipe(void); 
void LireDepuisPipe(void); 
void AfficherErreur(PTSTR); 

BOOL APIENTRY DllMain( HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
    MessageBox(0, "Librairie créé par Patrice Waechter-Ebling 2023\nv:1.0.0.1\t Pipe Facile\nIl est permis de l'utiliser à condition de ne pas altérer l'iterration", titre, MB_ICONINFORMATION|MB_OK);
    return TRUE;
}

void CreerProcessusEnfant(char* Instruction)
{ 
   BOOL bSuccess = FALSE; 
   ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.hStdError = g_hChildStd_OUT_Wr;
   siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
   siStartInfo.hStdInput = g_hChildStd_IN_Rd;
   siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
   bSuccess = CreateProcess(NULL, Instruction,NULL,   NULL, TRUE, 0,NULL, NULL, &siStartInfo, &piProcInfo);  
   if ( ! bSuccess ) 
      AfficherErreur((PTSTR)"CreateProcess");
   else 
   {
      CloseHandle(piProcInfo.hProcess);
      CloseHandle(piProcInfo.hThread);
      CloseHandle(g_hChildStd_OUT_Wr);
      CloseHandle(g_hChildStd_IN_Rd);
   }
}
void EcrireVersPipe(void) 
{ 
   DWORD dwRead, dwWritten; 
   CHAR chBuf[BUFSIZE];
   BOOL bSuccess = FALSE; 
   for (;;) 
   { 
      bSuccess = ReadFile(g_hInputFile, chBuf, BUFSIZE, &dwRead, NULL);
      if ( ! bSuccess || dwRead == 0 ) break;       
      bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
      if ( ! bSuccess ) break; 
   } 
   if ( ! CloseHandle(g_hChildStd_IN_Wr) ) 
      AfficherErreur((PTSTR)"StdInWr CloseHandle"); 
} 
 
void LireDepuisPipe(void) 
{ 
   DWORD dwRead, dwWritten; 
   CHAR chBuf[BUFSIZE]; 
   BOOL bSuccess = FALSE;
   HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   for (;;) 
   { 
      bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
      if( ! bSuccess || dwRead == 0 ) break; 
      bSuccess = WriteFile(hParentStdOut, chBuf, dwRead, &dwWritten, NULL);
      if (! bSuccess ) break; 
   } 
} 
 
void AfficherErreur(PTSTR lpszFunction) 
{ 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s a retourné l'erreur 0x%x: %s"), lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf,titre, MB_OK); 
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}
/*
* exemple simple 
* 
#include <windows.h>
#include <stdio.h>

#define BUFSIZE 4096 
 
int main(void) 
{ 
   CHAR chBuf[BUFSIZE]; 
   BOOL bSuccess; 
   HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
   HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
   if ( 
       (hStdout == INVALID_HANDLE_VALUE) || 
       (hStdin == INVALID_HANDLE_VALUE) 
      ) 
      ExitProcess(1); 
   printf("\n ** Programme de base explicatif. ** \n");
   for (;;) 
   { 
      bSuccess = ReadFile(hStdin, chBuf, BUFSIZE, &dwRead, NULL); 
      if (! bSuccess || dwRead == 0) break; 
      bSuccess = WriteFile(hStdout, chBuf, dwRead, &dwWritten, NULL); 
      if (! bSuccess)break; 
   } 
   return 0;
}
*/
