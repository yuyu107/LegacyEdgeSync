#include "winmini.h"

static wchar_t edgeExe[1024];
static wchar_t dllPath[1024];
static wchar_t profile[1024];
static wchar_t statusPath[1024];
static wchar_t cmdline[4096];
static wchar_t localOwnerPath[1024];

static int append_path(wchar_t*dst,SIZE_T cap,const wchar_t*s){return wappend(dst,cap,s);}
static int init_paths(void){
    zero_mem(edgeExe,sizeof(edgeExe));zero_mem(dllPath,sizeof(dllPath));
    zero_mem(profile,sizeof(profile));zero_mem(statusPath,sizeof(statusPath));zero_mem(cmdline,sizeof(cmdline));
    if(!GetModuleFileNameW(0,dllPath,1024))return 0;
    SIZE_T i=wlen(dllPath);
    while(i>0 && dllPath[i-1]!=L'\\' && dllPath[i-1]!=L'/')i--;
    dllPath[i]=0;
    if(!append_path(dllPath,1024,L"Edge109SyncHook.dll"))return 0;
    if(GetFileAttributesW(dllPath)==INVALID_FILE_ATTRIBUTES)return 0;
    DWORD n=GetEnvironmentVariableW(L"ProgramFiles(x86)",edgeExe,1024);
    if(n==0||n>=1024){zero_mem(edgeExe,sizeof(edgeExe));n=GetEnvironmentVariableW(L"ProgramFiles",edgeExe,1024);}
    if(n==0||n>=1024)return 0;
    if(!append_path(edgeExe,1024,L"\\Microsoft\\Edge\\Application\\msedge.exe"))return 0;
    if(GetFileAttributesW(edgeExe)==INVALID_FILE_ATTRIBUTES)return 0;
    n=GetEnvironmentVariableW(L"TEMP",profile,1024);
    if(n==0||n>=1024)return 0;
    if(!append_path(profile,1024,L"\\Edge109SyncFix-Test"))return 0;
    n=GetEnvironmentVariableW(L"TEMP",statusPath,1024);
    if(n==0||n>=1024)return 0;
    if(!append_path(statusPath,1024,L"\\Edge109SyncFix-status.txt"))return 0;
    if(!append_path(cmdline,4096,L"\"") || !append_path(cmdline,4096,edgeExe) || !append_path(cmdline,4096,L"\" --user-data-dir=\"") ||
      !append_path(cmdline,4096,profile) || !append_path(cmdline,4096,L"\" --no-first-run --disable-background-mode --sync-url=\"https://edge.microsoft.com/sync/v1/feeds/me/syncEntities\""))return 0;
    return 1;
}
static int inject(PROCESS_INFORMATION*pi){
    HMODULE localKernel=GetModuleHandleW(L"kernel32.dll");
    if(!localKernel)return 0;
    void*lp=(void*)GetProcAddress(localKernel,"LoadLibraryW");
    if(!lp)return 0;
    HMODULE owner=0;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)lp,&owner))return 0;
    zero_mem(localOwnerPath,sizeof(localOwnerPath));
    if(!GetModuleFileNameW(owner,localOwnerPath,1024))return 0;
    SIZE_T pos=wlen(localOwnerPath);
    while(pos>0 && localOwnerPath[pos-1]!=L'\\' && localOwnerPath[pos-1]!=L'/')pos--;
    wchar_t*modname=localOwnerPath+pos;
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pi->dwProcessId);
    if(snap==(HANDLE)(-1)||!snap)return 0;
    MODULEENTRY32W me;zero_mem(&me,sizeof(me));me.dwSize=sizeof(me);
    BYTE*remoteBase=0;
    if(Module32FirstW(snap,&me)){
        do{if(sweq(me.szModule,modname)){remoteBase=me.modBaseAddr;break;}}while(Module32NextW(snap,&me));
    }
    CloseHandle(snap);
    if(!remoteBase)return 0;
    ULONGLONG offset=(ULONGLONG)(ULONG_PTR)lp-(ULONGLONG)(ULONG_PTR)owner;
    LPTHREAD_START_ROUTINE remoteFn=(LPTHREAD_START_ROUTINE)(remoteBase+offset);
    SIZE_T bytes=(wlen(dllPath)+1)*sizeof(wchar_t);
    LPVOID remotePath=VirtualAllocEx(pi->hProcess,0,bytes,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(!remotePath)return 0;
    SIZE_T written=0;
    if(!WriteProcessMemory(pi->hProcess,remotePath,dllPath,bytes,&written) || written!=bytes){VirtualFreeEx(pi->hProcess,remotePath,0,MEM_RELEASE);return 0;}
    HANDLE remoteThread=CreateRemoteThread(pi->hProcess,0,0,remoteFn,remotePath,0,0);
    if(!remoteThread){VirtualFreeEx(pi->hProcess,remotePath,0,MEM_RELEASE);return 0;}
    DWORD wait=WaitForSingleObject(remoteThread,20000);
    if(wait==WAIT_OBJECT_0){
        // Return value of LoadLibraryW is 64 bits; GetExitCodeThread exposes only 32 bits.
        // The DLL writes a status file that is the authoritative success signal.
        CloseHandle(remoteThread);
        VirtualFreeEx(pi->hProcess,remotePath,0,MEM_RELEASE);
        return 1;
    }
    // Do not free the remote parameter on timeout; the remote thread might still read it.
    CloseHandle(remoteThread);
    return 0;
}
void WINAPI mainCRTStartup(void){
    out("Edge 109 SyncFix v0.1-test3 (Win7 x64)\r\n");
    out("Experimental. Do not use a daily Microsoft account or enable password sync.\r\n");
    if(!init_paths()){out("ERROR: Could not locate Edge executable or Edge109SyncHook.dll.\r\n");ExitProcess(2);}
    DeleteFileW(statusPath);
    STARTUPINFOW si;PROCESS_INFORMATION pi;
    zero_mem(&si,sizeof(si));zero_mem(&pi,sizeof(pi));si.cb=sizeof(si);
    if(!CreateProcessW(edgeExe,cmdline,0,0,FALSE,0,0,0,&si,&pi)){
        out("ERROR: Cannot start Edge. Windows GetLastError: ");char s[16];u32toa(GetLastError(),s);out(s);out("\r\n");ExitProcess(3);
    }
    out("Started isolated Edge test profile. Attempting immediate hook injection...\r\n");
    // Chromium browser startup has no reliable input-idle boundary.
    // WaitForInputIdle is exported by USER32.dll, not KERNEL32.dll.
    // Deliberately omit the API to avoid an extra import.
    // Previous build waited 2500ms and could miss automatic sync after reboot.
    // Inject as soon as remote kernel32 has been mapped, no fixed idle wait.
    int injected=0;
    for(int attempt=0;attempt<160;attempt++){
        if(inject(&pi)){injected=1;break;}
        DWORD exitcode=0;
        if(GetExitCodeProcess(pi.hProcess,&exitcode) && exitcode!=STILL_ACTIVE){
            out("ERROR: Edge exited before hook injection.\r\n");break;
        }
        Sleep(25);
    }
    if(!injected){
        out("ERROR: Remote module injection failed. GetLastError: ");char s[16];u32toa(GetLastError(),s);out(s);out("\r\n");
        out("Edge is still running without a validated patch. Quit it normally.\r\n");
    }else{
        out("Module load requested. Checking status (up to 20 seconds)...\r\n");
        int ok=0;
        for(int i=0;i<40;i++){
            if(GetFileAttributesW(statusPath)!=INVALID_FILE_ATTRIBUTES){ok=1;break;}
            Sleep(500);
        }
        if(ok){out("Status file created. Open via 02-Open-Status.cmd and verify INSTALLED.\r\n");}
        else{out("WARNING: No module status file. Injection may have failed or been blocked.\r\n");}
    }
    out("Check hook status and patch counters. If sync had failed before installation, sign out/in in TEST profile.\r\n");
    out("Do not close this window until you have read the messages.\r\n");
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
    ExitProcess(0);
}
