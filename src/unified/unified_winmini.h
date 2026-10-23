#ifndef LEGACYEDGE_X86MINI_H
#define LEGACYEDGE_X86MINI_H
/* A tiny Windows 7 x86 header without CRT/Windows SDK dependency. */
#if !defined(_M_IX86) || defined(_WIN64)
#error LegacyEdgeSync x86 research build requires 32-bit x86
#endif
#define WINAPI __stdcall
#define CDECL __cdecl
#define DLLIMPORT __declspec(dllimport)
#define IMP(ret,name,args) DLLIMPORT ret WINAPI name args __asm__("_" #name)
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned int SIZE_T;
typedef unsigned int ULONG_PTR;
typedef int BOOL;
typedef unsigned short wchar16;
typedef void* HANDLE;
typedef void* HMODULE;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef wchar16* LPWSTR;
typedef const wchar16* LPCWSTR;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef DWORD (WINAPI *THREADPROC)(LPVOID);
#define FALSE 0
#define TRUE 1
#define MAXW 1024
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFFU
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_SHARE_READ 1
#define FILE_SHARE_WRITE 2
#define GENERIC_READ 0x80000000U
#define GENERIC_WRITE 0x40000000U
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define MEM_RELEASE 0x8000
#define PAGE_READWRITE 4
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_READONLY 2
#define PAGE_READWRITE 4
#define PAGE_WRITECOPY 8
#define PAGE_EXECUTE_READ 0x20
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD 0x100
#define PAGE_NOACCESS 1
#define TH32CS_SNAPPROCESS 2
#define TH32CS_SNAPMODULE 8
#define TH32CS_SNAPMODULE32 0x10
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 0x4
#define GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT 0x2
#define STD_INPUT_HANDLE ((DWORD)-10)
#define STD_OUTPUT_HANDLE ((DWORD)-11)
#define CP_UTF8 65001U
#define STILL_ACTIVE 259
#define INFINITE 0xFFFFFFFFU
#define DLL_PROCESS_ATTACH 1
#define MEM_COMMIT 0x1000
#define MAX_PATH 260

typedef struct _STARTUPINFOW {
 DWORD cb; LPWSTR lpReserved; LPWSTR lpDesktop; LPWSTR lpTitle; DWORD dwX,dwY,dwXSize,dwYSize,dwXCountChars,dwYCountChars,dwFillAttribute,dwFlags; WORD wShowWindow,cbReserved2; BYTE* lpReserved2; HANDLE hStdInput,hStdOutput,hStdError;
} STARTUPINFOW;
typedef struct _PROCESS_INFORMATION { HANDLE hProcess,hThread;DWORD dwProcessId,dwThreadId;} PROCESS_INFORMATION;
typedef struct _PROCESSENTRY32W {
 DWORD dwSize,cntUsage,th32ProcessID;ULONG_PTR th32DefaultHeapID;DWORD th32ModuleID,cntThreads,th32ParentProcessID;long pcPriClassBase;DWORD dwFlags;wchar16 szExeFile[260];
} PROCESSENTRY32W;
typedef struct _MODULEENTRY32W {
 DWORD dwSize,th32ModuleID,th32ProcessID,GlblcntUsage,ProccntUsage;BYTE* modBaseAddr;DWORD modBaseSize;HMODULE hModule;wchar16 szModule[256];wchar16 szExePath[260];
} MODULEENTRY32W;
typedef struct _MEMORY_BASIC_INFORMATION {
 LPVOID BaseAddress,AllocationBase;DWORD AllocationProtect;SIZE_T RegionSize;DWORD State,Protect,Type;
} MEMORY_BASIC_INFORMATION;
IMP(HMODULE,GetModuleHandleW,(LPCWSTR));
IMP(HMODULE,LoadLibraryW,(LPCWSTR));
IMP(DWORD,GetModuleFileNameW,(HMODULE,LPWSTR,DWORD));
IMP(DWORD,GetEnvironmentVariableW,(LPCWSTR,LPWSTR,DWORD));
IMP(DWORD,GetFileAttributesW,(LPCWSTR));
IMP(BOOL,GetModuleHandleExW,(DWORD,LPCWSTR,HMODULE*));
IMP(void*,GetProcAddress,(HMODULE,LPCSTR));
IMP(HANDLE,CreateThread,(LPVOID,SIZE_T,THREADPROC,LPVOID,DWORD,DWORD*));
IMP(void,Sleep,(DWORD));
IMP(HANDLE,CreateFileW,(LPCWSTR,DWORD,DWORD,LPVOID,DWORD,DWORD,HANDLE));
IMP(BOOL,WriteFile,(HANDLE,LPCVOID,DWORD,DWORD*,LPVOID));
IMP(BOOL,ReadFile,(HANDLE,LPVOID,DWORD,DWORD*,LPVOID));
IMP(BOOL,CloseHandle,(HANDLE));
IMP(DWORD,GetLastError,(void));
IMP(void,ExitProcess,(DWORD));
IMP(LPVOID,VirtualAlloc,(LPVOID,SIZE_T,DWORD,DWORD));
IMP(BOOL,VirtualFree,(LPVOID,SIZE_T,DWORD));
IMP(BOOL,VirtualProtect,(LPVOID,SIZE_T,DWORD,DWORD*));
IMP(SIZE_T,VirtualQuery,(LPCVOID,MEMORY_BASIC_INFORMATION*,SIZE_T));
IMP(BOOL,FlushInstructionCache,(HANDLE,LPCVOID,SIZE_T));
IMP(HANDLE,GetCurrentProcess,(void));
IMP(BOOL,CreateProcessW,(LPCWSTR,LPWSTR,LPVOID,LPVOID,BOOL,DWORD,LPVOID,LPCWSTR,STARTUPINFOW*,PROCESS_INFORMATION*));
IMP(HANDLE,CreateToolhelp32Snapshot,(DWORD,DWORD));
IMP(BOOL,Process32FirstW,(HANDLE,PROCESSENTRY32W*));
IMP(BOOL,Process32NextW,(HANDLE,PROCESSENTRY32W*));
IMP(BOOL,Module32FirstW,(HANDLE,MODULEENTRY32W*));
IMP(BOOL,Module32NextW,(HANDLE,MODULEENTRY32W*));
IMP(LPVOID,VirtualAllocEx,(HANDLE,LPVOID,SIZE_T,DWORD,DWORD));
IMP(BOOL,VirtualFreeEx,(HANDLE,LPVOID,SIZE_T,DWORD));
IMP(BOOL,WriteProcessMemory,(HANDLE,LPVOID,LPCVOID,SIZE_T,SIZE_T*));
IMP(HANDLE,CreateRemoteThread,(HANDLE,LPVOID,SIZE_T,THREADPROC,LPVOID,DWORD,DWORD*));
IMP(DWORD,WaitForSingleObject,(HANDLE,DWORD));
IMP(BOOL,GetExitCodeThread,(HANDLE,DWORD*));
IMP(BOOL,GetExitCodeProcess,(HANDLE,DWORD*));
IMP(DWORD,GetCurrentProcessId,(void));
IMP(BOOL,DeleteFileW,(LPCWSTR));
IMP(HANDLE,GetStdHandle,(DWORD));
IMP(BOOL,WriteConsoleW,(HANDLE,LPCVOID,DWORD,DWORD*,LPVOID));
IMP(BOOL,DisableThreadLibraryCalls,(HMODULE));
IMP(void,OutputDebugStringA,(LPCSTR));
IMP(LPCWSTR,GetCommandLineW,(void));
IMP(WORD,GetUserDefaultUILanguage,(void));
IMP(WORD,GetSystemDefaultUILanguage,(void));
IMP(DWORD,GetTickCount,(void));
IMP(int,WideCharToMultiByte,(unsigned int,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,BOOL*));

static SIZE_T wslen(LPCWSTR s){SIZE_T n=0;if(s)while(s[n])n++;return n;}
static SIZE_T slen(const char*s){SIZE_T n=0;if(s)while(s[n])n++;return n;}
static int wsame(LPCWSTR a,LPCWSTR b){if(!a||!b)return 0;while(*a&&*b){wchar16 x=*a++,y=*b++;if(x>='A'&&x<='Z')x+=32;if(y>='A'&&y<='Z')y+=32;if(x!=y)return 0;}return *a==*b;}
static void zero_mem(void*p,SIZE_T n){volatile BYTE*b=(volatile BYTE*)p;while(n--)*b++=0;}
static void copy_mem(void*d,const void*s,SIZE_T n){BYTE*a=(BYTE*)d;const BYTE*b=(const BYTE*)s;while(n--)*a++=*b++;}
static int eq_bytes(const BYTE*a,const BYTE*b,SIZE_T n){while(n--)if(*a++!=*b++)return 0;return 1;}
static int wappend(wchar16*d,SIZE_T cap,LPCWSTR s){SIZE_T n=wslen(d),z=wslen(s);if(n+z+1>cap)return 0;copy_mem(d+n,s,z*2);d[n+z]=0;return 1;}
static void output(const char*t){HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);DWORD w;if(h&&(ULONG_PTR)h!=(ULONG_PTR)-1)WriteFile(h,t,(DWORD)slen(t),&w,0);}
static void outnum(DWORD v){char tmp[16],out[16];int i=0;do{tmp[i++]=(char)('0'+v%10);v/=10;}while(v&&i<15);for(int j=0;j<i;j++)out[j]=tmp[i-j-1];out[i]=0;output(out);}
#endif
/* Unified launcher: Windows 7 system directory for a trusted version.dll load. */
IMP(DWORD,GetSystemDirectoryW,(LPWSTR,DWORD));
