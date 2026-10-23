#ifndef WINMINI_H
#define WINMINI_H
#include <stddef.h>
#if !defined(_WIN64)
#error x64 only
#endif
#define WINAPI __stdcall
#define DLLIMPORT __declspec(dllimport)
#define EXTERN_C extern
#define NULLPTR ((void*)0)
typedef void* HANDLE;
typedef void* HMODULE;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef unsigned long DWORD;
typedef char _assert_win_wchar_is_2_bytes[(sizeof(wchar_t)==2)?1:-1];
typedef int BOOL;
typedef unsigned long long SIZE_T;
typedef unsigned long long ULONGLONG;
typedef long long LONGLONG;
typedef unsigned long long ULONG_PTR;
typedef const wchar_t* LPCWSTR;
typedef wchar_t* LPWSTR;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef void* (*LPTHREAD_START_ROUTINE)(void*);
#define TRUE 1
#define FALSE 0
#define INVALID_FILE_ATTRIBUTES ((DWORD)0xFFFFFFFF)
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define GENERIC_WRITE 0x40000000
#define GENERIC_READ 0x80000000
#define FILE_SHARE_WRITE 2
#define OPEN_EXISTING 3
#define FILE_SHARE_READ 1
#define CREATE_ALWAYS 2
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_END 2
#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define MEM_RELEASE 0x8000
#define PAGE_READWRITE 0x04
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_READ 0x20
#define PAGE_READONLY 0x02
#define PAGE_WRITECOPY 0x08
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_NOACCESS 0x01
#define PAGE_GUARD 0x100
#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258
#define STILL_ACTIVE 259
#define INFINITE 0xFFFFFFFF
#define TH32CS_SNAPPROCESS 0x00000002
#define TH32CS_SNAPMODULE 0x00000008
#define TH32CS_SNAPMODULE32 0x00000010
#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT 2
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 4
#define STARTF_USESHOWWINDOW 1
#define CREATE_NO_WINDOW 0x08000000
#define STD_INPUT_HANDLE ((DWORD)-10)
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define STD_ERROR_HANDLE ((DWORD)-12)
#define MAX_PATH_W 32768

typedef struct {
  DWORD cb;
  LPWSTR lpReserved;
  LPWSTR lpDesktop;
  LPWSTR lpTitle;
  DWORD dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars;
  DWORD dwFillAttribute;
  DWORD dwFlags;
  WORD wShowWindow;
  WORD cbReserved2;
  BYTE* lpReserved2;
  HANDLE hStdInput, hStdOutput, hStdError;
} STARTUPINFOW;
typedef struct { HANDLE hProcess; HANDLE hThread; DWORD dwProcessId; DWORD dwThreadId; } PROCESS_INFORMATION;
typedef struct {
  DWORD dwSize;
  DWORD th32ModuleID;
  DWORD th32ProcessID;
  DWORD GlblcntUsage;
  DWORD ProccntUsage;
  BYTE* modBaseAddr;
  DWORD modBaseSize;
  HMODULE hModule;
  wchar_t szModule[256];
  wchar_t szExePath[260];
} MODULEENTRY32W;
typedef struct {
  DWORD dwSize;
  DWORD cntUsage;
  DWORD th32ProcessID;
  ULONG_PTR th32DefaultHeapID;
  DWORD th32ModuleID;
  DWORD cntThreads;
  DWORD th32ParentProcessID;
  long pcPriClassBase;
  DWORD dwFlags;
  wchar_t szExeFile[260];
} PROCESSENTRY32W;
typedef struct {
  LPVOID lpBaseAddress;
  LPVOID lpAllocationBase;
  DWORD AllocationProtect;
  DWORD _padding1;
  SIZE_T RegionSize;
  DWORD State;
  DWORD Protect;
  DWORD Type;
  DWORD _padding2;
} MEMORY_BASIC_INFORMATION;

DLLIMPORT LPCWSTR WINAPI GetCommandLineW(void);
DLLIMPORT HMODULE WINAPI GetModuleHandleW(LPCWSTR);
DLLIMPORT DWORD WINAPI GetModuleFileNameW(HMODULE,LPWSTR,DWORD);
DLLIMPORT DWORD WINAPI GetEnvironmentVariableW(LPCWSTR,LPWSTR,DWORD);
DLLIMPORT DWORD WINAPI GetFileAttributesW(LPCWSTR);
DLLIMPORT BOOL WINAPI GetModuleHandleExW(DWORD,LPCWSTR,HMODULE*);
DLLIMPORT void* WINAPI GetProcAddress(HMODULE,LPCSTR);
DLLIMPORT HANDLE WINAPI CreateThread(LPVOID,SIZE_T,LPTHREAD_START_ROUTINE,LPVOID,DWORD,DWORD*);
DLLIMPORT void WINAPI Sleep(DWORD);
DLLIMPORT BOOL WINAPI CloseHandle(HANDLE);
DLLIMPORT HANDLE WINAPI CreateFileW(LPCWSTR,DWORD,DWORD,LPVOID,DWORD,DWORD,HANDLE);
DLLIMPORT BOOL WINAPI WriteFile(HANDLE,LPCVOID,DWORD,DWORD*,LPVOID);
DLLIMPORT BOOL WINAPI WriteConsoleW(HANDLE,LPCVOID,DWORD,DWORD*,LPVOID);
DLLIMPORT int WINAPI WideCharToMultiByte(unsigned int,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,BOOL*);
DLLIMPORT BOOL WINAPI ReadFile(HANDLE,LPVOID,DWORD,DWORD*,LPVOID);
DLLIMPORT BOOL WINAPI FlushFileBuffers(HANDLE);
DLLIMPORT DWORD WINAPI GetLastError(void);
DLLIMPORT void WINAPI ExitProcess(DWORD);
DLLIMPORT LPVOID WINAPI VirtualAlloc(LPVOID,SIZE_T,DWORD,DWORD);
DLLIMPORT BOOL WINAPI VirtualFree(LPVOID,SIZE_T,DWORD);
DLLIMPORT BOOL WINAPI VirtualProtect(LPVOID,SIZE_T,DWORD,DWORD*);
DLLIMPORT SIZE_T WINAPI VirtualQuery(LPCVOID,MEMORY_BASIC_INFORMATION*,SIZE_T);
DLLIMPORT BOOL WINAPI FlushInstructionCache(HANDLE,LPCVOID,SIZE_T);
DLLIMPORT HANDLE WINAPI GetCurrentProcess(void);
DLLIMPORT BOOL WINAPI DisableThreadLibraryCalls(HMODULE);
DLLIMPORT BOOL WINAPI CreateProcessW(LPCWSTR,LPWSTR,LPVOID,LPVOID,BOOL,DWORD,LPVOID,LPCWSTR,STARTUPINFOW*,PROCESS_INFORMATION*);
DLLIMPORT HANDLE WINAPI CreateToolhelp32Snapshot(DWORD,DWORD);
DLLIMPORT BOOL WINAPI Process32FirstW(HANDLE,PROCESSENTRY32W*);
DLLIMPORT BOOL WINAPI Process32NextW(HANDLE,PROCESSENTRY32W*);
DLLIMPORT BOOL WINAPI Module32FirstW(HANDLE,MODULEENTRY32W*);
DLLIMPORT BOOL WINAPI Module32NextW(HANDLE,MODULEENTRY32W*);
DLLIMPORT LPVOID WINAPI VirtualAllocEx(HANDLE,LPVOID,SIZE_T,DWORD,DWORD);
DLLIMPORT BOOL WINAPI VirtualFreeEx(HANDLE,LPVOID,SIZE_T,DWORD);
DLLIMPORT BOOL WINAPI WriteProcessMemory(HANDLE,LPVOID,LPCVOID,SIZE_T,SIZE_T*);
DLLIMPORT HANDLE WINAPI CreateRemoteThread(HANDLE,LPVOID,SIZE_T,LPTHREAD_START_ROUTINE,LPVOID,DWORD,DWORD*);
DLLIMPORT DWORD WINAPI WaitForSingleObject(HANDLE,DWORD);
DLLIMPORT BOOL WINAPI GetExitCodeThread(HANDLE,DWORD*);
DLLIMPORT BOOL WINAPI TerminateProcess(HANDLE, unsigned int);
DLLIMPORT BOOL WINAPI GetExitCodeProcess(HANDLE,DWORD*);
DLLIMPORT DWORD WINAPI GetCurrentProcessId(void);
DLLIMPORT BOOL WINAPI DeleteFileW(LPCWSTR);
DLLIMPORT HANDLE WINAPI GetStdHandle(DWORD);
DLLIMPORT void WINAPI OutputDebugStringA(LPCSTR);
DLLIMPORT BOOL WINAPI SetConsoleOutputCP(unsigned int);
DLLIMPORT WORD WINAPI GetUserDefaultUILanguage(void);
DLLIMPORT WORD WINAPI GetSystemDefaultUILanguage(void);
DLLIMPORT DWORD WINAPI GetTickCount(void);

static unsigned long long wlen(const wchar_t* s){unsigned long long n=0;if(s)while(s[n])++n;return n;}
static unsigned long long slen(const char* s){unsigned long long n=0;if(s)while(s[n])++n;return n;}
static int sweq(const wchar_t*a,const wchar_t*b){if(!a||!b)return 0;while(*a&&*b){wchar_t x=*a++,y=*b++;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y)return 0;}return *a==*b;}
static int abytes_eq(const void*a,const void*b,SIZE_T n){const BYTE*x=(const BYTE*)a,*y=(const BYTE*)b;for(SIZE_T i=0;i<n;i++)if(x[i]!=y[i])return 0;return 1;}
static void acopy(void*dst,const void*src,SIZE_T n){BYTE*d=(BYTE*)dst;const BYTE*s=(const BYTE*)src;for(SIZE_T i=0;i<n;i++)d[i]=s[i];}
static void zero_mem(void*p,SIZE_T n){BYTE*b=(BYTE*)p;for(SIZE_T i=0;i<n;i++)b[i]=0;}
static int wappend(wchar_t*d,SIZE_T cap,const wchar_t*s){SIZE_T pos=wlen(d),n=wlen(s);if(pos+n+1>cap)return 0;for(SIZE_T i=0;i<=n;i++)d[pos+i]=s[i];return 1;}
static void out(const char*s){HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);if(h&&h!=(HANDLE)(-1)){DWORD n=0;WriteFile(h,s,(DWORD)slen(s),&n,0);}}
/* Unicode console output works on Chinese Windows 7 without depending on the batch file code page.
   When output is redirected, write UTF-8 bytes instead of silently dropping text. */
static void outw(const wchar_t*s){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    if(!h || h==(HANDLE)(-1) || !s)return;
    DWORD written=0;
    if(WriteConsoleW(h,s,(DWORD)wlen(s),&written,0))return;
    char utf8[1024];
    int n=WideCharToMultiByte(65001,0,s,-1,utf8,(int)sizeof(utf8),0,0);
    if(n>1)WriteFile(h,utf8,(DWORD)(n-1),&written,0);
}
static void u32toa(DWORD value,char*outbuf){char b[16];int n=0;do{b[n++]=(char)('0'+value%10);value/=10;}while(value&&n<15);for(int i=0;i<n;i++)outbuf[i]=b[n-1-i];outbuf[n]=0;}
#endif

/* No CRT: provide a bounded, non-optimized memset for compiler-generated zeroing. */
void* memset(void *dest, int value, SIZE_T count) {
  volatile unsigned char *d=(volatile unsigned char*)dest;
  while(count--) *d++=(unsigned char)value;
  return dest;
}
