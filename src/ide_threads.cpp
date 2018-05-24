//---------------------------------------------------------------------------
#if defined _MSC_VER
#define __WIN32__
#define for if(0);else for
#endif

#ifdef __WIN32__
 #include <windows.h>
 #include <process.h>
 #define THREAD_START(tid,f,p)  ( ((tid)=_beginthread((f),0,(p))) != (unsigned)-1L)
 #define THREAD_WAIT(tid)       WaitForSingleObject((HANDLE)tid,INFINITE)
 #define THREAD_INIT_ATTR()
 typedef unsigned long          TID_T;
 #define _sleep(n)              Sleep(n)
 typedef CRITICAL_SECTION       CRIT_SEC;
 #define CRIT_SEC_INIT(cs)      InitializeCriticalSection(cs)
 #define CRIT_SEC_DONE(cs)      DeleteCriticalSection(cs)
 #define CRIT_SEC_ENTER(cs)     EnterCriticalSection(cs)
 #define CRIT_SEC_LEAVE(cs)     LeaveCriticalSection(cs)
#else
 #include <unistd.h>
 #include <pthread.h>
 #define THREAD_START(tid,f,p)  (pthread_create(&tid,NULL,(f),(p))==0)
 #define THREAD_WAIT(tid)       pthread_join( tid, NULL)
 #define THREAD_INIT_ATTR()     \
 do\
 {\
    pthread_attr_t attr;\
    pthread_attr_init(&attr);\
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);\
 }\
 while (0)
 typedef pthread_t              TID_T;
 #define _sleep(n)              usleep((n)*1000)
 typedef pthread_mutex_t        CRIT_SEC;
 #define CRIT_SEC_INIT(cs)      pthread_mutex_init(cs,NULL)
 #define CRIT_SEC_DONE(cs)
 #define CRIT_SEC_ENTER(cs)     pthread_mutex_lock(cs)
 #define CRIT_SEC_LEAVE(cs)     pthread_mutex_unlock(cs)
#endif

class ServerInfo;
typedef struct
{
    ServerInfo **sv;
    int svnumb;
    int recvnumb;
    int timeout;
    bool thread_stop;
    bool thread_on;
    CRIT_SEC cs;
} threadParam;

#include "ide_netbasic.h"
#include "ide_svinfo.h"
#include "ide_threads.h"
//---------------------------------------------------------------------------
enum{
 senddelayDefault=10
,recvdelayDefault=0
,numtryGroup=6
};

extern long Lnch2SrvChallenge[GM_NUMB];
extern bool MustTerminated;
//---------------------------------------------------------------------------
static void
#ifdef __WIN32__
__cdecl
#else
*
#endif
ReceiveThread(void *p)
{
    threadParam *tp=(threadParam*)p;
    tp->thread_on=true;
    while(!tp->thread_stop && !MustTerminated)
    {
        if(I_DoSelect()<=0)
        {
		    CRIT_SEC_ENTER(&tp->cs);
            tp->timeout++;
		    CRIT_SEC_LEAVE(&tp->cs);
            continue;
        }
        if(NET_GetPacket()==0) continue;
        int t=I_GetTime();
        CRIT_SEC_ENTER(&tp->cs);
        tp->timeout=0;
        CRIT_SEC_LEAVE(&tp->cs);
        for(int i=0;i<tp->svnumb;++i)
        {
            if(tp->sv[i]->connected) continue;
            if(!NET_CompareAdr(tp->sv[i]->naddress,net_from)) continue;
            tp->sv[i]->recvtime=t;
            tp->sv[i]->ReadPacket();
		    CRIT_SEC_ENTER(&tp->cs);
            tp->recvnumb++;
		    CRIT_SEC_LEAVE(&tp->cs);
            break;
        }
    }
    tp->thread_on=false;
#if !defined __WIN32__
	return NULL;
#endif
}
//---------------------------------------------------------------------------
void ServerGroupQuery(ServerInfo **sv, int svnumb,int senddelay,int recvdelay)
{
    if(senddelay<0) senddelay=senddelayDefault;
    if(recvdelay<0) recvdelay=recvdelayDefault;

    threadParam tp;
    TID_T tid;

	CRIT_SEC_INIT(&tp.cs);
    tp.sv=sv;
    tp.svnumb=svnumb;
    tp.thread_stop=false;
    tp.thread_on=false;
    tp.timeout=0;
    tp.recvnumb=0;

    for(int j=0;j<svnumb;++j) sv[j]->connected=false;

    NET_Clear();

    THREAD_INIT_ATTR();
    if(!THREAD_START(tid,ReceiveThread,&tp)) return;
    while(!tp.thread_on) _sleep(100);

    int add_step=2;   // additional number of tries if not answered first time
    int sendnumb,recvnumb;
    for(int i=0;i<numtryGroup && !MustTerminated;++i)
    {
        sendnumb=0;
        for(int j=0;j<svnumb && !MustTerminated;++j)
        {
            if(sv[j]->connected) continue;
            QueryServer(sv[j]->address,sv[j]->gmflag,Lnch2SrvChallenge[sv[j]->gmflag]);
            sv[j]->sendtime=I_GetTime();
            sendnumb++;
            _sleep(senddelay);
        }
        if(!sendnumb) break;

        while(tp.timeout<=recvdelay && tp.thread_on) _sleep(100);
        CRIT_SEC_ENTER(&tp.cs);
        recvnumb=tp.recvnumb;
        tp.timeout=0;
        tp.recvnumb=0;
        CRIT_SEC_LEAVE(&tp.cs);

        if(sendnumb==recvnumb) break;
        if(recvnumb)      add_step=0;
        else if(add_step) add_step--;
        else              break;
    }
    tp.thread_stop=true;
    THREAD_WAIT(tid);
	CRIT_SEC_DONE(&tp.cs);
}
//---------------------------------------------------------------------------

