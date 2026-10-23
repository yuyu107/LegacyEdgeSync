#include "winmini.h"

// Unverified Windows 7 x64 experiment. No filesystem changes to Edge binaries.
// The A site is specifically Edge 109.0.1518.140 x64, discovered by the user's analysis.
static volatile long gHits=0;
static volatile long gPatched=0;
static volatile long gSkipped=0;
static volatile long gInstalled=0;
static const char gBefore[14] = "109.0.1518.140";
static const char gAfter[14] = "133.0.3065.120";
static const BYTE gOriginalA[7] = {0x4C,0x8D,0x05,0x94,0x45,0x63,0x04};
static wchar_t gStatusFile[1024];
static BYTE* gTarget=0;
static BYTE* gRelay=0;

// Called inside relay with shadow space and XMM0..XMM5 preserved.
__declspec(noinline) static void PatchHeader(char* base){
    ++gHits;
    if(!base){++gSkipped;return;}
    // Avoid touching unexpected memory. Check both marker and old version.
    MEMORY_BASIC_INFORMATION mi;
    BYTE* check=(BYTE*)base+0x49;
    if(VirtualQuery(check,&mi,sizeof(mi))!=sizeof(mi) || mi.State!=MEM_COMMIT || (mi.Protect&(PAGE_GUARD|PAGE_NOACCESS)) ||
       !((mi.Protect&0xFF)==PAGE_READWRITE || (mi.Protect&0xFF)==PAGE_EXECUTE_READWRITE ||
         (mi.Protect&0xFF)==PAGE_WRITECOPY || (mi.Protect&0xFF)==PAGE_EXECUTE_WRITECOPY) ||
       (ULONG_PTR)mi.lpBaseAddress+mi.RegionSize < (ULONG_PTR)(check+7+14)){
       ++gSkipped;return;
    }
    if(!abytes_eq(check,"appVer=",7) || !abytes_eq(check+7,gBefore,14)){
       ++gSkipped;return;
    }
    acopy(check+7,gAfter,14);
    ++gPatched;
}

static void write_status(const char*msg){
   if(!gStatusFile[0])return;
   HANDLE h=CreateFileW(gStatusFile,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
   if(h==(HANDLE)(-1)||!h)return;
   DWORD wrote=0;
   WriteFile(h,msg,(DWORD)slen(msg),&wrote,0);
   const char* fields="\r\nEdge109 Sync Hook v0.1-test3 (experimental)\r\nTarget Edge version: 109.0.1518.140 x64\r\n";
   WriteFile(h,fields,(DWORD)slen(fields),&wrote,0);
   char num[16];
   u32toa(GetCurrentProcessId(),num);WriteFile(h,"pid=",4,&wrote,0);WriteFile(h,num,(DWORD)slen(num),&wrote,0);
   u32toa((DWORD)gHits,num);WriteFile(h,"\r\nhits=",7,&wrote,0);WriteFile(h,num,(DWORD)slen(num),&wrote,0);
   u32toa((DWORD)gPatched,num);WriteFile(h,"\r\npatched=",10,&wrote,0);WriteFile(h,num,(DWORD)slen(num),&wrote,0);
   u32toa((DWORD)gSkipped,num);WriteFile(h,"\r\nskipped=",10,&wrote,0);WriteFile(h,num,(DWORD)slen(num),&wrote,0);
   const char*safe="\r\nNo account data or tokens are recorded.\r\n";WriteFile(h,safe,(DWORD)slen(safe),&wrote,0);
   CloseHandle(h);
}
static int validate_module(HMODULE mod){
   BYTE*b=(BYTE*)mod;
   if(b[0]!='M'||b[1]!='Z')return 0;
   DWORD peoff=*(DWORD*)(b+0x3C);
   if(peoff<0x40||peoff>0x400)return 0;
   BYTE*pe=b+peoff;
   if(pe[0]!='P'||pe[1]!='E'||pe[2]!=0||pe[3]!=0)return 0;
   if(*(WORD*)(pe+4)!=0x8664 || *(WORD*)(pe+24)!=0x20b)return 0;
   DWORD imageSize=*(DWORD*)(pe+24+56);
   if(imageSize<=0xD128330 || imageSize>0x20000000)return 0;
   wchar_t path[1024];zero_mem(path,sizeof(path));
   if(!GetModuleFileNameW(mod,path,1024))return 0;
   const wchar_t*version=L"109.0.1518.140";
   // Require version folder before loading patch; simple substring compare.
   int found=0;for(SIZE_T i=0;path[i];i++){SIZE_T j=0;while(version[j]&&path[i+j]==version[j])j++;if(!version[j]){found=1;break;}}
   if(!found)return 0;
   BYTE*site=b+0x8AF3D95;
   if(!abytes_eq(site,gOriginalA,7))return 0;
   if(!abytes_eq(b+0xD128330,"X-AFS-ClientInfo",16))return 0;
   int disp=*(int*)(site+3);
   if(site+7+(LONGLONG)disp!=b+0xD128330)return 0;
   return 1;
}
static BYTE*alloc_near(BYTE*site){
  ULONG_PTR b=(ULONG_PTR)site & ~(ULONG_PTR)0xFFFF;
  for(ULONG_PTR delta=0x10000;delta<=0x70000000;delta+=0x10000){
    for(int direction=0;direction<2;direction++){
      ULONG_PTR p= direction ? b+delta : (b>=delta ? b-delta:0);
      if(!p || p>0x7FFFFFFF0000ULL)continue;
      if((LONGLONG)p-(LONGLONG)((ULONG_PTR)site+5) > 0x7FFFFFFFLL ||
         (LONGLONG)p-(LONGLONG)((ULONG_PTR)site+5) < -0x80000000LL)continue;
      void*q=VirtualAlloc((LPVOID)p,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
      if(q)return (BYTE*)q;
    }
  }
  return 0;
}
static int relay_emit(BYTE*outp,BYTE*target,BYTE*name){
    BYTE*p=outp;
    #define E1(x) (*p++=(BYTE)(x))
    #define EB(x) do{const BYTE*z=(const BYTE*)(x);for(SIZE_T i=0;i<sizeof(x)-1;i++)E1(z[i]);}while(0)
    // preserve flags, all volatile GPRs, XMM0..5 (x64 Windows ABI)
    E1(0x9C);E1(0x50);E1(0x51);E1(0x52);
    E1(0x41);E1(0x50);E1(0x41);E1(0x51);E1(0x41);E1(0x52);E1(0x41);E1(0x53);
    // At the verified A site, RSP modulo 16 is 8. Eight pushes preserve that; 0x88 aligns before call.
    // 0x88 allocation reserves aligned 32-byte shadow space plus six 16-byte XMM saves and 8-byte pad.
    BYTE subrsp[]={0x48,0x81,0xEC,0x88,0x00,0x00,0x00};for(int i=0;i<7;i++)E1(subrsp[i]);
    for(int i=0;i<6;i++){E1(0xF3);E1(0x0F);E1(0x7F);E1((BYTE)(0x44+i*8));E1(0x24);E1((BYTE)(0x20+i*16));}
    // mov rcx,r14; mov rax,PatchHeader; call rax
    E1(0x4C);E1(0x89);E1(0xF1);E1(0x48);E1(0xB8);
    ULONGLONG fn=(ULONGLONG)(ULONG_PTR)&PatchHeader;acopy(p,&fn,8);p+=8;E1(0xFF);E1(0xD0);
    for(int i=0;i<6;i++){E1(0xF3);E1(0x0F);E1(0x6F);E1((BYTE)(0x44+i*8));E1(0x24);E1((BYTE)(0x20+i*16));}
    BYTE addrsp[]={0x48,0x81,0xC4,0x88,0x00,0x00,0x00};for(int i=0;i<7;i++)E1(addrsp[i]);
    E1(0x41);E1(0x5B);E1(0x41);E1(0x5A);E1(0x41);E1(0x59);E1(0x41);E1(0x58);
    E1(0x5A);E1(0x59);E1(0x58);E1(0x9D);
    // emulate displaced LEA r8,[rip+...] using mov r8,abs64
    E1(0x49);E1(0xB8);ULONGLONG nameptr=(ULONGLONG)(ULONG_PTR)name;acopy(p,&nameptr,8);p+=8;
    // jmp [rip+0] absolute to next instruction. Preserve every register.
    E1(0xFF);E1(0x25);E1(0);E1(0);E1(0);E1(0);
    ULONGLONG ret=(ULONGLONG)(ULONG_PTR)(target+7);acopy(p,&ret,8);p+=8;
    #undef E1
    #undef EB
    return (int)(p-outp);
}
static int install_hook(HMODULE mod){
  BYTE*base=(BYTE*)mod,*site=base+0x8AF3D95,*name=base+0xD128330;
  BYTE*relay=alloc_near(site);
  if(!relay)return 0;
  int len=relay_emit(relay,site,name);
  if(len<60||len>4096){VirtualFree(relay,0,MEM_RELEASE);return 0;}
  FlushInstructionCache(GetCurrentProcess(),relay,len);
  LONGLONG delta=(LONGLONG)(ULONG_PTR)relay-(LONGLONG)(ULONG_PTR)(site+5);
  if(delta>0x7FFFFFFFLL||delta < -0x80000000LL){VirtualFree(relay,0,MEM_RELEASE);return 0;}
  DWORD old=0;
  if(!VirtualProtect(site,7,PAGE_EXECUTE_READWRITE,&old)){VirtualFree(relay,0,MEM_RELEASE);return 0;}
  // Recheck original bytes before installation.
  if(!abytes_eq(site,gOriginalA,7)){DWORD temp;VirtualProtect(site,7,old,&temp);VirtualFree(relay,0,MEM_RELEASE);return 0;}
  BYTE branch[7]={0xE9,0,0,0,0,0x90,0x90};int rel=(int)delta;acopy(branch+1,&rel,4);
  // Since other Edge threads may run, installation has an intrinsic race.
  // The experiment must be run before signing in, only in an isolated VM profile.
  acopy(site,branch,7);
  FlushInstructionCache(GetCurrentProcess(),site,7);
  DWORD temp=0;VirtualProtect(site,7,old,&temp);
  gTarget=site;gRelay=relay;gInstalled=1;
  return 1;
}
static LPVOID WINAPI worker(LPVOID unused){
  (void)unused;
  wchar_t temp[900];zero_mem(temp,sizeof(temp));
  if(GetEnvironmentVariableW(L"TEMP",temp,900)>0 && wappend(temp,900,L"\\Edge109SyncFix-status.txt")){
    acopy(gStatusFile,temp,(wlen(temp)+1)*sizeof(wchar_t));
  }
  write_status("WAITING_FOR_MODULE");
  HMODULE mod=0;
  for(int i=0;i<150;i++){
    mod=GetModuleHandleW(L"msedge.dll");
    if(mod)break;
    Sleep(100);
  }
  if(!mod){write_status("FAILED: msedge.dll not loaded");return 0;}
  if(!validate_module(mod)){write_status("FAILED: unsupported Edge build / mismatched RVA or machine code");return 0;}
  if(!install_hook(mod)){write_status("FAILED: could not install validated hook");return 0;}
  write_status("INSTALLED: in-process header hook active (UNVERIFIED)");
  OutputDebugStringA("Edge109SyncFix: hook installed\n");
  for(;;){Sleep(4000);write_status("INSTALLED: hook active, experimental build");}
}
BOOL WINAPI DllMain(HMODULE h,DWORD why,LPVOID reserved){
  (void)reserved;
  if(why==DLL_PROCESS_ATTACH){
    DisableThreadLibraryCalls(h);
    HANDLE t=CreateThread(0,0,worker,0,0,0);
    if(t)CloseHandle(t);
  }
  return TRUE;
}
