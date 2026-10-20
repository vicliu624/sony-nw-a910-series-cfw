/* R14 ABI1: bounded FAT manifests; no shell interpretation or raw writes. */
#ifndef APP_ROOT
#define APP_ROOT "/tmp/a918-game-disk/APPS"
#endif
#ifndef APP_RAM
#define APP_RAM "/tmp/a918-app-runtime"
#endif
static unsigned slen(const char*s){unsigned n=0;while(s[n])n++;return n;}
struct launcher_entry {char id[33],name[33];unsigned size,crc;};
static struct launcher_entry entries[32];static int entry_count;
static int same(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
static void copystr(char*d,const char*s){while((*d++=*s++)){} }
static int validid(const char*s){int n=0;for(;*s;s++,n++)if(!((*s>='a'&&*s<='z')||(*s>='0'&&*s<='9')||*s=='-'))return 0;return n>0&&n<=32;}
static void apppath(char*out,const char*id,const char*file){copystr(out,APP_ROOT "/");while(*out)out++;copystr(out,id);while(*out)out++;*out++='/';copystr(out,file);}
static unsigned crcbytes(unsigned crc,const unsigned char*p,int n){while(n--){crc^=*p++;for(int b=0;b<8;b++)crc=(crc>>1)^(0xedb88320u&-(crc&1));}return crc;}
static int parse_number(const char*s,unsigned*out,int base){unsigned v=0;int n=0;while(*s){int d=*s>='0'&&*s<='9'?*s-'0':*s>='a'&&*s<='f'?*s-'a'+10:-1;if(d<0||d>=base||v>(0xffffffffu-d)/(unsigned)base)return 0;v=v*base+d;s++;n++;}if(!n)return 0;*out=v;return 1;}
static int readmanifest(const char*id,struct launcher_entry*e){
 char path[192],buf[513];if(!validid(id))return 0;apppath(path,id,"app.ini");int fd=SC(5,path,0100000,0);if(fd<0)return 0;int n=SC(3,fd,buf,513);SC(6,fd,0,0);if(n<=0||n>512)return 0;buf[n]=0;unsigned seen=0;
 for(int start=0;start<n;){int end=start;while(end<n&&buf[end]!='\n')end++;buf[end]=0;char*line=buf+start;char*v=line;while(*v&&*v!='=')v++;if(!*v)return 0;*v++=0;unsigned bit=0;
  if(same(line,"api")){bit=1;if(!same(v,"1"))return 0;}
  else if(same(line,"id")){bit=2;if(!same(v,id))return 0;copystr(e->id,v);}
  else if(same(line,"name")){bit=4;int k=0;while(v[k]){if(k>=32||!((v[k]>='A'&&v[k]<='Z')||(v[k]>='0'&&v[k]<='9')||v[k]==' '||v[k]=='-'))return 0;k++;}if(!k)return 0;copystr(e->name,v);}
  else if(same(line,"exec")){bit=8;if(!same(v,"app"))return 0;}
  else if(same(line,"bytes")){bit=16;if(!parse_number(v,&e->size,10)||e->size<52||e->size>2097152)return 0;}
  else if(same(line,"crc32")){bit=32;if(!parse_number(v,&e->crc,16))return 0;}
  else if(same(line,"version")){bit=64;int k=0;for(;v[k];k++)if(k>=24||!((v[k]>='0'&&v[k]<='9')||v[k]=='.'||(v[k]>='a'&&v[k]<='z')||v[k]=='-'))return 0;if(!k)return 0;}
  else return 0;
  if(seen&bit)return 0;seen|=bit;start=end+1;
 }return seen==127;
}
int launcher_scan(void){entry_count=0;int fd=SC(5,APP_ROOT,040000|0100000,0);if(fd<0)return 0;unsigned char buf[2048];int n;
 while((n=SC(141,fd,buf,sizeof(buf)))>0){for(int off=0;off<n;){if(n-off<11)goto done;unsigned len=buf[off+8]|buf[off+9]<<8;if(len<12||len>(unsigned)(n-off))goto done;char*id=(char*)buf+off+10;int limit=len-10,k=0;while(k<limit&&id[k])k++;if(k==limit)goto done;struct launcher_entry e;if(entry_count<32&&readmanifest(id,&e))entries[entry_count++]=e;off+=len;}}
 done:SC(6,fd,0,0);
 /* Stable ordering also binds selection index across --games and --stage. */
 for(int i=0;i<entry_count;i++)for(int j=i+1;j<entry_count;j++){int k=0;while(entries[i].id[k]&&entries[i].id[k]==entries[j].id[k])k++;if(entries[i].id[k]>entries[j].id[k]){struct launcher_entry t=entries[i];entries[i]=entries[j];entries[j]=t;}}return entry_count;
}
const char*launcher_name(int i){return entries[i].name;}
static int write_full(int fd,const void*p,int n){int off=0;while(off<n){int r=SC(4,fd,(const char*)p+off,n-off);if(r==-4)continue;if(r<=0)return 0;off+=r;}return 1;}
static int launcher_stage(const char*arg){unsigned index;if(!parse_number(arg,&index,10)||index>=32)return 1;launcher_scan();if(index>=(unsigned)entry_count)return 1;struct launcher_entry*e=&entries[index];char path[192];apppath(path,e->id,"app");int in=SC(5,path,0100000,0);if(in<0)return 1;
 int out=SC(5,APP_RAM "/program.tmp",1|64|512|0100000,0700),ok=out>=0;unsigned total=0,crc=0xffffffffu;unsigned char buf[4096];
 while(ok){int n=SC(3,in,buf,sizeof(buf));if(n==-4)continue;if(n<0){ok=0;break;}if(!n)break;if(!total&&(n<52||buf[0]!=127||buf[1]!='E'||buf[2]!='L'||buf[3]!='F'||buf[4]!=1||buf[5]!=1||buf[16]!=2||buf[17]!=0||buf[18]!=40||buf[19]!=0)){ok=0;break;}total+=n;if(total>e->size){ok=0;break;}crc=crcbytes(crc,buf,n);ok=write_full(out,buf,n);}
 SC(6,in,0,0);if(out>=0&&SC(6,out,0,0)<0)ok=0;ok=ok&&total==e->size&&(crc^0xffffffffu)==e->crc;
 if(ok)ok=SC(15,APP_RAM "/program.tmp",0700,0)>=0&&SC(38,APP_RAM "/program.tmp",APP_RAM "/program",0)>=0;
 if(ok){/* App working directory owns its ROM and saves. */apppath(path,e->id,"");int fd=SC(5,APP_RAM "/cwd",1|64|512|0100000,0600);ok=fd>=0&&write_full(fd,path,slen(path));if(fd>=0)SC(6,fd,0,0);}
 if(!ok){SC(10,APP_RAM "/program.tmp",0,0);SC(10,APP_RAM "/program",0,0);}return ok?0:1;
}
static int launcher_catalog_test(void){launcher_scan();for(int i=0;i<entry_count;i++){SC(4,1,entries[i].id,slen(entries[i].id));SC(4,1,"\n",1);}return 0;}
