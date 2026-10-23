#include "x86mini.h"

/* LegacyEdgeSync x86 v0.3-test2 - build-specific diagnostic triple-callsite hook.
 * Untested on Windows 7 x86. Keeps the v0.3-test1 x86 target build restrictions.
 * Patches only writable in-memory header values, never Edge files on disk. */

static volatile DWORD gHits=0,gPatched=0,gSkipped=0,gInstalledSites=0;
static volatile DWORD gPathHits[3]={0,0,0};
static volatile DWORD gPathPatched[3]={0,0,0};
static BYTE* gRelays[3]={0,0,0};
static wchar16 gStatus[1024];
static const char kOld[]="109.0.1518.140";
static const char kNew[]="133.0.3065.120";
static const BYTE kMark[]="appVer=";
static const DWORD kExpectedImageSize=0x0bf99000;

typedef struct _HOOK_POINT {
 DWORD siteRva,oldFnRva,nameRva;
 BYTE expected[5];
 BYTE kind; /* 0: setter(name,length,value,length); 1: formatter(out,format,name,value). */
} HOOK_POINT;
static const HOOK_POINT kPoints[3]={
 /* Original v0.3-test1 site (setter). */
 {0x05e976c8,0x03deed70,0x0af744dc,{0xe8,0xa3,0x76,0xf5,0xfd},0},
 /* Additional setter site uses a different copy of the header name. */
 {0x0765ac2b,0x03deed70,0x0b17bd6e,{0xe8,0x40,0x41,0x79,0xfc},0},
 /* Additional formatting path: "X-AFS-ClientInfo:%s". */
 {0x05e9723d,0x03c288b0,0x0af744dc,{0xe8,0x6e,0x16,0xd9,0xfd},1}
};

static void write_key(HANDLE h,const char*key,DWORD v){
 DWORD w=0;char rev[16],buf[16];int n=0;
 WriteFile(h,key,(DWORD)slen(key),&w,0);
 do{rev[n++]=(char)('0'+v%10);v/=10;}while(v&&n<15);
 for(int i=0;i<n;i++)buf[i]=rev[n-1-i];
 WriteFile(h,buf,n,&w,0);
}
static void status(const char*state){
 if(!gStatus[0])return;
 HANDLE h=CreateFileW(gStatus,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
 if(!h||(ULONG_PTR)h==(ULONG_PTR)-1)return;
 DWORD w=0;
 WriteFile(h,state,(DWORD)slen(state),&w,0);
 const char hdr[]="\r\nLegacyEdgeSync x86 v0.3-test2 (diagnostic, unverified)\r\nTarget: Edge 109.0.1518.140 x86\r\n";
 WriteFile(h,hdr,sizeof(hdr)-1,&w,0);
 write_key(h,"installed_sites=",gInstalledSites);
 write_key(h,"\r\npid=",GetCurrentProcessId());
 write_key(h,"\r\nhits=",gHits);
 write_key(h,"\r\npatched=",gPatched);
 write_key(h,"\r\nskipped=",gSkipped);
 write_key(h,"\r\nsetterA_hits=",gPathHits[0]);
 write_key(h,"\r\nsetterA_patched=",gPathPatched[0]);
 write_key(h,"\r\nsetterB_hits=",gPathHits[1]);
 write_key(h,"\r\nsetterB_patched=",gPathPatched[1]);
 write_key(h,"\r\nformat_hits=",gPathHits[2]);
 write_key(h,"\r\nformat_patched=",gPathPatched[2]);
 WriteFile(h,"\r\n",2,&w,0);CloseHandle(h);
}
static int proper_path(HMODULE m){
 wchar16 path[MAXW];zero_mem(path,sizeof(path));
 DWORD n=GetModuleFileNameW(m,path,MAXW);
 if(!n||n>=MAXW)return 0;
 const wchar16 version[]=L"109.0.1518.140";
 SIZE_T z=wslen(version);
 for(SIZE_T i=0;i+z<=n;i++){
  SIZE_T j=0;while(j<z&&path[i+j]==version[j])j++;
  if(j==z)return 1;
 }return 0;
}
static int validate(HMODULE mod){
 BYTE*b=(BYTE*)mod;
 if(b[0]!='M'||b[1]!='Z')return 0;
 DWORD peoff=*(DWORD*)(b+0x3c);
 if(peoff<0x40||peoff>0x400)return 0;
 BYTE*pe=b+peoff;
 if(pe[0]!='P'||pe[1]!='E'||pe[2]!=0||pe[3]!=0)return 0;
 if(*(WORD*)(pe+4)!=0x14c || *(WORD*)(pe+24)!=0x10b)return 0;
 if(*(DWORD*)(pe+24+56)!=kExpectedImageSize)return 0;
 if(!proper_path(mod))return 0;
 if(!eq_bytes(b+0x0af745ad,(const BYTE*)"appVer=",7))return 0;
 if(!eq_bytes(b+0x0b17bd55,(const BYTE*)"appVer=",7))return 0;
 for(int i=0;i<3;i++){
  const HOOK_POINT*pt=&kPoints[i];BYTE*site=b+pt->siteRva;
  if(!eq_bytes(site,pt->expected,5))return 0;
  if(!eq_bytes(b+pt->nameRva,(const BYTE*)"X-AFS-ClientInfo",16))return 0;
  int rel=*(int*)(site+1);
  if(site+5+rel!=b+pt->oldFnRva)return 0;
  if(pt->kind==0){
   if(site[-5]!=0x68 || *(DWORD*)(site-4)!=(DWORD)(ULONG_PTR)(b+pt->nameRva))return 0;
   if(site[-7]!=0x6a || site[-6]!=0x10)return 0;
  }else{
   /* push value ; push header ; push format ; push destination ; call */
   if(site[-12]!=0x57 || site[-11]!=0x68 || *(DWORD*)(site-10)!=(DWORD)(ULONG_PTR)(b+pt->nameRva) ||
      site[-6]!=0x68 || site[-1]!=0x56)return 0;
  }
 }
 return 1;
}
/* The ID is passed by each call-site relay (0=A, 1=B, 2=formatter).
 * The formatter provides a zero-terminated string; the setters have explicit lengths. */
static void CDECL patch_header(const BYTE*val,DWORD len,DWORD id){
 ++gHits;
 if(id<3)++gPathHits[id];
 if(!val||id>2||len>4096){++gSkipped;return;}
 MEMORY_BASIC_INFORMATION mi;zero_mem(&mi,sizeof(mi));
 if(VirtualQuery(val,&mi,sizeof(mi))!=sizeof(mi)||mi.State!=MEM_COMMIT||(mi.Protect&(PAGE_GUARD|PAGE_NOACCESS))){++gSkipped;return;}
 DWORD prot=mi.Protect&0xff;
 if(!(prot==PAGE_READWRITE||prot==PAGE_EXECUTE_READWRITE||prot==PAGE_WRITECOPY||prot==PAGE_EXECUTE_WRITECOPY)){++gSkipped;return;}
 ULONG_PTR start=(ULONG_PTR)val,regionStart=(ULONG_PTR)mi.BaseAddress;
 if(start<regionStart){++gSkipped;return;}
 SIZE_T within=start-regionStart;
 if(within>=mi.RegionSize){++gSkipped;return;}
 SIZE_T remaining=mi.RegionSize-within;
 if(id==2){
  /* Format arguments are expected to be a C string; bound the scan by mapped memory. */
  if(remaining>4096)remaining=4096;
  len=0;while(len<remaining&&val[len])len++;
 } else if(len>remaining){++gSkipped;return;}
 if(len<21){++gSkipped;return;}
 for(DWORD i=0;i+21<=len;i++){
  if(eq_bytes(val+i,kMark,7)&&eq_bytes(val+i+7,(const BYTE*)kOld,14)&&
    (i+21==len || val[i+21]==';'||val[i+21]==' '||val[i+21]=='\0'||val[i+21]=='&')){
   copy_mem((BYTE*)val+i+7,kNew,14);
   ++gPatched;++gPathPatched[id];return;
  }
 }
 ++gSkipped;
}
static BYTE*alloc_near(BYTE*site){
 ULONG_PTR ref=((ULONG_PTR)site)&~(ULONG_PTR)0xffff;
 for(ULONG_PTR delta=0x10000;delta<0x60000000;delta+=0x10000){
  ULONG_PTR p[2]={ref>=delta?ref-delta:0,ref+delta};
  for(int j=0;j<2;j++){
   if(!p[j]||p[j]>0x7fff0000)continue;
   long long distance=(long long)p[j]-((ULONG_PTR)site+5);
   if(distance>0x7fffffffLL||distance<(-2147483647LL-1))continue;
   BYTE*area=(BYTE*)VirtualAlloc((LPVOID)p[j],4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
   if(area)return area;
  }
 }
 return 0;
}
static int install_hook(HMODULE module,int id){
 BYTE*base=(BYTE*)module;
 const HOOK_POINT*pt=&kPoints[id];
 BYTE*site=base+pt->siteRva;
 BYTE*relay=alloc_near(site);
 if(!relay)return 0;
 BYTE*p=relay;
 *p++=0x9c;*p++=0x60; /* pushfd, pushad */
 *p++=0x6a;*p++=(BYTE)id; /* third C argument: site ID */
 if(pt->kind==0){
  /* Original call args: [ESP+4] header, +8 headerLen, +12 value, +16 valueLen.
   * Saved ESP delta is 36 bytes; after ID push, +16 = +0x38.
   * After length push, value pointer +12 is also +0x38. */
  for(int i=0;i<2;i++){
   *p++=0xff;*p++=0x74;*p++=0x24;*p++=0x38;
  }
 }else{
  /* Formatter original call: [ESP+4] dst, +8 fmt, +12 header, +16 value.
   * A zero length instructs the callback to scan a bounded C string. */
  *p++=0x6a;*p++=0x00;
  *p++=0xff;*p++=0x74;*p++=0x24;*p++=0x3c;
 }
 *p++=0xb8;DWORD cb=(DWORD)(ULONG_PTR)&patch_header;copy_mem(p,&cb,4);p+=4;
 *p++=0xff;*p++=0xd0; /* call eax */
 *p++=0x83;*p++=0xc4;*p++=0x0c; /* caller cleans three arguments */
 *p++=0x61;*p++=0x9d; /* popad, popfd */
 *p++=0x68;DWORD fn=(DWORD)(ULONG_PTR)(base+pt->oldFnRva);copy_mem(p,&fn,4);p+=4;
 *p++=0xc3; /* ret to original function, leaving the call's original return address */
 SIZE_T size=(SIZE_T)(p-relay);
 if(size>128){VirtualFree(relay,0,MEM_RELEASE);return 0;}
 FlushInstructionCache(GetCurrentProcess(),relay,size);
 long long delta=(long long)(ULONG_PTR)relay-(long long)((ULONG_PTR)site+5);
 if(delta>0x7fffffffLL||delta<(-2147483647LL-1)){VirtualFree(relay,0,MEM_RELEASE);return 0;}
 DWORD old=0;
 if(!VirtualProtect(site,5,PAGE_EXECUTE_READWRITE,&old)){VirtualFree(relay,0,MEM_RELEASE);return 0;}
 if(!eq_bytes(site,pt->expected,5)){
  DWORD tmp;VirtualProtect(site,5,old,&tmp);VirtualFree(relay,0,MEM_RELEASE);return 0;
 }
 int newRel=(int)delta;copy_mem(site+1,&newRel,4);
 FlushInstructionCache(GetCurrentProcess(),site,5);
 DWORD tmp;VirtualProtect(site,5,old,&tmp);
 gRelays[id]=relay;
 ++gInstalledSites;
 return 1;
}
static DWORD WINAPI worker(LPVOID context){
 (void)context;
 wchar16 temp[1024];zero_mem(temp,sizeof(temp));
 if(GetEnvironmentVariableW(L"TEMP",temp,1024)>0 && wappend(temp,1024,L"\\LegacyEdgeSync-x86-status.txt"))
  copy_mem(gStatus,temp,(wslen(temp)+1)*2);
 status("WAITING_FOR_MODULE");
 HMODULE m=0;
 for(int i=0;i<180;i++){m=GetModuleHandleW(L"msedge.dll");if(m)break;Sleep(100);}
 if(!m){status("FAILED: msedge.dll not loaded");return 0;}
 if(!validate(m)){status("FAILED: unsupported x86 Edge build / mismatched call-site bytes");return 0;}
 for(int i=0;i<3;i++){
  if(!install_hook(m,i)){
   status("FAILED: partial x86 hook installation; exit Edge without syncing");return 0;
  }
 }
 status("INSTALLED: x86 three-path research hooks installed");
 for(;;){Sleep(1500);status("INSTALLED: x86 experimental hooks active");}
}
BOOL WINAPI DllMain(HMODULE h,DWORD why,LPVOID unused){
 (void)unused;
 if(why==DLL_PROCESS_ATTACH){
  DisableThreadLibraryCalls(h);
  HANDLE th=CreateThread(0,0,worker,0,0,0);
  if(th)CloseHandle(th);
 }
 return TRUE;
}
