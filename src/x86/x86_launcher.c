#include "x86mini.h"
/* LegacyEdgeSync v0.3-test3 x86 frontend.
 * Sync hook DLL and remote injection routine originate unchanged from verified v0.3-test2.
 * Launcher only: multilingual console, existing profile mode, backup and safety/status UX.
 */
enum {LANG_ZH_CN=0,LANG_ZH_TW=1,LANG_EN=2};
static int gLanguage=LANG_EN;
static int normalMode=0;
static wchar16 edgeExe[MAXW],dllPath[MAXW],profile[MAXW],statusPath[MAXW],cmdline[4096];

static LPCWSTR TR(LPCWSTR cn,LPCWSTR tw,LPCWSTR en){
 return gLanguage==LANG_ZH_CN?cn:(gLanguage==LANG_ZH_TW?tw:en);
}
static void outw(LPCWSTR s){
 HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);if(!h||(ULONG_PTR)h==(ULONG_PTR)-1||!s)return;
 DWORD n=0;
 if(WriteConsoleW(h,s,(DWORD)wslen(s),&n,0))return;
 static char utf8[8192];int length=WideCharToMultiByte(CP_UTF8,0,s,-1,utf8,sizeof(utf8),0,0);
 if(length>1)WriteFile(h,utf8,(DWORD)(length-1),&n,0);
}
static void detect_language(void){
 WORD id=GetUserDefaultUILanguage();if(!id)id=GetSystemDefaultUILanguage();
 if((id&0x3ff)!=0x04){gLanguage=LANG_EN;return;}
 WORD sub=(WORD)(id>>10);
 gLanguage=(sub==1||sub==3||sub==5||sub==0x1f)?LANG_ZH_TW:LANG_ZH_CN;
}
static int wprefix(LPCWSTR p,LPCWSTR q){while(*q){if(!*p||*p++!=*q++)return 0;}return 1;}
static void language_override(LPCWSTR cmd){
 if(!cmd)return;
 for(LPCWSTR p=cmd;*p;p++){
  if(p!=cmd&&p[-1]!=L' '&&p[-1]!=L'\t')continue;
  if(wprefix(p,L"--lang=zh-CN")&&(p[12]==0||p[12]==L' '||p[12]==L'\t')){gLanguage=LANG_ZH_CN;return;}
  if(wprefix(p,L"--lang=zh-TW")&&(p[12]==0||p[12]==L' '||p[12]==L'\t')){gLanguage=LANG_ZH_TW;return;}
  if(wprefix(p,L"--lang=en")&&(p[9]==0||p[9]==L' '||p[9]==L'\t')){gLanguage=LANG_EN;return;}
 }
}
static LPCWSTR langflag(void){
 return gLanguage==LANG_ZH_CN?L"--lang=zh-CN":(gLanguage==LANG_ZH_TW?L"--lang=zh-TW":L"--lang=en");
}
static int flag(LPCWSTR cmd,LPCWSTR name){
 SIZE_T len=wslen(name);
 for(SIZE_T i=0;cmd&&cmd[i];i++){
  if(i&&cmd[i-1]!=L' '&&cmd[i-1]!=L'\t')continue;
  SIZE_T j=0;while(j<len&&cmd[i+j]==name[j])j++;
  if(j==len&&(cmd[i+j]==0||cmd[i+j]==L' '||cmd[i+j]==L'\t'))return 1;
 }return 0;
}
static int path_env(LPCWSTR key,wchar16*dst,SIZE_T cap,LPCWSTR suffix){
 zero_mem(dst,cap*2);DWORD n=GetEnvironmentVariableW(key,dst,(DWORD)cap);
 return n>0&&n<cap&&wappend(dst,cap,suffix);
}
static int status_path(void){return path_env(L"TEMP",statusPath,MAXW,L"\\LegacyEdgeSync-x86-status.txt");}
static int set_paths(void){
 zero_mem(edgeExe,sizeof(edgeExe));zero_mem(dllPath,sizeof(dllPath));
 zero_mem(profile,sizeof(profile));zero_mem(cmdline,sizeof(cmdline));
 if(!GetModuleFileNameW(0,dllPath,MAXW))return 0;
 SIZE_T n=wslen(dllPath);while(n&&dllPath[n-1]!=L'\\'&&dllPath[n-1]!=L'/')n--;
 dllPath[n]=0;
 if(!wappend(dllPath,MAXW,L"LegacyEdgeSyncHook32.dll"))return 0;
 if(GetFileAttributesW(dllPath)==INVALID_FILE_ATTRIBUTES)return 0;
 if(!path_env(L"ProgramFiles",edgeExe,MAXW,L"\\Microsoft\\Edge\\Application\\msedge.exe"))return 0;
 if(GetFileAttributesW(edgeExe)==INVALID_FILE_ATTRIBUTES)return 0;
 if(!status_path())return 0;
 if(!wappend(cmdline,4096,L"\"")||!wappend(cmdline,4096,edgeExe)||!wappend(cmdline,4096,L"\""))return 0;
 if(!normalMode){
  if(!path_env(L"TEMP",profile,MAXW,L"\\LegacyEdgeSync-x86-TestProfile-test3"))return 0;
  if(!wappend(cmdline,4096,L" --user-data-dir=\"")||!wappend(cmdline,4096,profile)||!wappend(cmdline,4096,L"\""))return 0;
 }
 return wappend(cmdline,4096,L" --no-first-run --disable-background-mode --sync-url=\"https://edge.microsoft.com/sync/v1/feeds/me/syncEntities\"");
}
/* Conservative process gate: do not tamper with Edge already running. */
static int edge_running(void){
 HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
 if(!s||(ULONG_PTR)s==(ULONG_PTR)-1)return -1;
 PROCESSENTRY32W e;zero_mem(&e,sizeof(e));e.dwSize=sizeof(e);
 if(!Process32FirstW(s,&e)){CloseHandle(s);return -1;}
 int found=0;do{if(wsame(e.szExeFile,L"msedge.exe")){found=1;break;}}while(Process32NextW(s,&e));
 CloseHandle(s);return found;
}
static int pid_running(DWORD pid){
 if(!pid)return 0;
 HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
 if(!s||(ULONG_PTR)s==(ULONG_PTR)-1)return -1;
 PROCESSENTRY32W e;zero_mem(&e,sizeof(e));e.dwSize=sizeof(e);
 if(!Process32FirstW(s,&e)){CloseHandle(s);return -1;}
 int found=0;do{if(e.th32ProcessID==pid&&wsame(e.szExeFile,L"msedge.exe")){found=1;break;}}while(Process32NextW(s,&e));
 CloseHandle(s);return found;
}
static void input_line(char*b,DWORD cap){
 if(!b||cap<2)return;zero_mem(b,cap);
 HANDLE in=GetStdHandle(STD_INPUT_HANDLE);
 if(in&&(ULONG_PTR)in!=(ULONG_PTR)-1){DWORD n=0;ReadFile(in,b,cap-1,&n,0);}
 b[cap-1]=0;
}
static int confirm_normal(void){
 outw(TR(L"\r\n[注意] 即将使用 Edge 原有用户配置。\r\n",L"\r\n[注意] 即將使用 Edge 原有使用者設定檔。\r\n",L"\r\n[Warning] Existing Edge profile mode selected.\r\n"));
 outw(TR(L"可能修改本地和云端收藏夹，或引起浏览器崩溃。\r\n",L"可能變更本機與雲端我的最愛，或導致瀏覽器當機。\r\n",L"This can change local/cloud favorites or crash the browser.\r\n"));
 outw(TR(L"请先使用 06-Backup-Original-Profile.cmd 备份原有配置。\r\n",L"請先用 06-Backup-Original-Profile.cmd 備份原有設定檔。\r\n",L"Back up the existing profile with 06-Backup-Original-Profile.cmd first.\r\n"));
 outw(TR(L"请勿同步密码、支付信息或其它敏感数据。\r\n",L"請勿同步密碼、付款資訊或其他敏感資料。\r\n",L"Do not sync passwords, payment data or other sensitive information.\r\n"));
 outw(TR(L"输入大写 YES 确认，其他内容取消：",L"輸入大寫 YES 確認，其他內容取消：",L"Type uppercase YES to proceed (anything else cancels): "));
 char b[32];input_line(b,sizeof(b));
 return b[0]=='Y'&&b[1]=='E'&&b[2]=='S'&&(b[3]=='\r'||b[3]=='\n'||b[3]==0);
}
static int read_status(char*b,DWORD cap){
 if(!status_path()||!b||cap<2)return 0;
 HANDLE h=CreateFileW(statusPath,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 if(!h||(ULONG_PTR)h==(ULONG_PTR)-1)return 0;
 DWORD got=0;int ok=ReadFile(h,b,cap-1,&got,0);CloseHandle(h);
 if(!ok)return 0;b[got]=0;return (int)got;
}
static int contains(const char*s,const char*q){
 if(!s||!q)return 0;for(SIZE_T i=0;s[i];i++){SIZE_T k=0;while(q[k]&&s[i+k]&&s[i+k]==q[k])k++;if(!q[k])return 1;}return 0;
}
static DWORD status_pid(const char*s){
 if(!s)return 0;
 for(SIZE_T i=0;s[i];i++)if(s[i]=='p'&&s[i+1]=='i'&&s[i+2]=='d'&&s[i+3]=='='){
  i+=4;DWORD n=0;int digit=0;while(s[i]>='0'&&s[i]<='9'&&digit<10){n=n*10+(DWORD)(s[i]-'0');i++;digit++;}
  return digit?n:0;
 }
 return 0;
}
static void show_status(void){
 char b[1536];zero_mem(b,sizeof(b));
 if(!read_status(b,sizeof(b))){outw(TR(L"找不到状态文件；先使用启动器运行 Edge。\r\n",L"找不到狀態檔案；請先以啟動器執行 Edge。\r\n",L"No status file found. Launch Edge through this tool first.\r\n"));return;}
 outw(TR(L"\r\n======== 原始 Hook 状态 ========\r\n",L"\r\n======== 原始 Hook 狀態 ========\r\n",L"\r\n======== Raw Hook status ========\r\n"));
 output(b);outw(L"\r\n=================================\r\n");
 DWORD pid=status_pid(b);int running=pid_running(pid);
 if(pid&&running==1)outw(TR(L"进程检查：对应的 Edge 仍在运行。\r\n",L"處理程序檢查：對應的 Edge 仍在執行。\r\n",L"Process check: The recorded Edge process is running.\r\n"));
 else if(pid&&running==0)outw(TR(L"进程检查：该 Edge 已退出；状态可能过期。\r\n",L"處理程序檢查：該 Edge 已結束；狀態可能過期。\r\n",L"Process check: The Edge process exited; status may be stale.\r\n"));
 else outw(TR(L"进程状态无法确认。\r\n",L"無法確認處理程序狀態。\r\n",L"Process status could not be confirmed.\r\n"));
 if(contains(b,"FAILED:"))outw(TR(L"Hook 报告失败，请检查目标版本和日志。\r\n",L"Hook 回報失敗，請檢查目標版本及記錄。\r\n",L"Hook reports failure. Check build and status details.\r\n"));
 else if(contains(b,"INSTALLED:"))outw(TR(L"Hook 已安装；请结合 patched 计数与实际同步判断是否成功。\r\n",L"Hook 已安裝；請結合 patched 計數與實際同步判斷是否成功。\r\n",L"Hook installed; verify patched counts and actual sync.\r\n"));
 outw(TR(L"请在 edge://sync-internals/ 检查服务器连接。\r\n",L"請在 edge://sync-internals/ 檢查伺服器連線。\r\n",L"Check server connection in edge://sync-internals/.\r\n"));
}

/* Tested x86 process injection implementation copied verbatim from v0.3-test2. */
static int remote_inject(PROCESS_INFORMATION*pi){
 HMODULE k=GetModuleHandleW(L"kernel32.dll");
 if(!k)return 0;
 BYTE*lp=(BYTE*)GetProcAddress(k,"LoadLibraryW");if(!lp)return 0;
 HMODULE localOwner=0;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)lp,&localOwner))return 0;
 wchar16 localName[1024];zero_mem(localName,sizeof(localName));
 if(!GetModuleFileNameW(localOwner,localName,1024))return 0;
 SIZE_T n=wslen(localName);while(n&&localName[n-1]!=L'\\'&&localName[n-1]!=L'/')n--;
 BYTE*remoteBase=0;
 HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pi->dwProcessId);
 if(!snap||(ULONG_PTR)snap==(ULONG_PTR)-1)return 0;
 MODULEENTRY32W entry;zero_mem(&entry,sizeof(entry));entry.dwSize=sizeof(entry);
 if(Module32FirstW(snap,&entry)){
  do{if(wsame(entry.szModule,localName+n)){remoteBase=entry.modBaseAddr;break;}}while(Module32NextW(snap,&entry));
 }
 CloseHandle(snap);if(!remoteBase)return 0;
 BYTE*remoteFn=remoteBase+(lp-(BYTE*)localOwner);
 SIZE_T bytes=(wslen(dllPath)+1)*2;
 LPVOID remoteStr=VirtualAllocEx(pi->hProcess,0,bytes,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
 if(!remoteStr)return 0;
 SIZE_T written=0;
 if(!WriteProcessMemory(pi->hProcess,remoteStr,dllPath,bytes,&written)||written!=bytes){VirtualFreeEx(pi->hProcess,remoteStr,0,MEM_RELEASE);return 0;}
 HANDLE t=CreateRemoteThread(pi->hProcess,0,0,(THREADPROC)remoteFn,remoteStr,0,0);
 if(!t){VirtualFreeEx(pi->hProcess,remoteStr,0,MEM_RELEASE);return 0;}
 DWORD wait=WaitForSingleObject(t,12000),rc=0;
 int ok=(wait==0 && GetExitCodeThread(t,&rc) && rc!=0);
 CloseHandle(t);VirtualFreeEx(pi->hProcess,remoteStr,0,MEM_RELEASE);
 return ok;
}

static void append_decimal(wchar16*dst,SIZE_T cap,DWORD v){
 char tmp[16];int k=0;do{tmp[k++]=(char)('0'+v%10);v/=10;}while(v&&k<15);
 wchar16 text[16];for(int j=0;j<k;j++)text[j]=(wchar16)tmp[k-1-j];text[k]=0;
 wappend(dst,cap,text);
}
static int backup_profile(void){
 int running=edge_running();
 if(running!=0){outw(TR(L"备份已取消：请先关闭全部 Edge 进程。\r\n",L"備份已取消：請先關閉所有 Edge 處理程序。\r\n",L"Backup cancelled: close all Edge processes first.\r\n"));return 1;}
 static wchar16 src[MAXW],dst[MAXW],robocopy[MAXW],command[4096];zero_mem(src,sizeof(src));zero_mem(dst,sizeof(dst));
 zero_mem(robocopy,sizeof(robocopy));zero_mem(command,sizeof(command));
 if(!path_env(L"LOCALAPPDATA",src,MAXW,L"\\Microsoft\\Edge\\User Data"))return 2;
 DWORD attr=GetFileAttributesW(src);
 if(attr==INVALID_FILE_ATTRIBUTES||!(attr&FILE_ATTRIBUTE_DIRECTORY)){
  outw(TR(L"没有找到原有 Edge 配置目录。\r\n",L"找不到原有 Edge 設定檔目錄。\r\n",L"Existing Edge profile directory not found.\r\n"));return 2;
 }
 if(!path_env(L"USERPROFILE",dst,MAXW,L"\\Desktop\\LegacyEdgeSync-EdgeBackup-x86-"))return 3;
 append_decimal(dst,MAXW,GetTickCount());wappend(dst,MAXW,L"-");append_decimal(dst,MAXW,GetCurrentProcessId());
 if(GetFileAttributesW(dst)!=INVALID_FILE_ATTRIBUTES){
  outw(TR(L"目标文件夹已存在，拒绝覆盖。\r\n",L"目標資料夾已存在，拒絕覆蓋。\r\n",L"Backup destination already exists; refusing overwrite.\r\n"));return 4;
 }
 if(!path_env(L"SystemRoot",robocopy,MAXW,L"\\System32\\robocopy.exe")||GetFileAttributesW(robocopy)==INVALID_FILE_ATTRIBUTES){
  outw(TR(L"找不到 Windows Robocopy。\r\n",L"找不到 Windows Robocopy。\r\n",L"Windows Robocopy executable not found.\r\n"));return 5;
 }
 outw(TR(L"\r\n原有配置：",L"\r\n原有設定檔：",L"\r\nSource profile: "));outw(src);outw(L"\r\n");
 outw(TR(L"备份目标：",L"備份目標：",L"Backup destination: "));outw(dst);outw(L"\r\n");
 outw(TR(L"正在使用 Robocopy 复制配置文件...\r\n",L"正在以 Robocopy 複製設定檔...\r\n",L"Copying with Robocopy...\r\n"));
 if(!wappend(command,4096,L"\"")||!wappend(command,4096,robocopy)||!wappend(command,4096,L"\" \"")||
    !wappend(command,4096,src)||!wappend(command,4096,L"\" \"")||!wappend(command,4096,dst)||
    !wappend(command,4096,L"\" /E /R:1 /W:1 /XJ /COPY:DAT"))return 6;
 STARTUPINFOW si;PROCESS_INFORMATION pi;zero_mem(&si,sizeof(si));zero_mem(&pi,sizeof(pi));si.cb=sizeof(si);
 if(!CreateProcessW(robocopy,command,0,0,FALSE,0,0,0,&si,&pi)){
  outw(TR(L"无法启动 Robocopy。\r\n",L"無法啟動 Robocopy。\r\n",L"Could not start Robocopy.\r\n"));return 7;
 }
 WaitForSingleObject(pi.hProcess,INFINITE);
 DWORD code=0xFFFFFFFF;GetExitCodeProcess(pi.hProcess,&code);
 CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
 if(code>=8){outw(TR(L"备份失败，Robocopy 退出代码：",L"備份失敗，Robocopy 結束代碼：",L"Backup failed; Robocopy exit code: "));outnum(code);outw(L"\r\n");return 8;}
 outw(TR(L"备份完成：",L"備份完成：",L"Backup completed: "));outw(dst);outw(L"\r\n");return 0;
}
static void guide(void){
 outw(TR(L"\r\n===== 使用与退出说明 =====\r\n",L"\r\n===== 使用與結束說明 =====\r\n",L"\r\n===== Usage and exit guidance =====\r\n"));
 outw(TR(L"1. 退出全部 Edge 窗口和后台进程即可卸载内存中的 Hook。\r\n",L"1. 關閉所有 Edge 視窗及背景處理程序，即可停止記憶體中的 Hook。\r\n",L"1. Close every Edge window and background process to stop the in-memory hook.\r\n"));
 outw(TR(L"2. 本工具不修改磁盘上的 msedge.dll，也不自动删除账户资料。\r\n",L"2. 本工具不修改磁碟上的 msedge.dll，也不會自動刪除帳戶資料。\r\n",L"2. Edge files on disk are unchanged; account data is never automatically deleted.\r\n"));
 outw(TR(L"3. 正常配置模式会读取原有 Edge 账户，测试前请先备份。\r\n",L"3. 原有設定檔模式會使用既有帳戶資料；測試前請先備份。\r\n",L"3. Existing-profile mode uses real account data; back it up before testing.\r\n"));
 outw(TR(L"4. 独立测试目录：%TEMP%\\LegacyEdgeSync-x86-TestProfile-test3\r\n",L"4. 獨立測試目錄：%TEMP%\\LegacyEdgeSync-x86-TestProfile-test3\r\n",L"4. Isolated profile: %TEMP%\\LegacyEdgeSync-x86-TestProfile-test3\r\n"));
 outw(TR(L"5. Hook DLL 保留成功的 v0.3-test2 版本，诊断状态会标注 test2。\r\n",L"5. Hook DLL 沿用成功的 v0.3-test2，診斷狀態仍會顯示 test2。\r\n",L"5. The proven v0.3-test2 Hook DLL is unchanged; its status still says test2.\r\n"));
 outw(TR(L"6. 不建议使用真实密码或支付数据测试同步。\r\n",L"6. 不建議使用真實密碼或付款資料測試同步。\r\n",L"6. Do not test password or payment-data sync.\r\n"));
}
static void press_enter(void){
 outw(TR(L"\r\n按 Enter 返回菜单...",L"\r\n按 Enter 返回選單...",L"\r\nPress Enter to return to the menu..."));
 char b[64];input_line(b,sizeof(b));outw(L"\r\n");
}
static DWORD run_self(LPCWSTR action){
 static wchar16 exe[MAXW],args[1500];zero_mem(exe,sizeof(exe));zero_mem(args,sizeof(args));
 if(!GetModuleFileNameW(0,exe,MAXW))return 0xFFFFFFFF;
 if(!wappend(args,1500,L"\"")||!wappend(args,1500,exe)||!wappend(args,1500,L"\" ")||
    !wappend(args,1500,action)||!wappend(args,1500,L" ")||!wappend(args,1500,langflag()))return 0xFFFFFFFF;
 STARTUPINFOW si;PROCESS_INFORMATION pi;zero_mem(&si,sizeof(si));zero_mem(&pi,sizeof(pi));si.cb=sizeof(si);
 if(!CreateProcessW(exe,args,0,0,FALSE,0,0,0,&si,&pi)){
  outw(TR(L"无法启动子操作。\r\n",L"無法啟動子操作。\r\n",L"Could not start requested operation.\r\n"));return 0xFFFFFFFF;
 }
 WaitForSingleObject(pi.hProcess,INFINITE);
 DWORD code=0xFFFFFFFF;GetExitCodeProcess(pi.hProcess,&code);
 CloseHandle(pi.hThread);CloseHandle(pi.hProcess);return code;
}
static void menu(void){
 for(;;){
  outw(L"\r\n====================================================\r\n");
  outw(L"LegacyEdgeSync v0.3-test3 / x86\r\n");
  outw(TR(L"32 位 Edge 109 云同步兼容实验版\r\n",L"32 位 Edge 109 雲端同步相容性實驗版\r\n",L"32-bit Edge 109 experimental sync compatibility tool\r\n"));
  outw(TR(L"界面语言：简体中文\r\n",L"介面語言：繁體中文\r\n",L"Interface language: English\r\n"));
  outw(L"====================================================\r\n");
  outw(TR(L"  [1] 原有 Edge 配置模式（需确认）\r\n",L"  [1] 原有 Edge 設定檔模式（須確認）\r\n",L"  [1] Existing Edge profile (confirmation required)\r\n"));
  outw(TR(L"  [2] 独立测试配置（建议先使用）\r\n",L"  [2] 獨立測試設定檔（建議先使用）\r\n",L"  [2] Isolated test profile (recommended first)\r\n"));
  outw(TR(L"  [3] 查看 Hook 状态\r\n",L"  [3] 查看 Hook 狀態\r\n",L"  [3] View hook status\r\n"));
  outw(TR(L"  [4] 备份原有 Edge 配置\r\n",L"  [4] 備份原有 Edge 設定檔\r\n",L"  [4] Back up existing profile\r\n"));
  outw(TR(L"  [5] 使用与退出说明\r\n",L"  [5] 使用與結束說明\r\n",L"  [5] Usage and exit guidance\r\n"));
  outw(TR(L"  [0] 退出\r\n",L"  [0] 結束\r\n",L"  [0] Exit\r\n"));
  outw(TR(L"\r\n请选择：",L"\r\n請選擇：",L"\r\nChoose: "));
  char b[64];input_line(b,sizeof(b));
  if(b[0]=='0'||b[0]==0)return;
  else if(b[0]=='1')run_self(L"--normal");
  else if(b[0]=='2')run_self(L"--test");
  else if(b[0]=='3')run_self(L"--status");
  else if(b[0]=='4')run_self(L"--backup");
  else if(b[0]=='5')run_self(L"--guide");
  else{outw(TR(L"未知选项。\r\n",L"未知選項。\r\n",L"Invalid option.\r\n"));continue;}
  press_enter();
 }
}
void WINAPI mainCRTStartup(void){
 LPCWSTR cmd=GetCommandLineW();detect_language();language_override(cmd);
 outw(TR(L"LegacyEdgeSync v0.3-test3 x86 | 云同步兼容实验版\r\n",L"LegacyEdgeSync v0.3-test3 x86 | 雲端同步相容性實驗版\r\n",L"LegacyEdgeSync v0.3-test3 x86 | Experimental sync fix\r\n"));
 if(flag(cmd,L"--help")||flag(cmd,L"-h")){
  outw(TR(L"用法：LegacyEdgeSync32.exe [--menu | --test | --normal | --status | --backup | --guide]\r\n",L"用法：LegacyEdgeSync32.exe [--menu | --test | --normal | --status | --backup | --guide]\r\n",L"Usage: LegacyEdgeSync32.exe [--menu | --test | --normal | --status | --backup | --guide]\r\n"));
  outw(TR(L"语言参数：--lang=zh-CN / --lang=zh-TW / --lang=en\r\n",L"語言參數：--lang=zh-CN / --lang=zh-TW / --lang=en\r\n",L"Language override: --lang=zh-CN / --lang=zh-TW / --lang=en\r\n"));
  outw(TR(L"无参数默认独立测试模式。\r\n",L"無參數預設獨立測試模式。\r\n",L"No args defaults to isolated test mode.\r\n"));ExitProcess(0);
 }
 if(flag(cmd,L"--menu")){menu();ExitProcess(0);}
 if(flag(cmd,L"--guide")){guide();ExitProcess(0);}
 if(flag(cmd,L"--status")){show_status();ExitProcess(0);}
 if(flag(cmd,L"--backup")){ExitProcess((DWORD)backup_profile());}
 if(flag(cmd,L"--test")&&flag(cmd,L"--normal")){
  outw(TR(L"不能同时指定 --test 和 --normal。\r\n",L"不可同時指定 --test 與 --normal。\r\n",L"Cannot combine --test and --normal.\r\n"));ExitProcess(2);
 }
 normalMode=flag(cmd,L"--normal")?1:0;
 if(!set_paths()){
  outw(TR(L"找不到 Edge x86 主程序或 LegacyEdgeSyncHook32.dll。\r\n",L"找不到 Edge x86 主程式或 LegacyEdgeSyncHook32.dll。\r\n",L"Edge x86 or LegacyEdgeSyncHook32.dll missing.\r\n"));ExitProcess(3);
 }
 int running=edge_running();
 if(running!=0){outw(TR(L"Edge 已在运行或无法检查进程，请先退出全部 Edge。\r\n",L"Edge 已在執行或無法檢查處理程序，請先結束所有 Edge。\r\n",L"Edge is running or process check failed. Exit all Edge processes.\r\n"));ExitProcess(4);}
 if(normalMode){if(!confirm_normal()){outw(TR(L"已取消原有配置模式。\r\n",L"已取消原有設定檔模式。\r\n",L"Existing-profile mode cancelled.\r\n"));ExitProcess(5);}}
 else outw(TR(L"当前：独立测试配置（不会直接读取现有 Edge 配置）。\r\n",L"目前：獨立測試設定檔（不會直接讀取原有 Edge 設定檔）。\r\n",L"Mode: isolated test profile. Existing Edge profile is not loaded.\r\n"));
 DeleteFileW(statusPath);
 STARTUPINFOW si;PROCESS_INFORMATION pi;zero_mem(&si,sizeof(si));zero_mem(&pi,sizeof(pi));si.cb=sizeof(si);
 if(!CreateProcessW(edgeExe,cmdline,0,0,FALSE,0,0,0,&si,&pi)){
  outw(TR(L"Edge 无法启动，Win32 错误码：",L"Edge 無法啟動，Win32 錯誤碼：",L"Edge failed to start, Win32 error: "));outnum(GetLastError());outw(L"\r\n");ExitProcess(6);
 }
 outw(TR(L"Edge 已启动，PID：",L"Edge 已啟動，PID：",L"Edge started, PID: "));outnum(pi.dwProcessId);outw(L"\r\n");
 int done=0;
 for(int i=0;i<160;i++){
  if(remote_inject(&pi)){done=1;break;}
  DWORD rc=0;if(GetExitCodeProcess(pi.hProcess,&rc)&&rc!=STILL_ACTIVE)break;
  Sleep(25);
 }
 if(!done){outw(TR(L"未能注入 x86 Hook DLL；浏览器可能仍在运行但未修复。\r\n",L"無法注入 x86 Hook DLL；瀏覽器可能仍在執行但未修復。\r\n",L"Failed to inject x86 Hook DLL. Browser may remain open without patch.\r\n"));CloseHandle(pi.hThread);CloseHandle(pi.hProcess);ExitProcess(7);}
 outw(TR(L"DLL 已注入，正在等待 Hook 状态...\r\n",L"DLL 已注入，正在等待 Hook 狀態...\r\n",L"DLL injected. Waiting for hook status...\r\n"));
 int hook=0;char b[1536];
 for(int i=0;i<40;i++){
  zero_mem(b,sizeof(b));if(read_status(b,sizeof(b))){
   if(contains(b,"FAILED:")){hook=-1;break;}
   if(contains(b,"INSTALLED:")){hook=1;break;}
  }
  Sleep(500);
 }
 show_status();
 if(hook==1)outw(TR(L"Hook 已安装，但还需确认实际同步成功。\r\n",L"Hook 已安裝，但仍須確認實際同步成功。\r\n",L"Hook reports installed; actual sync must still be verified.\r\n"));
 else if(hook==-1)outw(TR(L"Hook 报告失败，请先关闭浏览器排查。\r\n",L"Hook 回報失敗，請先關閉瀏覽器調查。\r\n",L"Hook reports failure. Close Edge and investigate.\r\n"));
 else outw(TR(L"等待 Hook 状态超时，结果不确定。\r\n",L"等待 Hook 狀態逾時，結果未定。\r\n",L"Timed out waiting for hook status; state unknown.\r\n"));
 CloseHandle(pi.hThread);CloseHandle(pi.hProcess);ExitProcess(hook==1?0:(hook==-1?8:9));
}
