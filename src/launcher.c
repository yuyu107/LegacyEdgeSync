#include "winmini.h"

/* LegacyEdgeSync v0.2-test3: Windows user display-language auto-detection (zh-CN/zh-TW/en). */
/* Keep the verified test3 remote DLL injection implementation unchanged. */
/* User UI language (not ANSI code page / geographic locale). Windows 7 compatible. */
enum { LANG_ZH_CN=0, LANG_ZH_TW=1, LANG_EN=2 };
static int g_language=LANG_EN;
static const wchar_t* TR(const wchar_t*cn,const wchar_t*tw,const wchar_t*en){
    return g_language==LANG_ZH_CN?cn:(g_language==LANG_ZH_TW?tw:en);
}
static void detect_language(void){
    WORD ui=GetUserDefaultUILanguage();
    if(!ui)ui=GetSystemDefaultUILanguage();
    if((ui&0x3FF)!=0x04){g_language=LANG_EN;return;}
    // zh-TW=0404, zh-HK=0c04, zh-MO=1404 (traditional); zh-CN=0804, zh-SG=1004 (simplified).
    WORD sub=(WORD)(ui>>10);
    g_language=(sub==1 || sub==3 || sub==5 || sub==0x1F)?LANG_ZH_TW:LANG_ZH_CN;
}
static int wstarts(const wchar_t*p,const wchar_t*part){
    while(*part){if(!*p || *p++!=*part++)return 0;}return 1;
}
static void apply_language_override(const wchar_t*cmd){
    // ASCII-only switches; case-sensitive by design. Actual detection is the default.
    for(const wchar_t*p=cmd;p && *p;p++){
      if(p>cmd && p[-1]!=L' ' && p[-1]!=L'\t')continue;
      if(wstarts(p,L"--lang=zh-CN") && (p[12]==0 || p[12]==L' ')){g_language=LANG_ZH_CN;return;}
      if(wstarts(p,L"--lang=zh-TW") && (p[12]==0 || p[12]==L' ')){g_language=LANG_ZH_TW;return;}
      if(wstarts(p,L"--lang=en") && (p[9]==0 || p[9]==L' ')){g_language=LANG_EN;return;}
    }
}
static const wchar_t* language_flag(void){return g_language==LANG_ZH_CN?L"--lang=zh-CN":(g_language==LANG_ZH_TW?L"--lang=zh-TW":L"--lang=en");}
static void print_language(void){
    outw(TR(L"界面语言：简体中文\r\n",L"介面語言：繁體中文\r\n",L"Interface language: English\r\n"));
}
static wchar_t edgeExe[1024];
static wchar_t dllPath[1024];
static wchar_t profile[1024];
static wchar_t statusPath[1024];
static wchar_t cmdline[4096];
static wchar_t localOwnerPath[1024];
static int normalMode=0;

static int append_path(wchar_t*dst,SIZE_T cap,const wchar_t*s){return wappend(dst,cap,s);}
static int has_flag(const wchar_t*cmd,const wchar_t*flag){
    SIZE_T n=wlen(flag);
    for(SIZE_T i=0;cmd && cmd[i];i++){
        if(i>0 && cmd[i-1]!=L' ' && cmd[i-1]!=L'\t')continue;
        if(cmd[i]!=L'-')continue;
        SIZE_T j=0;
        while(j<n && cmd[i+j]==flag[j])j++;
        if(j==n && (cmd[i+n]==0 || cmd[i+n]==L' ' || cmd[i+n]==L'\t'))return 1;
    }
    return 0;
}
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
    n=GetEnvironmentVariableW(L"TEMP",statusPath,1024);
    if(n==0||n>=1024)return 0;
    if(!append_path(statusPath,1024,L"\\Edge109SyncFix-status.txt"))return 0;
    if(!append_path(cmdline,4096,L"\"") || !append_path(cmdline,4096,edgeExe) || !append_path(cmdline,4096,L"\""))return 0;
    if(!normalMode){
        n=GetEnvironmentVariableW(L"TEMP",profile,1024);
        if(n==0||n>=1024)return 0;
        if(!append_path(profile,1024,L"\\Edge109SyncFix-Test"))return 0;
        if(!append_path(cmdline,4096,L" --user-data-dir=\"") ||
          !append_path(cmdline,4096,profile) || !append_path(cmdline,4096,L"\""))return 0;
    }
    if(!append_path(cmdline,4096,L" --no-first-run --disable-background-mode --sync-url=\"https://edge.microsoft.com/sync/v1/feeds/me/syncEntities\""))return 0;
    return 1;
}

/* Reject launching when any Edge process is already running.
   This is intentionally conservative: no forced termination, no existing-process injection. */
static int edge_processes_running(void){
    HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(!s || s==(HANDLE)(-1))return -1;
    PROCESSENTRY32W entry;
    zero_mem(&entry,sizeof(entry));entry.dwSize=sizeof(entry);
    int found=0;
    if(!Process32FirstW(s,&entry)){CloseHandle(s);return -1;}
    do{
        if(sweq(entry.szExeFile,L"msedge.exe")){found=1;break;}
    }while(Process32NextW(s,&entry));
    CloseHandle(s);
    return found;
}

/* Never allow default-profile mode to be selected without direct human acknowledgement. */
static int approve_normal_mode(void){
    outw(TR(L"\r\n[注意] 你选择了【原有 Edge 配置模式】。\r\n",L"\r\n[注意] 您選擇了【原有 Edge 設定檔模式】。\r\n",L"\r\n[Warning] You selected the existing Edge profile mode.\r\n"));
    outw(TR(L"将使用已保存的收藏夹、历史记录和微软账户登录状态。\r\n",L"將使用已儲存的我的最愛、瀏覽記錄及 Microsoft 帳戶登入狀態。\r\n",L"This will use your existing favorites, history, and Microsoft account sign-in.\r\n"));
    outw(TR(L"实验性 Hook 仍可能失败或导致浏览器崩溃，云同步也可能删除收藏夹。\r\n",L"實驗性 Hook 可能失敗或導致瀏覽器當機，雲端同步也可能刪除我的最愛。\r\n",L"The experimental hook may fail or crash Edge. Cloud sync may also delete favorites.\r\n"));
    outw(TR(L"请先关闭全部 Edge 进程，并运行【06-Backup-Original-Profile.cmd】。\r\n",L"請先關閉所有 Edge 處理程序，並執行【06-Backup-Original-Profile.cmd】。\r\n",L"Close all Edge processes and run 06-Backup-Original-Profile.cmd first.\r\n"));
    outw(TR(L"默认配置路径：%LOCALAPPDATA%\\Microsoft\\Edge\\User Data\r\n",L"預設設定檔路徑：%LOCALAPPDATA%\\Microsoft\\Edge\\User Data\r\n",L"Default profile path: %LOCALAPPDATA%\\Microsoft\\Edge\\User Data\r\n"));
    outw(TR(L"不建议启用密码和支付信息同步。\r\n",L"不建議啟用密碼及付款資訊同步。\r\n",L"Do not enable password or payment information sync.\r\n"));
    outw(TR(L"请输入大写 YES 再按回车确认（输入其他内容取消）：",L"請輸入大寫 YES 並按 Enter 確認（其他輸入將取消）：",L"Type YES in uppercase and press Enter to continue (anything else cancels): "));
    HANDLE input=GetStdHandle(STD_INPUT_HANDLE);
    if(!input || input==(HANDLE)(-1))return 0;
    char b[32];zero_mem(b,sizeof(b));DWORD got=0;
    if(!ReadFile(input,b,sizeof(b)-1,&got,0) || got<3)return 0;
    return b[0]=='Y' && b[1]=='E' && b[2]=='S' && (got==3 || b[3]=='\r' || b[3]=='\n');
}

static int status_file(char*buf,DWORD cap){
    if(!buf || cap<2 || !statusPath[0])return 0;
    buf[0]=0;
    HANDLE h=CreateFileW(statusPath,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    if(!h || h==(HANDLE)(-1))return 0;
    DWORD got=0;
    BOOL ok=ReadFile(h,buf,cap-1,&got,0);CloseHandle(h);
    if(!ok)return 0;
    buf[got]=0;
    return (int)got;
}
static int contains(const char*h,const char*needle){
    if(!h||!needle)return 0;
    for(SIZE_T i=0;h[i];i++){
        SIZE_T k=0;while(needle[k] && h[i+k] && h[i+k]==needle[k])k++;
        if(!needle[k])return 1;
    }
    return 0;
}
static DWORD get_pid(const char*txt){
    if(!txt)return 0;
    for(SIZE_T i=0;txt[i];i++){
        if(txt[i]=='p'&&txt[i+1]=='i'&&txt[i+2]=='d'&&txt[i+3]=='='){
            i+=4;DWORD pid=0;int digits=0;
            while(txt[i]>='0'&&txt[i]<='9'&&digits<10){pid=pid*10+(DWORD)(txt[i]-'0');i++;digits++;}
            if(digits)return pid;
        }
    }
    return 0;
}
static int running_edge_pid(DWORD pid){
    if(!pid)return 0;
    HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(!s || s==(HANDLE)(-1))return -1;
    PROCESSENTRY32W e;zero_mem(&e,sizeof(e));e.dwSize=sizeof(e);
    if(!Process32FirstW(s,&e)){CloseHandle(s);return -1;}
    int found=0;
    do{if(e.th32ProcessID==pid && sweq(e.szExeFile,L"msedge.exe")){found=1;break;}}while(Process32NextW(s,&e));
    CloseHandle(s);return found;
}
static int init_status_path(void){
    statusPath[0]=0;
    DWORD n=GetEnvironmentVariableW(L"TEMP",statusPath,1024);
    return n>0 && n<1024 && append_path(statusPath,1024,L"\\Edge109SyncFix-status.txt");
}
static void show_status(void){
    if(!init_status_path()){outw(TR(L"错误：无法获取临时目录路径。\r\n",L"錯誤：無法取得暫存資料夾路徑。\r\n",L"Error: Cannot determine the temporary directory.\r\n"));return;}
    char b[1536];
    if(!status_file(b,sizeof(b))){
       outw(TR(L"当前没有找到修复状态文件。请先通过启动器打开 Edge。\r\n",L"目前找不到修復狀態檔案。請先使用啟動器開啟 Edge。\r\n",L"No patch status file found. Start Edge with this launcher first.\r\n"));return;
    }
    outw(TR(L"\r\n======== 修复模块原始状态 ========\r\n",L"\r\n======== 修復模組原始狀態 ========\r\n",L"\r\n======== Raw patch status ========\r\n"));
    out(b);outw(TR(L"\r\n==================================\r\n",L"\r\n==================================\r\n",L"\r\n==================================\r\n"));
    DWORD pid=get_pid(b);
    if(pid){
        int r=running_edge_pid(pid);
        if(r==1)outw(TR(L"进程检查：状态文件对应的 Edge 进程仍在运行。\r\n",L"處理程序檢查：狀態檔案對應的 Edge 處理程序仍在執行。\r\n",L"Process check: The Edge process from this status file is running.\r\n"));
        else if(r==0)outw(TR(L"进程检查：记录的 Edge 进程已退出，以上可能是上次启动的旧状态。\r\n",L"處理程序檢查：記錄的 Edge 處理程序已結束，以上可能是上次啟動的舊狀態。\r\n",L"Process check: That Edge process has exited; the status may be stale.\r\n"));
        else outw(TR(L"进程检查：无法读取进程列表，请手动确认。\r\n",L"處理程序檢查：無法讀取處理程序清單，請手動確認。\r\n",L"Process check: Unable to read the process list. Check manually.\r\n"));
    }else outw(TR(L"进程检查：状态文件没有 PID，无法判断是否过期。\r\n",L"處理程序檢查：狀態檔案沒有 PID，無法判斷是否過期。\r\n",L"Process check: No PID in status file. Freshness cannot be verified.\r\n"));
    if(contains(b,"FAILED:"))outw(TR(L"结果：修复模块报告失败。请检查 Edge 版本和路径。\r\n",L"結果：修復模組回報失敗。請檢查 Edge 版本及路徑。\r\n",L"Result: Patch module reports failure. Check the Edge build and path.\r\n"));
    else if(contains(b,"INSTALLED:"))outw(TR(L"结果：Hook 已报告安装。还需要确认 patched 大于 0 且服务器同步成功。\r\n",L"結果：Hook 已回報安裝。仍需確認 patched 大於 0 且伺服器同步成功。\r\n",L"Result: Hook reports installed. Verify patched > 0 and server sync succeeds.\r\n"));
    else outw(TR(L"结果：尚未看到 Hook 安装完成的确认信息。\r\n",L"結果：尚未收到 Hook 安裝完成的確認訊息。\r\n",L"Result: No confirmation that the hook finished installing.\r\n"));
    outw(TR(L"请在 Edge 打开 edge://sync-internals/ 并检查 Transport State 是否为 Active。\r\n",L"請在 Edge 開啟 edge://sync-internals/，確認 Transport State 是否為 Active。\r\n",L"Open edge://sync-internals/ in Edge and verify Transport State is Active.\r\n"));
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


/* Built-in Unicode menu; scripts contain ASCII only and never force chcp 936/950. */
static void read_console_line(char*b,DWORD cap){
    if(!b||cap<2)return;
    zero_mem(b,cap);DWORD n=0;
    HANDLE h=GetStdHandle(STD_INPUT_HANDLE);
    if(h && h!=(HANDLE)(-1)){ReadFile(h,b,cap-1,&n,0);}
    b[cap-1]=0;
}
static void press_enter(void){
    outw(TR(L"\r\n按 Enter 返回菜单...",L"\r\n按 Enter 返回選單...",L"\r\nPress Enter to return to the menu..."));
    char b[64];read_console_line(b,sizeof(b));outw(L"\r\n");
}
static __declspec(noinline) DWORD run_self(const wchar_t*arg){
    static wchar_t self[1024],cmd[1300];zero_mem(self,sizeof(self));zero_mem(cmd,sizeof(cmd));
    if(!GetModuleFileNameW(0,self,1024))return 0xFFFFFFFF;
    if(!wappend(cmd,1300,L"\"") || !wappend(cmd,1300,self) || !wappend(cmd,1300,L"\" ") ||
       !wappend(cmd,1300,arg) || !wappend(cmd,1300,L" ") || !wappend(cmd,1300,language_flag()))return 0xFFFFFFFF;
    STARTUPINFOW si;PROCESS_INFORMATION pi;zero_mem(&si,sizeof(si));zero_mem(&pi,sizeof(pi));si.cb=sizeof(si);
    if(!CreateProcessW(self,cmd,0,0,FALSE,0,0,0,&si,&pi)){
      outw(TR(L"错误：无法启动子进程。\r\n",L"錯誤：無法啟動子處理程序。\r\n",L"Error: Could not start the requested operation.\r\n"));
      return 0xFFFFFFFF;
    }
    WaitForSingleObject(pi.hProcess,INFINITE);
    DWORD result=0xFFFFFFFF;GetExitCodeProcess(pi.hProcess,&result);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
    return result;
}
static __declspec(noinline) void show_guide(void){
    outw(TR(L"\r\n===== 退出与恢复说明 =====\r\n",
            L"\r\n===== 結束與還原說明 =====\r\n",
            L"\r\n===== Exit and recovery =====\r\n"));
    outw(TR(L"1. 正常关闭所有 Edge 窗口与后台进程。\r\n",
            L"1. 正常關閉所有 Edge 視窗與背景處理程序。\r\n",
            L"1. Close all Edge windows and background processes normally.\r\n"));
    outw(TR(L"2. Hook 只存在于 Edge 进程内存中，进程结束后失效。\r\n",
            L"2. Hook 僅存在於 Edge 處理程序記憶體中，結束後便失效。\r\n",
            L"2. The hook exists only in Edge process memory and disappears on exit.\r\n"));
    outw(TR(L"3. 本工具不会修改磁盘上的 msedge.dll 或安装 HTTPS 根证书。\r\n",
            L"3. 本工具不會修改磁碟上的 msedge.dll 或安裝 HTTPS 根憑證。\r\n",
            L"3. Original msedge.dll files and HTTPS root certificates are unchanged.\r\n"));
    outw(TR(L"4. 原有配置模式的同步可能更改本地及云端收藏夹。\r\n",
            L"4. 原有設定檔模式的同步可能變更本機及雲端我的最愛。\r\n",
            L"4. Sync in existing-profile mode may change local and cloud favorites.\r\n"));
    outw(TR(L"5. 恢复配置备份前，请先退出 Edge 并谨慎操作。\r\n",
            L"5. 還原設定檔備份前，請先結束 Edge 並謹慎操作。\r\n",
            L"5. Close Edge before restoring a profile backup. Proceed carefully.\r\n"));
    outw(TR(L"6. 账户头像显示异常仍是已知问题，本版不处理。\r\n",
            L"6. 帳戶頭像無法顯示仍為已知問題，本版尚未修復。\r\n",
            L"6. Account avatars may remain missing; not addressed in this build.\r\n"));
}
static void append_decimal(wchar_t*buf,SIZE_T cap,DWORD num){
    char v[16];u32toa(num,v);
    wchar_t w[16];int i=0;
    while(v[i] && i<15){w[i]=(wchar_t)(BYTE)v[i];i++;}
    w[i]=0;wappend(buf,cap,w);
}
static __declspec(noinline) int backup_original_profile(void){
    static wchar_t src[1024],dst[1024],robo[1024],command[4096];
    zero_mem(src,sizeof(src));zero_mem(dst,sizeof(dst));zero_mem(robo,sizeof(robo));zero_mem(command,sizeof(command));
    outw(TR(L"\r\n===== 备份 Edge 原有配置 =====\r\n",
            L"\r\n===== 備份 Edge 原有設定檔 =====\r\n",
            L"\r\n===== Back up existing Edge profile =====\r\n"));
    int existing=edge_processes_running();
    if(existing!=0){
      outw(TR(L"错误：Edge 仍在运行或无法检测进程；请关闭后重试。\r\n",
              L"錯誤：Edge 仍在執行或無法偵測處理程序；請關閉後重試。\r\n",
              L"Error: Edge is running or the process check failed. Exit Edge first.\r\n"));return 1;
    }
    DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",src,1024);
    if(!n||n>=1024 || !wappend(src,1024,L"\\Microsoft\\Edge\\User Data") ||
      (GetFileAttributesW(src)&FILE_ATTRIBUTE_DIRECTORY)==0 || GetFileAttributesW(src)==INVALID_FILE_ATTRIBUTES){
      outw(TR(L"错误：找不到原有 Edge 配置目录。\r\n",
              L"錯誤：找不到原有 Edge 設定檔資料夾。\r\n",
              L"Error: Existing Edge profile directory was not found.\r\n"));return 2;
    }
    n=GetEnvironmentVariableW(L"USERPROFILE",dst,1024);
    if(!n||n>=1024 || !wappend(dst,1024,L"\\Desktop\\LegacyEdgeSync-EdgeBackup-") )return 3;
    append_decimal(dst,1024,GetTickCount());
    wappend(dst,1024,L"-");append_decimal(dst,1024,GetCurrentProcessId());
    if(GetFileAttributesW(dst)!=INVALID_FILE_ATTRIBUTES){
      outw(TR(L"错误：备份目标已存在，已取消。\r\n",L"錯誤：備份目標已存在，已取消。\r\n",L"Error: Backup destination already exists. Cancelled.\r\n"));return 4;
    }
    outw(TR(L"备份来源：",L"備份來源：",L"Source: "));outw(src);outw(L"\r\n");
    outw(TR(L"备份位置：",L"備份位置：",L"Destination: "));outw(dst);outw(L"\r\n");
    outw(TR(L"正在使用 Windows Robocopy 复制文件，请勿关闭窗口...\r\n",
            L"正在使用 Windows Robocopy 複製檔案，請勿關閉視窗...\r\n",
            L"Copying files using Windows Robocopy. Do not close this window...\r\n"));
    n=GetEnvironmentVariableW(L"SystemRoot",robo,1024);
    if(!n||n>=1024 || !wappend(robo,1024,L"\\System32\\robocopy.exe") || GetFileAttributesW(robo)==INVALID_FILE_ATTRIBUTES){
      outw(TR(L"错误：找不到系统 Robocopy 程序。\r\n",L"錯誤：找不到系統 Robocopy 程式。\r\n",L"Error: Windows Robocopy executable was not found.\r\n"));return 5;
    }
    if(!wappend(command,4096,L"\"") || !wappend(command,4096,robo) || !wappend(command,4096,L"\" \"") ||
      !wappend(command,4096,src) || !wappend(command,4096,L"\" \"") || !wappend(command,4096,dst) ||
      !wappend(command,4096,L"\" /E /R:1 /W:1 /XJ /COPY:DAT"))return 6;
    STARTUPINFOW si;PROCESS_INFORMATION pi;zero_mem(&si,sizeof(si));zero_mem(&pi,sizeof(pi));si.cb=sizeof(si);
    if(!CreateProcessW(robo,command,0,0,FALSE,0,0,0,&si,&pi)){
      outw(TR(L"错误：无法运行 Robocopy。\r\n",L"錯誤：無法執行 Robocopy。\r\n",L"Error: Could not start Robocopy.\r\n"));return 7;
    }
    WaitForSingleObject(pi.hProcess,INFINITE);
    DWORD result=0xFFFFFFFF;GetExitCodeProcess(pi.hProcess,&result);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
    if(result>=8){
      outw(TR(L"警告：Robocopy 报告备份失败，错误码：",L"警告：Robocopy 回報備份失敗，錯誤碼：",L"Warning: Robocopy reported failure; exit code: "));
      char code[16];u32toa(result,code);out(code);outw(L"\r\n");return 8;
    }
    outw(TR(L"备份完成：",L"備份完成：",L"Backup completed: "));outw(dst);outw(L"\r\n");return 0;
}
static __declspec(noinline) void menu_loop(void){
    for(;;){
      outw(L"\r\n============================================================\r\n");
      outw(L"LegacyEdgeSync v0.2-test3\r\n");
      outw(TR(L"旧版 Edge 云同步兼容工具（实验版）\r\n",
              L"舊版 Edge 雲端同步相容性工具（實驗版）\r\n",
              L"Legacy Edge cloud sync compatibility tool (experimental)\r\n"));
      print_language();
      outw(L"============================================================\r\n");
      outw(TR(L"  [1] 正常启动（原有 Edge 配置）\r\n",
              L"  [1] 正常啟動（原有 Edge 設定檔）\r\n",
              L"  [1] Normal launch (existing Edge profile)\r\n"));
      outw(TR(L"  [2] 独立测试模式\r\n",
              L"  [2] 獨立測試模式\r\n",
              L"  [2] Isolated test mode\r\n"));
      outw(TR(L"  [3] 查看修复状态\r\n",
              L"  [3] 查看修復狀態\r\n",
              L"  [3] View patch status\r\n"));
      outw(TR(L"  [4] 备份原有 Edge 配置\r\n",
              L"  [4] 備份原有 Edge 設定檔\r\n",
              L"  [4] Back up existing Edge profile\r\n"));
      outw(TR(L"  [5] 退出及恢复说明\r\n",
              L"  [5] 結束與還原說明\r\n",
              L"  [5] Exit and recovery information\r\n"));
      outw(TR(L"  [0] 退出\r\n",
              L"  [0] 結束\r\n",
              L"  [0] Exit\r\n"));
      outw(TR(L"\r\n请选择操作：",L"\r\n請選擇操作：",L"\r\nSelect an option: "));
      char input[64];read_console_line(input,sizeof(input));
      char key=input[0];
      if(key=='0')return;
      else if(key=='1')run_self(L"--normal");
      else if(key=='2')run_self(L"--test");
      else if(key=='3')run_self(L"--status");
      else if(key=='4')run_self(L"--backup");
      else if(key=='5')run_self(L"--guide");
      else{outw(TR(L"无效选项。\r\n",L"無效選項。\r\n",L"Invalid option.\r\n"));continue;}
      press_enter();
    }
}
void WINAPI mainCRTStartup(void){
    detect_language();
    apply_language_override(GetCommandLineW());
    outw(TR(L"LegacyEdgeSync v0.2-test3 | 旧版 Edge 云同步兼容实验版\r\n",L"LegacyEdgeSync v0.2-test3 | 舊版 Edge 雲端同步相容性實驗版\r\n",L"LegacyEdgeSync v0.2-test3 | Experimental legacy Edge sync patch\r\n"));
    LPCWSTR cmd=GetCommandLineW();
    int t=has_flag(cmd,L"--test"), n=has_flag(cmd,L"--normal");
    if(has_flag(cmd,L"--help") || has_flag(cmd,L"-h")){
        outw(TR(L"用法：Edge109SyncFix.exe [--menu | --test | --normal | --status | --backup | --guide]\r\n",L"用法：Edge109SyncFix.exe [--menu | --test | --normal | --status | --backup | --guide]\r\n",L"Usage: Edge109SyncFix.exe [--menu | --test | --normal | --status | --backup | --guide]\r\n"));
        outw(TR(L"  --test    独立测试配置（默认）\r\n",L"  --test    獨立測試設定檔（預設）\r\n",L"  --test    Isolated test profile (default)\r\n"));
        outw(TR(L"  --normal  使用 Edge 原有配置（需要输入 YES）\r\n",L"  --normal  使用 Edge 原有設定檔（須輸入 YES）\r\n",L"  --normal  Use the existing Edge profile (requires YES)\r\n"));
        outw(TR(L"  --status  查看修复状态（不会启动浏览器）\r\n",L"  --status  查看修復狀態（不啟動瀏覽器）\r\n",L"  --status  Show patch status (does not start Edge)\r\n"));
        outw(TR(L"  --menu    打开菜单\r\n  --backup  备份原有配置\r\n  --guide   查看退出说明\r\n",
                L"  --menu    開啟選單\r\n  --backup  備份原有設定檔\r\n  --guide   查看結束說明\r\n",
                L"  --menu    Show main menu\r\n  --backup  Back up existing profile\r\n  --guide   Show exit guidance\r\n"));
        outw(TR(L"语言：默认根据系统显示语言选择；可使用 --lang=zh-CN、--lang=zh-TW 或 --lang=en 覆盖。\r\n",
                L"語言：預設依系統顯示語言選擇；可使用 --lang=zh-CN、--lang=zh-TW 或 --lang=en 覆寫。\r\n",
                L"Language: Automatic from Windows display language; override with --lang=zh-CN, --lang=zh-TW, or --lang=en.\r\n"));
        ExitProcess(0);
    }
    if(has_flag(cmd,L"--menu")){menu_loop();ExitProcess(0);}
    if(has_flag(cmd,L"--guide")){show_guide();ExitProcess(0);}
    if(has_flag(cmd,L"--backup")){ExitProcess((DWORD)backup_original_profile());}
    if(has_flag(cmd,L"--status")){show_status();ExitProcess(0);}
    if(t && n){outw(TR(L"错误：不能同时选择测试模式和原有配置模式。\r\n",L"錯誤：不可同時選擇測試模式與原有設定檔模式。\r\n",L"Error: --test and --normal cannot be combined.\r\n"));ExitProcess(2);}
    normalMode=n?1:0;
    if(!init_paths()){
        outw(TR(L"错误：找不到 Edge109SyncHook.dll 或 Edge 安装文件。\r\n",L"錯誤：找不到 Edge109SyncHook.dll 或 Edge 安裝檔案。\r\n",L"Error: Edge109SyncHook.dll or the Edge executable was not found.\r\n"));
        outw(TR(L"请确认 EXE 和 DLL 在同一文件夹，并安装指定的 Edge 109 x64 版本。\r\n",L"請確認 EXE 和 DLL 位於同一資料夾，且已安裝指定的 Edge 109 x64 版本。\r\n",L"Keep the EXE and DLL together and install the specified Edge 109 x64 build.\r\n"));
        ExitProcess(3);
    }
    int existing=edge_processes_running();
    if(existing<0){outw(TR(L"错误：无法枚举进程。为避免覆盖已运行的 Edge，本次拒绝启动。\r\n",L"錯誤：無法列舉處理程序。為避免影響執行中的 Edge，本次拒絕啟動。\r\n",L"Error: Cannot enumerate processes. Refusing to start to avoid conflicts.\r\n"));ExitProcess(5);}
    if(existing){
        outw(TR(L"错误：检测到 msedge.exe 正在运行。请先退出全部 Edge 窗口和后台进程。\r\n",L"錯誤：偵測到 msedge.exe 正在執行。請先關閉所有 Edge 視窗及背景處理程序。\r\n",L"Error: msedge.exe is already running. Exit all Edge windows and background processes.\r\n"));
        outw(TR(L"本工具不会强制关闭现有浏览器，也不会向已有进程注入补丁。\r\n",L"本工具不會強制關閉現有瀏覽器，也不會對現有處理程序注入修復模組。\r\n",L"This tool will not kill existing browsers or inject into an existing process.\r\n"));
        ExitProcess(6);
    }
    if(normalMode){if(!approve_normal_mode()){outw(TR(L"已取消原有配置模式，没有启动 Edge。\r\n",L"已取消原有設定檔模式，未啟動 Edge。\r\n",L"Existing-profile mode cancelled. Edge was not started.\r\n"));ExitProcess(4);}}
    else outw(TR(L"启动模式：独立测试配置（不会直接使用原有 Edge 账户数据）。\r\n",L"啟動模式：獨立測試設定檔（不會直接使用原有 Edge 帳戶資料）。\r\n",L"Launch mode: Isolated test profile (existing account data is not used directly).\r\n"));
    DeleteFileW(statusPath);
    STARTUPINFOW si;PROCESS_INFORMATION pi;
    zero_mem(&si,sizeof(si));zero_mem(&pi,sizeof(pi));si.cb=sizeof(si);
    if(!CreateProcessW(edgeExe,cmdline,0,0,FALSE,0,0,0,&si,&pi)){
        outw(TR(L"错误：Edge 启动失败。Windows 错误代码：",L"錯誤：Edge 啟動失敗。Windows 錯誤代碼：",L"Error: Edge failed to start. Windows error code: "));char code[16];u32toa(GetLastError(),code);out(code);outw(L"\r\n");ExitProcess(7);
    }
    outw(TR(L"已启动 Edge，浏览器进程 PID：",L"已啟動 Edge，瀏覽器處理程序 PID：",L"Edge started. Browser PID: "));char pid[16];u32toa(pi.dwProcessId,pid);out(pid);outw(L"\r\n");
    outw(normalMode?TR(L"当前模式：原有配置。\r\n",L"目前模式：原有設定檔。\r\n",L"Current mode: Existing profile.\r\n"):TR(L"当前模式：独立测试配置。\r\n",L"目前模式：獨立測試設定檔。\r\n",L"Current mode: Isolated test profile.\r\n"));
    outw(TR(L"正在尽早加载运行时修复模块……\r\n",L"正在盡早載入執行階段修復模組……\r\n",L"Loading the runtime patch module as early as possible...\r\n"));
    int injected=0;
    for(int attempt=0;attempt<160;attempt++){
        if(inject(&pi)){injected=1;break;}
        DWORD exitcode=0;
        if(GetExitCodeProcess(pi.hProcess,&exitcode) && exitcode!=STILL_ACTIVE){
            outw(TR(L"错误：Edge 在注入前就已退出。\r\n",L"錯誤：Edge 在注入前已結束。\r\n",L"Error: Edge exited before patch injection.\r\n"));break;
        }
        Sleep(25);
    }
    if(!injected){
        outw(TR(L"错误：无法加载修复 DLL。最后的 Windows 错误代码：",L"錯誤：無法載入修復 DLL。最後的 Windows 錯誤代碼：",L"Error: Cannot load the patch DLL. Last Windows error: "));
        char code[16];u32toa(GetLastError(),code);out(code);outw(L"\r\n");
        outw(TR(L"Edge 可能仍在运行，但未应用补丁。请正常关闭浏览器。\r\n",L"Edge 可能仍在執行，但未套用修復。請正常關閉瀏覽器。\r\n",L"Edge may still be running without the patch. Close it normally.\r\n"));
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);ExitProcess(8);
    }
    outw(TR(L"DLL 加载请求已完成，正在等待 Hook 状态确认……\r\n",L"DLL 載入要求已完成，正在等待 Hook 狀態確認……\r\n",L"DLL load requested. Waiting for hook status confirmation...\r\n"));
    char b[1536];int ok=0;
    for(int i=0;i<40;i++){
        int count=status_file(b,sizeof(b));
        if(count>0){
            if(contains(b,"FAILED:")){ok=-1;break;}
            if(contains(b,"INSTALLED:")){ok=1;break;}
        }
        Sleep(500);
    }
    if(ok==1){outw(TR(L"Hook 已报告安装成功，但还需要等待实际请求命中。\r\n",L"Hook 已回報安裝成功，但仍須等待實際請求命中。\r\n",L"Hook reports installed. Actual requests still need to be patched.\r\n"));}
    else if(ok==-1){
        outw(TR(L"修复模块报告失败，以下为原始诊断状态：\r\n",L"修復模組回報失敗，以下為原始診斷狀態：\r\n",L"Patch module reported failure. Raw diagnostic status follows:\r\n"));out(b);outw(L"\r\n");
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);ExitProcess(9);
    }else{
        outw(TR(L"警告：等待 Hook 安装确认超时，实际修复状态尚未确定。\r\n",L"警告：等待 Hook 安裝確認逾時，實際修復狀態尚未確定。\r\n",L"Warning: Timed out waiting for hook installation; state is unknown.\r\n"));
    }
    outw(TR(L"请运行【02-Show-Status.cmd】查看命中次数和修改次数。\r\n",L"請執行【02-Show-Status.cmd】查看命中次數及修改次數。\r\n",L"Run 02-Show-Status.cmd to inspect hit and patch counters.\r\n"));
    outw(TR(L"再在 Edge 打开 edge://sync-internals/，检查同步是否为 Active。\r\n",L"接著在 Edge 開啟 edge://sync-internals/，確認同步狀態為 Active。\r\n",L"Open edge://sync-internals/ in Edge and check for Active sync status.\r\n"));
    outw(TR(L"本工具不会自动关闭 Edge，也不会删除用户配置。\r\n",L"本工具不會自動關閉 Edge，也不會刪除使用者設定檔。\r\n",L"This tool will not close Edge or delete user profile data.\r\n"));
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
    ExitProcess(ok==1?0:10);
}
