//---------------------------------------------------------------------------
#include <string.h>
#include "ide_const.h"
#include "inet.h"
#include "ide_netbasic.h"
#include "ide_zdmaster.h"
//----------------------------------------------------------------------------
typedef struct {
    char data[MAXSERVADR];
    int cursize;
} HttpBuf;

extern bool GmMaster;
//----------------------------------------------------------------------------
int HTTP_Transfer_s(void* cookie,unsigned char* buf,int n)
{
    HttpBuf &svlist=*((HttpBuf*)cookie);
    if(svlist.cursize+n > MAXSERVADR) return 0;
    memcpy(svlist.data+svlist.cursize,buf,n);
    svlist.cursize+=n;
    return 1;
}
//----------------------------------------------------------------------------
int ZDGetMasterServerList(char* servers[])
{
    HttpBuf svlist;
    svlist.cursize=0;
    GmMaster=false;
    inet_init();
    inet_error_type rc=inet_fetch("http://master.zdaemon.org/masterlist.php"
        ,HTTP_Transfer_s,0,0,0,0,(void*)&svlist,3,0);
    inet_done();
    if(rc!=INETERR_NOERROR) return 0;

    int n=0;
    char *p1=svlist.data,*p2;
    while((p2=strchr(p1,'\n'))!=0)
    {
        if(*(p2-2)!='\t') {p1=p2+1; continue;}
        *(p2-2)=0;
        if(*(p2-1)=='1') *(p2-2)='t';
        *(p2-1)=0;
        servers[n+1]=Stpcpy(servers[n],p1)+1;
        if(++n==MAXSERVERS-1) break;
        p1=p2+1;
    }
    GmMaster=true;
    return n;
}
//---------------------------------------------------------------------------

