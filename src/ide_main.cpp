#if defined _MSC_VER
#define for if(0);else for
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __IDE_DLL__

#include "ide_const.h"
#include "ide_net.h"
#include "ide_netbasic.h"
#include "ide_svinfo.h"
#include "ide_version.h"
#include "ide_threads.h"
#include "ide_main.h"

#define ST_MASTER_BAN    3
#define ST_MASTER_IGNORE 4
//---------------------------------------------------------------------------
bool GmMaster;
char* MasterSvDefault[GM_NUMB]={
   "voxelsoft.com:15000"
  ,"master.zdaemon.org:15300"
  ,"skulltag.servegame.com:15300"
};
char* MasterSv[GM_NUMB];
char* Sv[MAXSERVERS];
int SvNum=0;
bool IDE_NetWarning=false;
bool IDE_Ready=false;
char IDE_WarningMessage[256];
ShowStepFcn ShowStep=NULL;
bool MustTerminated=false;

#define DEFAULTLOCALPORT 12998
int LocalPort=DEFAULTLOCALPORT;
//---------------------------------------------------------------------------
char* Strlwr(char* s)
{
    int imax=strlen(s);
    for(int i=0;i<imax;i++)
        if(s[i]>='A' && s[i]<='Z') s[i]+=32;
    return s;
}
//---------------------------------------------------------------------------
char* Stpcpy(char* s1,char* s2){ return strcpy(s1,s2)+strlen(s2);}
//---------------------------------------------------------------------------
void Printf(char* fmt,...)
{
    va_list argptr;
    va_start(argptr,fmt);
    vsprintf(IDE_WarningMessage,fmt,argptr);
    va_end(argptr);

    IDE_NetWarning=true;
}
//---------------------------------------------------------------------------
extern "C" int DLL_EXP IDE_GetVersion(void){ return IDE_VERSION; }
//---------------------------------------------------------------------------
extern "C" bool DLL_EXP IDE_GetWarningMessage(char* msg)
{
    if(!IDE_NetWarning) return false;
    strcpy(msg,IDE_WarningMessage);
    return true;
}
//---------------------------------------------------------------------------
extern "C" bool DLL_EXP IDE_Init(void)
{
    if(IDE_Ready) return true;

    IDE_NetWarning=false;
    IDE_Ready=false;
    if(!InitNetwork()) return false;
    IDE_Ready=true;

    for(int i=0;i<GM_NUMB;i++)
        MasterSv[i]=MasterSvDefault[i];

    Sv[0]=new char[MAXSERVADR];

    return IDE_Ready;
}
//---------------------------------------------------------------------------
extern "C" void DLL_EXP IDE_Close(void)
{
    if(!IDE_Ready) return;
    delete[] Sv[0];
    CloseNetwork();
    IDE_Ready=false;
}
//---------------------------------------------------------------------------
void SortServerAddress(void)
{
    char s[MAXSERVERS][MAXADR+1];
    char* p[MAXSERVERS];
    int ind[MAXSERVERS];

    for(int i=0;i<SvNum;i++)
    {
        FormatAddress(Sv[i],s[i]);
        p[i]=s[i];
    }

    StringSort(p,SvNum,ind);

    for(int i=0;i<SvNum;i++) strcpy(s[i],Sv[i]);
    for(int i=0;i<SvNum;i++) Sv[i+1]=Stpcpy(Sv[i],s[ind[i]])+1;
}
//---------------------------------------------------------------------------
extern "C" int DLL_EXP IDE_GetMasterList(int gmflag)
{
    if(!IDE_Ready) return IDE_NOT_INITIALIZED;
    IDE_NetWarning=false;
    int n=GetMasterInfo(Sv,gmflag);
    if(!GmMaster)
    {
        if(n==IDE_BAD_PACKET)
            return IDE_BAD_PACKET;
        else if(gmflag==GM_SKULLTAG && n==ST_MASTER_BAN)
            return IDE_ST_BAN;
        else if(gmflag==GM_SKULLTAG && n==ST_MASTER_IGNORE)
            return IDE_ST_IGNORE;
        else
            return IDE_NOT_CONNECTED;
    }
    SvNum=n;
    SortServerAddress();
    return SvNum;
}
//---------------------------------------------------------------------------
extern "C" bool DLL_EXP IDE_GetMasterListServer(int i,char* sv)
{
    if(!IDE_Ready) return false;
    strcpy(sv,Sv[i]);
    char* p=&sv[strlen(sv)-1]; // remove possible 't' from end of string
    if(*p=='t')                // (for "trusted" zdaemon servers)
    {
        *p=0;
        return true;
    }
    else
        return false;
}
//---------------------------------------------------------------------------
extern "C" bool DLL_EXP IDE_GetServerInfo(char* address,int gmflag,ServerData* svi)
{
    if(!IDE_Ready) return false;
    IDE_NetWarning=false;
    ServerInfo Svi(address,gmflag);
    FillServerStruct(svi,&Svi);
    return Svi.result==SVMSG_OK;
}
//---------------------------------------------------------------------------
extern "C" bool DLL_EXP IDE_GetServerGroup(ServerData *svi[],int svnumb,int senddelay,int recvdelay)
{
    if(!IDE_Ready) return false;
    IDE_NetWarning=false;

    ServerInfo **Svi=new ServerInfo*[svnumb];
    for(int i=0;i<svnumb;++i)
        Svi[i]=new ServerInfo(svi[i]->address,svi[i]->gmflag,false);

    ServerGroupQuery(Svi,svnumb,senddelay,recvdelay);

    for(int i=0;i<svnumb;++i)
        FillServerStruct(svi[i],Svi[i]);

    for(int i=0;i<svnumb;++i) delete Svi[i];
    delete[] Svi;
    return true;
}
//---------------------------------------------------------------------------
int FillServerStruct(ServerData* svi,ServerInfo* Svi)
{
    svi->gmflag=Svi->gmflag;
    svi->result=Svi->result;
    svi->ping=Svi->iping;
    svi->numclients=Svi->numclients;
    svi->maxclients=Svi->maxclients;
    svi->wadnumb=Svi->wadnumb;
    svi->dmflags=Svi->dmflags;
    svi->dmflags2=Svi->dmflags2;
    svi->compatflags=Svi->compatflags;
    svi->gameskill=Svi->gameskill;
    svi->gametype=Svi->gametype;
    svi->gt_instagib=Svi->gt_instagib;
    svi->gt_buckshot=Svi->gt_buckshot;
    svi->version=Svi->version;
    strcpy(svi->address,Svi->address);
    svi->forcepassword=Svi->forcepassword;
    svi->forcejoinpassword=Svi->forcejoinpassword;
    svi->numplayers=Svi->numplayers;
    svi->maxplayers=Svi->maxplayers;
    svi->timelimit=Svi->timelimit;
    svi->timeleft=Svi->timeleft;
    svi->fraglimit=Svi->fraglimit;
    svi->fragleft=Svi->fragleft;
    svi->pointlimit=Svi->pointlimit;
    svi->duellimit=Svi->duellimit;
    svi->winlimit=Svi->winlimit;
    svi->numbots=Svi->numbots;
    svi->botskill=Svi->botskill;
    svi->minplayers=Svi->minplayers;
    svi->removebotswhenhumans=Svi->removebotswhenhumans;
    svi->numspectators=Svi->numspectators;
    svi->teamscorelimit=Svi->teamscorelimit;
    svi->teamscoreleft=Svi->teamscoreleft;
    svi->teamnumb=Svi->teamnumb;
    svi->teamdamage=Svi->teamdamage;
    svi->gravity=Svi->gravity;
    svi->aircontrol=Svi->aircontrol;
    for(int i=0;i<odfNUMB;++i) svi->odafeatures[i]=Svi->odafeatures[i];
    svi->startuptime=Svi->startuptime;
    svi->acl=Svi->acl;
    svi->acluser=Svi->acluser;
    svi->voting=Svi->voting;
    svi->sttestenabled=Svi->sttestenabled;
    svi->maxlives=Svi->maxlives;
    svi->splashfactor=Svi->splashfactor;
    svi->wadcheckenabled=Svi->wadcheckenabled;

    char* curp=svi->buf;
    svi->hostname=curp;  curp=Stpcpy(curp,Svi->hostname)+1;
    svi->mapname=curp;   curp=Stpcpy(curp,Svi->mapname)+1;
    svi->gamename=curp;  curp=Stpcpy(curp,Svi->gamename)+1;
    svi->gameiwad=curp;  curp=Stpcpy(curp,Svi->gameiwad)+1;
    svi->gameiwadalt=curp; curp=Stpcpy(curp,Svi->gameiwadalt)+1;
    svi->gameiwadchecksum=curp; curp=Stpcpy(curp,Svi->gameiwadchecksum)+1;
    svi->gameiwadaltchecksum=curp; curp=Stpcpy(curp,Svi->gameiwadaltchecksum)+1;
    svi->wadurl=curp;    curp=Stpcpy(curp,Svi->wadurl)+1;
    svi->email=curp;     curp=Stpcpy(curp,Svi->email)+1;
    svi->sversion=curp;  curp=Stpcpy(curp,Svi->sversion)+1;
    svi->sttestclient=curp;  curp=Stpcpy(curp,Svi->sttestclient)+1;

    for(int i=0;i<MAX_WAD_NUMB;i++)
    {
        svi->wadname[i]=curp; curp=Stpcpy(curp,Svi->wadname[i])+1;
        svi->wadnamealt[i]=curp; curp=Stpcpy(curp,Svi->wadnamealt[i])+1;
        svi->wadchecksum[i]=curp; curp=Stpcpy(curp,Svi->wadchecksum[i])+1;
        svi->wadaltchecksum[i]=curp; curp=Stpcpy(curp,Svi->wadaltchecksum[i])+1;
        svi->wadoptional[i]=Svi->wadoptional[i];
    }
    for(int i=0;i<svi->teamnumb;i++)
    {
        svi->teamscore[i]=Svi->teamscore[i];
        svi->teamcolor[i]=Svi->teamcolor[i];
        svi->teamname[i]=curp; curp=Stpcpy(curp,Svi->teamname[i])+1;
    }

    for(int i=0;i<MAX_PLAYER_NUMB;i++)
    {
        svi->player[i].name=curp; curp=Stpcpy(curp,Svi->player[i].name)+1;
        svi->player[i].namecolor=curp; curp=Stpcpy(curp,Svi->player[i].namecolor)+1;
        svi->player[i].fragcount=Svi->player[i].fragcount;
        svi->player[i].deathcount=Svi->player[i].deathcount;
        svi->player[i].ping=Svi->player[i].ping;
        svi->player[i].minutesingame=Svi->player[i].minutesingame;
        svi->player[i].isbot=Svi->player[i].isbot;
        svi->player[i].isspectator=Svi->player[i].isspectator;
        svi->player[i].team=Svi->player[i].team;
        svi->player[i].countrycode=curp; curp=Stpcpy(curp,Svi->player[i].countrycode)+1;
    }
    return int(curp-svi->buf);
}
//---------------------------------------------------------------------------
extern "C" int DLL_EXP IDE_GetLocalPort(void){ return LocalPort; }
extern "C" void DLL_EXP IDE_SetLocalPort(int port){LocalPort=port;}
//---------------------------------------------------------------------------
extern "C" void DLL_EXP IDE_GetMasterAddress(int gmflag,char* s){ strcpy(s,MasterSv[gmflag]);}
extern "C" void DLL_EXP IDE_SetMasterAddress(int gmflag,char* s)
{
    if(!s) MasterSv[gmflag]=MasterSvDefault[gmflag];
    else   MasterSv[gmflag]=s;
}
//---------------------------------------------------------------------------
extern "C" void DLL_EXP IDE_SetShowStep(ShowStepFcn fn){ShowStep=fn;}
//---------------------------------------------------------------------------
void ShowNetStep(void){ if(ShowStep) ShowStep();}
//---------------------------------------------------------------------------
extern "C" void DLL_EXP IDE_MustTerminated(bool m){MustTerminated=m;}
//---------------------------------------------------------------------------

