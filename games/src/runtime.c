#include <stddef.h>
void*memset(void*ptr,int value,size_t size){unsigned char*p=ptr;for(size_t i=0;i<size;i++)p[i]=(unsigned char)value;return ptr;}
void*memcpy(void*target,const void*source,size_t size){unsigned char*t=target;const unsigned char*s=source;for(size_t i=0;i<size;i++)t[i]=s[i];return target;}
void*memmove(void*target,const void*source,size_t size){unsigned char*t=target;const unsigned char*s=source;if(t<s){for(size_t i=0;i<size;i++)t[i]=s[i];}else{while(size)t[size-1]=s[size-1],size--;}return target;}
void __aeabi_memcpy(void*t,const void*s,size_t n){memcpy(t,s,n);}
void __aeabi_memcpy4(void*t,const void*s,size_t n){memcpy(t,s,n);}
void __aeabi_memclr(void*t,size_t n){memset(t,0,n);}
void __aeabi_memclr4(void*t,size_t n){memset(t,0,n);}
