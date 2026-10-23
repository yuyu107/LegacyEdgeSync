#include "unified_winmini.h"
/*
 * LegacyEdgeSync v0.3-test7 universal dispatcher, Win7 SP1 x86/x64.
 * This program does NOT inject, patch, or touch browser processes.
 * It only reads installed Edge executable/DLL headers, confirms a specific
 * Edge version using Windows version.dll, and starts a matched, unchanged
 * v0.3-test3 x86 or v0.2-test3 x64 sub-launcher.
 * x64 hook file is the Win7/Win8.1 dual-offset v0.3-test6 validated binary.
 *
 * The existing child launchers MUST retain their original install-path policy:
 * they select ProgramFiles(x86) if it exists, otherwise ProgramFiles.
 * This selector deliberately uses exactly that same priority and refuses
 * to silently use a different Edge install elsewhere on disk.
 */
#define MAXF 1024
#define CMDSZ 2400
#define EDGE_VER L"109.0.1518.140"
#define IMAGE_FILE_MACHINE_I386 0x014C
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define VERSION_SIGNATURE 0xFEEF04BDU
#define PE32_MAGIC 0x010BU
#define PE64_MAGIC 0x020BU

/* dynamic Win7 Version APIs */
typedef DWORD (WINAPI *PFN_VERSION_SIZE)(LPCWSTR,DWORD*);
typedef BOOL  (WINAPI *PFN_VERSION_GET)(LPCWSTR,DWORD,DWORD,LPVOID);
typedef BOOL  (WINAPI *PFN_VERSION_QUERY)(LPCVOID,LPCWSTR,LPVOID*,DWORD*);
typedef BOOL  (WINAPI *PFN_IS_WOW64)(HANDLE,BOOL*);
typedef struct _FIXED_VERSION {
 DWORD signature,structVersion,fileMS,fileLS,productMS,productLS;
 DWORD flagsMask,flags,fileOS,fileType,fileSubtype,fileDateMS,fileDateLS;
} FIXED_VERSION;

enum { L_CN=0,L_TW=1,L_EN=2 };
static int lang=L_EN;
static wchar16 folder[MAXF],edgeExe[MAXF],edgeDll[MAXF],child[MAXF];
static BYTE first_bytes[4096],version_bytes[65536];
static char utf8_out[8192];
static wchar16 child_cmd[CMDSZ],child_hook[MAXF];
static int diagnostics=0;

static LPCWSTR tr(LPCWSTR cn,LPCWSTR tw,LPCWSTR en){return lang==L_CN?cn:(lang==L_TW?tw:en);}
static void printw(LPCWSTR s){
 HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
 if(!h||(ULONG_PTR)h==(ULONG_PTR)-1||!s)return;
 DWORD done=0;
 if(WriteConsoleW(h,s,(DWORD)wslen(s),&done,0))return;
 int n=WideCharToMultiByte(CP_UTF8,0,s,-1,utf8_out,sizeof(utf8_out),0,0);
 if(n>1)WriteFile(h,utf8_out,(DWORD)(n-1),&done,0);
}
static void print_num(DWORD n){outnum(n);}
static int append(wchar16*b,SIZE_T cap,LPCWSTR x){return wappend(b,cap,x);}
static void copyw(wchar16*d,SIZE_T cap,LPCWSTR x){zero_mem(d,cap*2);append(d,cap,x);}
static int starts(LPCWSTR p,LPCWSTR q){while(*q){if(!*p||*p++!=*q++)return 0;}return 1;}
static void language_init(void){
 WORD id=GetUserDefaultUILanguage();if(!id)id=GetSystemDefaultUILanguage();
 if((id&0x3ff)!=0x04){lang=L_EN;return;}
 WORD sub=(WORD)(id>>10);
 lang=(sub==1||sub==3||sub==5||sub==0x1f)?L_TW:L_CN;
}
static void language_override(LPCWSTR cmd){
 if(!cmd)return;
 for(LPCWSTR p=cmd;*p;p++){
  if(p!=cmd&&p[-1]!=L' '&&p[-1]!=L'\t')continue;
  if(starts(p,L"--lang=zh-CN")&&(p[12]==0||p[12]==L' '||p[12]==L'\t')){lang=L_CN;return;}
  if(starts(p,L"--lang=zh-TW")&&(p[12]==0||p[12]==L' '||p[12]==L'\t')){lang=L_TW;return;}
  if(starts(p,L"--lang=en")&&(p[9]==0||p[9]==L' '||p[9]==L'\t')){lang=L_EN;return;}
 }
}
static LPCWSTR lang_flag(void){return lang==L_CN?L"--lang=zh-CN":(lang==L_TW?L"--lang=zh-TW":L"--lang=en");}
static int has_flag(LPCWSTR cmd,LPCWSTR sw){
 SIZE_T n=wslen(sw);
 for(SIZE_T i=0;cmd&&cmd[i];i++){
  if(i&&cmd[i-1]!=L' '&&cmd[i-1]!=L'\t')continue;
  SIZE_T j=0;while(j<n&&cmd[i+j]==sw[j])j++;
  if(j==n&&(cmd[i+j]==0||cmd[i+j]==L' '||cmd[i+j]==L'\t'))return 1;
 }return 0;
}
static int file_exists(LPCWSTR path){DWORD attr=GetFileAttributesW(path);return attr!=INVALID_FILE_ATTRIBUTES&&!(attr&FILE_ATTRIBUTE_DIRECTORY);}
static int envpath(LPCWSTR env,wchar16*out,SIZE_T cap,LPCWSTR sub){
 zero_mem(out,cap*2);DWORD n=GetEnvironmentVariableW(env,out,(DWORD)cap);
 return n>0&&n<cap&&append(out,cap,sub);
}
static int find_edge(void){
 /* Match existing x64 startup precedence exactly. */
 wchar16 p[MAXF];
 /* On Win7 x64, the legacy x64 helper uses ProgramFiles(x86) whenever
  * that variable exists, even when the file is missing. Mirror that exactly.
  * On Win7 x86, ProgramFiles(x86) normally does not exist.
  */
 if(envpath(L"ProgramFiles(x86)",p,MAXF,L"\\Microsoft\\Edge\\Application\\msedge.exe")){
  if(!file_exists(p))return 0;
  copyw(edgeExe,MAXF,p);
 }else if(envpath(L"ProgramFiles",p,MAXF,L"\\Microsoft\\Edge\\Application\\msedge.exe")&&file_exists(p)){
  copyw(edgeExe,MAXF,p);
 }else return 0;
 /* Core DLL always lives under a versioned directory, never the root. */
 copyw(edgeDll,MAXF,edgeExe);
 SIZE_T n=wslen(edgeDll);
 while(n&&edgeDll[n-1]!=L'\\'&&edgeDll[n-1]!=L'/')n--;
 edgeDll[n]=0;
 if(!append(edgeDll,MAXF,EDGE_VER)||!append(edgeDll,MAXF,L"\\msedge.dll"))return 0;
 return 1;
}
static DWORD u32le(const BYTE* p){return (DWORD)p[0]|((DWORD)p[1]<<8)|((DWORD)p[2]<<16)|((DWORD)p[3]<<24);}
static WORD u16le(const BYTE* p){return (WORD)((WORD)p[0]|((WORD)p[1]<<8));}
static WORD machine(LPCWSTR path){
 HANDLE h=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
 if(!h||(ULONG_PTR)h==(ULONG_PTR)-1)return 0;
 DWORD count=0;int ok=ReadFile(h,first_bytes,sizeof(first_bytes),&count,0);CloseHandle(h);
 if(!ok||count<256||first_bytes[0]!='M'||first_bytes[1]!='Z')return 0;
 DWORD off=u32le(first_bytes+0x3c);
 if(off<0x40||off>count-26)return 0;
 if(first_bytes[off]!='P'||first_bytes[off+1]!='E'||first_bytes[off+2]!=0||first_bytes[off+3]!=0)return 0;
 WORD peMachine=u16le(first_bytes+off+4);
 WORD magic=u16le(first_bytes+off+24);
 if((peMachine==IMAGE_FILE_MACHINE_I386&&magic==PE32_MAGIC)||
    (peMachine==IMAGE_FILE_MACHINE_AMD64&&magic==PE64_MAGIC))return peMachine;
 return 0;
}
/* Verify *actual* file version resource, not only a self-declared folder name. */
static int exact_version(LPCWSTR path){
 wchar16 system[MAXF];zero_mem(system,sizeof(system));
 DWORD n=GetSystemDirectoryW(system,MAXF);
 if(!n||n>=MAXF||!append(system,MAXF,L"\\version.dll"))return 0;
 HMODULE lib=LoadLibraryW(system);
 if(!lib)return 0;
 PFN_VERSION_SIZE getsize=(PFN_VERSION_SIZE)GetProcAddress(lib,"GetFileVersionInfoSizeW");
 PFN_VERSION_GET get=(PFN_VERSION_GET)GetProcAddress(lib,"GetFileVersionInfoW");
 PFN_VERSION_QUERY query=(PFN_VERSION_QUERY)GetProcAddress(lib,"VerQueryValueW");
 if(!getsize||!get||!query)return 0;
 DWORD unused=0;DWORD sz=getsize(path,&unused);
 if(!sz||sz>sizeof(version_bytes))return 0;
 if(!get(path,0,sz,version_bytes))return 0;
 LPVOID ptr=0;DWORD cb=0;
 if(!query(version_bytes,L"\\",&ptr,&cb)||cb<sizeof(FIXED_VERSION)||!ptr)return 0;
 const FIXED_VERSION*v=(const FIXED_VERSION*)ptr;
 if(v->signature!=VERSION_SIGNATURE)return 0;
 DWORD maj=(v->fileMS>>16),min=(v->fileMS&0xffff),build=(v->fileLS>>16),rev=(v->fileLS&0xffff);
 return maj==109 && min==0 && build==1518 && rev==140;
}
static int native_x64(void){
 HMODULE k=GetModuleHandleW(L"kernel32.dll");
 if(!k)return 0;
 PFN_IS_WOW64 wow=(PFN_IS_WOW64)GetProcAddress(k,"IsWow64Process");
 if(!wow)return 0;
 BOOL isWow=0;
 return wow(GetCurrentProcess(),&isWow)&&isWow;
}
static void status_arch(WORD m){
 if(m==IMAGE_FILE_MACHINE_I386)printw(L"x86 (PE32 / I386)\r\n");
 else if(m==IMAGE_FILE_MACHINE_AMD64)printw(L"x64 (PE32+ / AMD64)\r\n");
 else printw(L"Unknown / unreadable\r\n");
}
static void diagnosis(WORD ex,WORD dll,int verExe,int verDll){
 printw(tr(L"\r\n===== 检测结果 =====\r\n",L"\r\n===== 偵測結果 =====\r\n",L"\r\n===== Detection result =====\r\n"));
 printw(tr(L"Edge 路径：",L"Edge 路徑：",L"Edge path: "));printw(edgeExe);printw(L"\r\n");
 printw(tr(L"核心 DLL：",L"核心 DLL：",L"Core DLL: "));printw(edgeDll);printw(L"\r\n");
 printw(tr(L"EXE 架构：",L"EXE 架構：",L"EXE architecture: "));status_arch(ex);
 printw(tr(L"DLL 架构：",L"DLL 架構：",L"DLL architecture: "));status_arch(dll);
 printw(tr(L"EXE 文件版本：",L"EXE 檔案版本：",L"EXE file version: "));printw(verExe?L"109.0.1518.140\r\n":L"NOT VERIFIED\r\n");
 printw(tr(L"DLL 文件版本：",L"DLL 檔案版本：",L"DLL file version: "));printw(verDll?L"109.0.1518.140\r\n":L"NOT VERIFIED\r\n");
 printw(tr(L"检测到 64 位系统：",L"偵測到 64 位系統：",L"64-bit OS detected: "));printw(native_x64()?L"yes\r\n":L"no\r\n");
}
static void warn_security(void){
 printw(tr(L"提示：该工具会启动含注入/Hook 功能的实验修复器；安全软件可能告警。\r\n",
 L"提示：此工具將啟動含注入/Hook 功能的實驗修復器；安全軟體可能警告。\r\n",
 L"Notice: The experimental child patch uses DLL injection and hooks; antivirus alerts are possible.\r\n"));
 printw(tr(L"360 曾将部分 EXE 检测为 HEUR/QVM。尚未经过安全厂商复核，请勿关闭防护强行运行。\r\n",
 L"360 曾將部分 EXE 偵測為 HEUR/QVM。尚未經安全廠商複核，請勿停用防護強行執行。\r\n",
 L"360 has flagged some EXEs as HEUR/QVM. Detection is unreviewed; do not disable security controls.\r\n"));
}
/* Assemble exact component paths without executing any browser or patch code. */
static int component_paths(WORD arch){
 wchar16 local[MAXF];zero_mem(local,sizeof(local));
 DWORD n=GetModuleFileNameW(0,local,MAXF);
 if(!n||n>=MAXF)return 0;
 SIZE_T i=wslen(local);while(i&&local[i-1]!=L'\\'&&local[i-1]!=L'/')i--;
 local[i]=0;
 copyw(folder,MAXF,local);
 if(!append(folder,MAXF,arch==IMAGE_FILE_MACHINE_I386?L"Win7-x86":L"Win7-x64"))return 0;
 copyw(child,MAXF,folder);
 if(!append(child,MAXF,arch==IMAGE_FILE_MACHINE_I386?L"\\LegacyEdgeSync32.exe":L"\\Edge109SyncFix.exe"))return 0;
 copyw(child_hook,MAXF,folder);
 if(!append(child_hook,MAXF,arch==IMAGE_FILE_MACHINE_I386?L"\\LegacyEdgeSyncHook32.dll":L"\\Edge109SyncHook.dll"))return 0;
 return 1;
}
/* One integrity/architecture check shared by dispatch and diagnosis.
 * No injection and no bypass of security software is attempted here. */
static int components_ok(WORD arch,int verbose){
 if(arch!=IMAGE_FILE_MACHINE_I386&&arch!=IMAGE_FILE_MACHINE_AMD64)return 0;
 if(!component_paths(arch))return 0;
 int exist_exe=file_exists(child),exist_hook=file_exists(child_hook);
 WORD child_arch=exist_exe?machine(child):0;
 WORD hook_arch=exist_hook?machine(child_hook):0;
 if(verbose){
  printw(tr(L"修复器：",L"修復器：",L"Launcher: "));printw(child);printw(L"\r\n");
  printw(tr(L"Hook DLL：",L"Hook DLL：",L"Hook DLL: "));printw(child_hook);printw(L"\r\n");
 }
 if(!exist_exe||!exist_hook){
  if(verbose)printw(tr(L"错误：缺少配套 EXE/DLL。请完整解压。\r\n",L"錯誤：缺少配套 EXE/DLL。請完整解壓。\r\n",L"Error: missing EXE/DLL; extract the full ZIP.\r\n"));
  return 0;
 }
 if(child_arch!=arch||hook_arch!=arch){
  if(verbose)printw(tr(L"错误：EXE/DLL PE 架构不匹配。\r\n",L"錯誤：EXE/DLL PE 架構不相符。\r\n",L"Error: EXE/DLL PE machine mismatch.\r\n"));
  return 0;
 }
 if(verbose)printw(tr(L"对应组件架构检查：通过。\r\n",L"對應元件架構檢查：通過。\r\n",L"Matched component architectures: OK.\r\n"));
 return 1;
}
static void check_bundle(void){
 printw(tr(L"\r\n===== 安装包组件检测（不启动 Edge）=====\r\n",L"\r\n===== 套件元件檢查（不啟動 Edge）=====\r\n",L"\r\n===== Bundle check (no Edge launch) =====\r\n"));
 printw(L"\r\n[x86]\r\n");int a=components_ok(IMAGE_FILE_MACHINE_I386,1);
 printw(L"\r\n[x64]\r\n");int b=components_ok(IMAGE_FILE_MACHINE_AMD64,1);
 if(a&&b){
  printw(tr(L"\r\n双架构文件检查通过。该结果不代表 Hook 或同步已在当前系统通过测试。\r\n",L"\r\n雙架構檔案檢查通過。這不代表 Hook 或同步已在目前系統通過測試。\r\n",L"\r\nBoth PE pairs checked. This is not a functional sync test.\r\n"));
  ExitProcess(0);
 }
 printw(tr(L"\r\n组件文件不完整或架构不匹配，已停止。\r\n",L"\r\n元件檔案不完整或架構不符，已停止。\r\n",L"\r\nPackage is incomplete/mismatched.\r\n"));
 ExitProcess(6);
}
static int delegate(WORD arch,LPCWSTR mode){
 if(!components_ok(arch,1))return 0;
 zero_mem(child_cmd,sizeof(child_cmd));
 if(!append(child_cmd,CMDSZ,L"\"")||!append(child_cmd,CMDSZ,child)||!append(child_cmd,CMDSZ,L"\" ")||
    !append(child_cmd,CMDSZ,mode)||!append(child_cmd,CMDSZ,L" ")||!append(child_cmd,CMDSZ,lang_flag()))return 0;
 printw(tr(L"正在启动所选架构的修复器：",L"正在啟動選取架構的修復器：",L"Launching matched child launcher: "));
 printw(child);printw(L"\r\n");
 STARTUPINFOW si;PROCESS_INFORMATION pi;zero_mem(&si,sizeof(si));zero_mem(&pi,sizeof(pi));si.cb=sizeof(si);
 if(!CreateProcessW(child,child_cmd,0,0,FALSE,0,0,folder,&si,&pi)){
  printw(tr(L"错误：无法启动子程序。Windows 错误代码：",L"錯誤：無法啟動子程式。Windows 錯誤碼：",L"Error: Could not launch child. Windows error: "));
  print_num(GetLastError());printw(L"\r\n");return 0;
 }
 WaitForSingleObject(pi.hProcess,INFINITE);
 DWORD rc=0;GetExitCodeProcess(pi.hProcess,&rc);
 CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
 printw(tr(L"子程序已退出，返回代码：",L"子程式已結束，傳回碼：",L"Child process exited, code: "));print_num(rc);printw(L"\r\n");
 return (int)(rc==0?1:2);
}
void WINAPI mainCRTStartup(void){
 language_init();LPCWSTR args=GetCommandLineW();language_override(args);
 printw(L"LegacyEdgeSync v0.3-test7 | Win7/8.1 Edge x86/x64 selector (x86 executable)\r\n");
 if(has_flag(args,L"--help")||has_flag(args,L"-h")){
  printw(L"--menu (default), --test, --normal, --status, --backup, --guide, --diagnose, --bundle-check\r\n");
  printw(L"--lang=zh-CN | --lang=zh-TW | --lang=en\r\n");ExitProcess(0);
 }
 if(has_flag(args,L"--bundle-check"))check_bundle();
 diagnostics=has_flag(args,L"--diagnose");
 if(!find_edge()){
  printw(tr(L"未在标准安装位置找到 Edge，请确认已安装 Edge 109。\r\n",
   L"未於標準安裝位置找到 Edge，請確認已安裝 Edge 109。\r\n",
   L"Edge not found in standard system installation path. Install Edge 109 first.\r\n"));
  ExitProcess(2);
 }
 WORD exe_arch=machine(edgeExe),dll_arch=machine(edgeDll);
 int exe_version=exact_version(edgeExe),dll_version=exact_version(edgeDll);
 if(diagnostics)diagnosis(exe_arch,dll_arch,exe_version,dll_version);
 if(!exe_arch||exe_arch!=dll_arch){
  printw(tr(L"错误：无法读取 PE 架构，或 Edge EXE 与 DLL 架构不一致。\r\n",
   L"錯誤：無法讀取 PE 架構，或 Edge EXE 與 DLL 架構不相符。\r\n",
   L"Error: Invalid PE architecture or Edge EXE/DLL mismatch.\r\n"));ExitProcess(3);
 }
 if(exe_arch!=IMAGE_FILE_MACHINE_I386&&exe_arch!=IMAGE_FILE_MACHINE_AMD64){
  printw(L"Unsupported machine type.\r\n");ExitProcess(3);
 }
 if(!exe_version||!dll_version){
  printw(tr(L"错误：Edge 文件版本不等于 109.0.1518.140（或版本信息无法验证）。\r\n",
   L"錯誤：Edge 檔案版本不是 109.0.1518.140（或版本資訊無法驗證）。\r\n",
   L"Error: Edge file version is not exactly 109.0.1518.140 (or cannot be checked).\r\n"));
  printw(tr(L"不会使用其他 Edge 版本进行内存修补。\r\n",
   L"不會對其他 Edge 版本進行記憶體修補。\r\n",
   L"Refusing to patch another Edge build.\r\n"));ExitProcess(4);
 }
 if(exe_arch==IMAGE_FILE_MACHINE_AMD64&&!native_x64()){
  printw(tr(L"错误：当前系统无法执行 64 位 Edge。\r\n",L"錯誤：目前系統無法執行 64 位 Edge。\r\n",L"Error: OS cannot run x64 Edge.\r\n"));ExitProcess(5);
 }
 printw(tr(L"已检测 Edge 架构：",L"已偵測 Edge 架構：",L"Detected Edge architecture: "));status_arch(exe_arch);
 if(!diagnostics){
   if(exe_arch==IMAGE_FILE_MACHINE_AMD64)printw(tr(L"x64 包含 Win7 (0x49) 与 Win8.1 (0x45) 已验证 Hook 路径；请检查实际 patched 计数。\r\n",L"x64 包含 Win7 (0x49) 與 Win8.1 (0x45) 已驗證 Hook 路徑；請查看 patched 計數。\r\n",L"x64 Hook has Win7 (0x49) and Win8.1 (0x45) tested match paths; check live patched count.\r\n"));
   warn_security();
 }
 LPCWSTR mode=L"--menu";
 if(has_flag(args,L"--test"))mode=L"--test";
 else if(has_flag(args,L"--normal"))mode=L"--normal";
 else if(has_flag(args,L"--status"))mode=L"--status";
 else if(has_flag(args,L"--backup"))mode=L"--backup";
 else if(has_flag(args,L"--guide"))mode=L"--guide";
 if(diagnostics){
  printw(tr(L"\r\n匹配子组件检测：\r\n",L"\r\n相符子元件檢查：\r\n",L"\r\nMatching package components:\r\n"));
  int ok=components_ok(exe_arch,1);
  printw(tr(L"检测结束；未启动 Edge 或注入模块。\r\n",L"偵測完成；未啟動 Edge 或注入模組。\r\n",L"Diagnostic only; no browser or patch was started.\r\n"));
  ExitProcess(ok?0:6);
 }
 int result=delegate(exe_arch,mode);
 ExitProcess(result==1?0:(result==2?20:10));
}
