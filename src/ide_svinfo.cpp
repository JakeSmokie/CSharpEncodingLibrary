//--------------------------------------------------------------
// ServerInfo Class implementation and some attendant functions
//--------------------------------------------------------------
#if defined _MSC_VER
#define __WIN32__
#define for if(0);else for
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/types.h>
 #include <sys/time.h>
 #include <unistd.h>
 #define SOCKET_ERROR (-1)
#endif

#include "ide_netbasic.h"
#include "ide_svinfo.h"
//----------------------------------------------------------------------------
#define UPDATE_CHALLENGE   22383
//----------------------------------------------------------------------------
// Skulltag server query flags.
#define SQF_NAME                0x00000001
#define SQF_URL                 0x00000002
#define SQF_EMAIL               0x00000004
#define SQF_MAPNAME             0x00000008
#define SQF_MAXCLIENTS          0x00000010
#define SQF_MAXPLAYERS          0x00000020
#define SQF_PWADS               0x00000040
#define SQF_GAMETYPE            0x00000080
#define SQF_GAMENAME            0x00000100
#define SQF_IWAD                0x00000200
#define SQF_FORCEPASSWORD       0x00000400
#define SQF_FORCEJOINPASSWORD   0x00000800
#define SQF_GAMESKILL           0x00001000
#define SQF_BOTSKILL            0x00002000
#define SQF_DMFLAGS             0x00004000
#define SQF_LIMITS              0x00010000
#define SQF_TEAMDAMAGE          0x00020000
#define SQF_TEAMSCORES          0x00040000
#define SQF_NUMPLAYERS          0x00080000
#define SQF_PLAYERDATA          0x00100000
#define SQF_TEAMINFO_NUMBER     0x00200000
#define SQF_TEAMINFO_NAME       0x00400000
#define SQF_TEAMINFO_COLOR      0x00800000
#define SQF_TEAMINFO_SCORE      0x01000000
#define SQF_TESTING_SERVER      0x02000000
#define SQF_ALL ( SQF_NAME|SQF_URL|SQF_EMAIL|SQF_MAPNAME|SQF_MAXCLIENTS \
                 |SQF_MAXPLAYERS|SQF_PWADS|SQF_GAMETYPE|SQF_GAMENAME \
                 |SQF_IWAD|SQF_FORCEPASSWORD|SQF_FORCEJOINPASSWORD \
                 |SQF_GAMESKILL|SQF_BOTSKILL|SQF_DMFLAGS|SQF_LIMITS \
                 |SQF_TEAMDAMAGE|SQF_TEAMSCORES|SQF_NUMPLAYERS|SQF_PLAYERDATA \
                 |SQF_TEAMINFO_NUMBER|SQF_TEAMINFO_NAME|SQF_TEAMINFO_COLOR|SQF_TEAMINFO_SCORE \
                 |SQF_TESTING_SERVER \
                )

#define ST_CHALLENGE_BAN    5660025
#define ST_CHALLENGE_IGNORE 5660024
#define ST_MASTER_BEGINLISTPART 6
#define ST_MASTER_ENDLIST       2

enum {Numtry=4};
enum {with_exception=1};
//----------------------------------------------------------------------------
enum TeamColors {
 teamcolorRED=0xBF0000
,teamcolorGREEN=0xBF00
,teamcolorBLUE=0xBF
,teamcolorWHITE=0x9f9F9F
,teamcolorGOLD=0xffd700
};
static char *teamnameRED="Red";
static char *teamnameBLUE="Blue";
static char *teamnameGREEN="Green";
static char *teamnameWHITE="White";
static char *teamnameGOLD="Gold";

int teamMAX[GM_NUMB]={3,4,4};

static int GameTypesNumb[GM_NUMB]={4,4,gtNUMB};
static int GameTypes[GM_NUMB][gtNUMB]={
//Odamex
{ gtDeathmatch
 ,gtTeamDeathmatch
 ,gtCooperative
 ,gtCTF
},
//ZDaemon
{ gtDeathmatch
 ,gtTeamDeathmatch
 ,gtCooperative
 ,gtCTF
},
//Skulltag
{ gtCooperative
 ,gtSurvival
 ,gtInvasion
 ,gtDeathmatch
 ,gtTeamDeathmatch
 ,gtDuel
 ,gtTerminator
 ,gtLMS
 ,gtTeamLMS
 ,gtPossession
 ,gtTeamPossession
 ,gtTeamgame
 ,gtCTF
 ,gtOneFlagCTF
 ,gtSkulltag
 ,gtDomination
}};

char* empty_string="";

long Lnch2SrvChallenge[GM_NUMB]={777123,777123,199};
long Srv2LnchChallenge[GM_NUMB]={5560020,5560020,5660023};

const long ODLnch2MstChallenge=777123;
const long ODMst2LnchChallenge=777123;

const long STLnch2MstChallenge=5660028;
const int STMasterServerVersion=2;

const long Lnch2SrvProtocol_zdver=4294901244LU;
const long Lnch2SrvVersion_zdver=3;
const long Srv2LnchChallenge_zdver=5560022;

netadr_t LastQueriedServer;

extern bool MustTerminated;
extern bool GmMaster;
extern char* MasterSv[GM_NUMB];

void ShowNetStep(void);
int ZDGetMasterServerList(char* servers[]);
//----------------------------------------------------------------------------
ServerInfo::ServerInfo()
{
    Init();
}
//----------------------------------------------------------------------------
ServerInfo::ServerInfo(char* adr,int _gmflag,bool queryserver)
{
    Init();

    gmflag=_gmflag;
    strncpy(address,adr,MAXADR);
    address[MAXADR]=0;
    int alen=strlen(address)-1;
    if(address[alen]=='t') address[alen]=0; //remove 'trusted' flag
    NET_StringToAdr(address,&naddress);

    connected=false;
    result=SVMSG_NCON;

    if(queryserver) FillInfo();
}
//----------------------------------------------------------------------------
void ServerInfo::Init(void)
{
    connected=false;
    result=SVMSG_INIT;
    gmflag=0;
    iping=0;
    address[0]=0;
    naddress.ip[0]=naddress.ip[1]=naddress.ip[2]=naddress.ip[3]=0;
    naddress.port=naddress.pad=0;
    sendtime=0;
    recvtime=0;

    InitPacket();

    cursize=0;
}
//----------------------------------------------------------------------------
void ServerInfo::InitPacket(void)
{
    numclients=0;
    maxclients=0;
    numplayers=0;
    maxplayers=0;
    numbots=0;
    numspectators=0;
    minplayers=0;
    removebotswhenhumans=false;
    gameskill=0;
    botskill=0;
    gametype=0;
    gt_instagib=false;
    gt_buckshot=false;
    forcepassword=false;
    forcejoinpassword=false;
    gamename=empty_string;
    gameiwad=empty_string;
    gameiwadchecksum=empty_string;
    gameiwadalt=empty_string;
    gameiwadaltchecksum=empty_string;
    wadurl=empty_string;
    email=empty_string;
    sversion=empty_string;
    version=0;
    dmflags=0;
    dmflags2=0;
    compatflags=0;
    hostname=empty_string;
    mapname=empty_string;
    wadnumb=0;
    for(int i=0;i<MAX_WAD_NUMB;i++)
    {
        wadname[i]=empty_string;
        wadoptional[i]=false;
        wadnamealt[i]=empty_string;
        wadchecksum[i]=empty_string;
        wadaltchecksum[i]=empty_string;
    }
    wadcheckenabled=false;
    timelimit=0;
    timeleft=0;
    fraglimit=0;
    pointlimit=0;
    duellimit=0;
    winlimit=0;
    fragleft=0;
    teamflag=false;
    teamnumb=0;
    teamscorelimit=0;
    teamscoreleft=0;
    teamdamage=0;
    gravity=0;
    aircontrol=0;
    startuptime=0;
    acl=false;
    acluser=0;
    voting=0;
    proto_zdver=0;
    sttestenabled=false;
    sttestclient=empty_string;
    maxlives=0;
    splashfactor=0;

    for(int i=0;i<odfNUMB;++i) odafeatures[i]=false;

    InitPlayer(player,MAX_PLAYER_NUMB);
    InitTeams();
}
//----------------------------------------------------------------------------
void ServerInfo::InitPlayer(PlayerInfo* player,int n)
{
    for(int i=0;i<n;i++)
    {
        player[i].name=empty_string;
        player[i].namecolor=empty_string;
        player[i].fragcount=0;
        player[i].deathcount=0;
        player[i].ping=0;
        player[i].level=0;
        player[i].minutesingame=0;
        player[i].isbot=false;
        player[i].isspectator=false;
        player[i].team=0;
        player[i].countrycode=empty_string;
    }
}
//----------------------------------------------------------------------------
void ServerInfo::InitTeams()
{
    for(int i=0;i<teamMAX[gmflag];++i) teamscore[i]=0;
    switch(gmflag)
    {
    case GM_ODAMEX:
        teamcolor[0]=teamcolorBLUE; teamname[0]=teamnameBLUE;
        teamcolor[1]=teamcolorRED;  teamname[1]=teamnameRED;
        teamcolor[2]=teamcolorGOLD; teamname[2]=teamnameGOLD;
        break;
    case GM_ZDAEMON:
        teamcolor[0]=teamcolorRED;  teamname[0]=teamnameRED;
        teamcolor[1]=teamcolorBLUE; teamname[1]=teamnameBLUE;
        teamcolor[2]=teamcolorGREEN;teamname[2]=teamnameGREEN;
        teamcolor[3]=teamcolorWHITE;teamname[3]=teamnameWHITE;
        break;
    case GM_SKULLTAG:
        teamcolor[0]=teamcolorBLUE; teamname[0]=teamnameBLUE;
        teamcolor[1]=teamcolorRED;  teamname[1]=teamnameRED;
        break;
    }
}
//----------------------------------------------------------------------------
char* ServerInfo::SetString(char* s)
{
    int l=strlen(s)+1;
    if(cursize+l>=MAX_UDP_PACKET)
    {
        result=SVMSG_MAXBUF;
        return empty_string;
    }
    char* r=strcpy(&buf[cursize],s);
    cursize+=l;
    return r;
}
//----------------------------------------------------------------------------
char* ServerInfo::ReadChecksum()
{
    static char s[33];
    for(int i=0;i<16;i++)
        sprintf(&s[2*i],"%02x",MSG_ReadByte(with_exception));
    s[32]=0;
    return s;
}
//----------------------------------------------------------------------------
// Query server, receive and analyse info
//----------------------------------------
void ServerInfo::FillInfo(void)
{
    NET_Clear();
    int numtry=Numtry;
    while(numtry--)
    {
        if(MustTerminated) return;
        QueryServer(address,gmflag,Lnch2SrvChallenge[gmflag]);
        sendtime=I_GetTime();
        ShowNetStep();
        if(!(I_DoSelect() && NET_GetPacket() && NET_CompareAdr(naddress,net_from)))
            continue; //repeat query
        recvtime=I_GetTime();
        ReadPacket();
        return;
    }
}
//----------------------------------------------------------------------------
// analyse info from received packet
//----------------------------------------
void ServerInfo::ReadPacket(void)
{
    if(gmflag==GM_SKULLTAG) NET_HuffDecodePacket();

    int chl=MSG_ReadLong();
    if(chl==Srv2LnchChallenge[gmflag])
        result=SVMSG_OK;
    else if(chl==Srv2LnchChallenge_zdver && gmflag==GM_ZDAEMON)
        result=SVMSG_OK;
    else if(chl==ST_CHALLENGE_IGNORE && gmflag==GM_SKULLTAG)
        result=SVMSG_ST_IGNORE;
    else if(chl==ST_CHALLENGE_BAN && gmflag==GM_SKULLTAG)
        result=SVMSG_ST_BAN;
    else
    {
        result=SVMSG_BADCL;
        return;
    }

    proto_zdver=0;
    if(gmflag==GM_ZDAEMON && chl!=Srv2LnchChallenge_zdver )
        iping=recvtime-sendtime;
    else
    {
        if(gmflag==GM_ODAMEX)       MSG_ReadLong(); // unused "token"
        else if(gmflag==GM_ZDAEMON) proto_zdver=MSG_ReadLong();
        iping=recvtime-MSG_ReadLong();
    }

    connected=true;
    if(result!=SVMSG_OK) return;
    cursize=0;
    if(!ParsePacket())
    {
        InitPacket();
        result=SVMSG_BADPKT;
    }
}
//----------------------------------------------------------------------------
bool ServerInfo::ParsePacket(void)
{
#define MAXTMPBUF 256
    char tmpbuf[MAXTMPBUF+1];
    PlayerInfo playertmp[MAX_PLAYER_NUMB];
    InitPlayer(playertmp,MAX_PLAYER_NUMB);
    InitTeams();
    int j,scoremax;
    int indtmp[MAX_PLAYER_NUMB],fragtmp[MAX_PLAYER_NUMB];
// begin try_____________________________________________
    try
    {
// -- Odamex packet -------------------------------------
    if(gmflag==GM_ODAMEX)
    {
        hostname=SetString(MSG_ReadString(with_exception));
        numclients=MSG_ReadByte(with_exception);
        if(numclients<0 || numclients>255) return false;
        maxclients=MSG_ReadByte(with_exception);
        if(maxclients<0 || maxclients>255) return false;
        mapname=SetString(Strlwr(MSG_ReadString(with_exception)));
        if(mapname[0]=='e')
            gamename=SetString("DOOM");
        else
            gamename=SetString("DOOM II");
        wadnumb=MSG_ReadByte(with_exception);
        if(wadnumb<1) return false;
        sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
        gameiwad=SetString(Strlwr(tmpbuf));
        wadnumb--;
        int wadmax=wadnumb<=MAX_WAD_NUMB?wadnumb:MAX_WAD_NUMB;
        for(j=0;j<wadmax;j++)
        {
            sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            wadname[j]=SetString(Strlwr(tmpbuf));
        }
        for(;j<MAX_WAD_NUMB;j++) wadname[j]=empty_string;
        for(;j<wadnumb;j++) MSG_ReadString(with_exception);   // if >MAX_WAD_NUMB

        int od_deathmatch=MSG_ReadByte(with_exception);
        gameskill=MSG_ReadByte(with_exception);
        int od_teamplay=MSG_ReadByte(with_exception);
        int od_ctf=MSG_ReadByte(with_exception);

        if(od_ctf)             gametype=gtCTF;
        else if(od_teamplay)   gametype=gtTeamDeathmatch;
        else if(od_deathmatch) gametype=gtDeathmatch;
        else                   gametype=gtCooperative;

        int numcl=numclients<=MAX_PLAYER_NUMB?numclients:MAX_PLAYER_NUMB;
        for(int i=0;i<numcl;i++)
        {
            sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            playertmp[i].name=SetString(tmpbuf);
            playertmp[i].fragcount=fragtmp[i]=(short)MSG_ReadShort(with_exception);
            playertmp[i].ping=MSG_ReadLong(with_exception);
            int team=MSG_ReadByte(with_exception);
            if(team>=0 && team<teamMAX[gmflag])
            {
                playertmp[i].team=team;
                teamscore[team]+=playertmp[i].fragcount;
            }
            else
                playertmp[i].team=teamNONE;
        }
        for(int i=MAX_PLAYER_NUMB;i<numclients;i++)
        {
            MSG_ReadString(with_exception);
            MSG_ReadShort(with_exception);
            MSG_ReadLong(with_exception);
            MSG_ReadByte(with_exception);
        }

        wadcheckenabled=true;
        gameiwadchecksum=SetString(MSG_ReadString(with_exception));
        for(j=0;j<wadmax;j++)
            wadchecksum[j]=SetString(MSG_ReadString(with_exception));
        for(;j<wadnumb;j++) MSG_ReadString(with_exception);   // if > MAX_WAD_NUMB
        wadnumb=wadmax;

        wadurl=SetString(MSG_ReadString(with_exception));
        if(od_ctf || od_teamplay)
        {
            teamscorelimit=MSG_ReadLong(with_exception);
            scoremax=0;
            j=0;
            for(int i=0;i<teamMAX[gmflag];i++)
            {
                bool teamenabled=MSG_ReadByte(with_exception);
                if(!teamenabled) continue;
                teamscore[j]=MSG_ReadLong(with_exception);
                if(scoremax<teamscore[j]) scoremax=teamscore[j];
                teamcolor[j]=teamcolor[i];
                teamname[j]=teamname[i];
                ++j;
            }
            teamnumb=j;
        }
        version=MSG_ReadShort(with_exception);
        email=SetString(MSG_ReadString(with_exception));
        timelimit=MSG_ReadShort(with_exception);
        timeleft=MSG_ReadShort(with_exception);
        fraglimit=MSG_ReadShort(with_exception);

        for(int i=0;i<odfNUMB;i++) odafeatures[i]=MSG_ReadByte();

        for (int i=0;i<numcl;i++)
        {
            int kill=(short)MSG_ReadShort(with_exception);
            if(gametype==gtCooperative)
                playertmp[i].fragcount=fragtmp[i]=kill;
            playertmp[i].deathcount=MSG_ReadShort(with_exception);
            playertmp[i].minutesingame=MSG_ReadShort(with_exception);
        }
        for(int i=MAX_PLAYER_NUMB;i<numclients;i++)
        {
            MSG_ReadShort(with_exception);
            MSG_ReadShort(with_exception);
            MSG_ReadShort(with_exception);
        }

        maxplayers=maxclients;
        numspectators=0;
        int gameversion=0;
        if(MSG_ReadLong(with_exception)==0x01020304) // ver>0.3
        {
            maxplayers=MSG_ReadShort(with_exception);
            for (int i=0;i<numcl;i++)
            {
                playertmp[i].isspectator=MSG_ReadByte(with_exception);
                if(playertmp[i].isspectator) ++numspectators;
            }
            for(int i=MAX_PLAYER_NUMB;i<numclients;i++)
                MSG_ReadByte(with_exception);
        }
        numplayers=numclients-numspectators;
        if(maxplayers==2 && gametype==gtDeathmatch) gametype=gtDuel;
        if(MSG_ReadLong(with_exception)==0x01020305) // ver>0.3
        {
            forcepassword=MSG_ReadShort(with_exception);
            gameversion=MSG_ReadLong(with_exception);
        }
        sprintf(tmpbuf,"%d/%d",gameversion,version);
        sversion=SetString(tmpbuf);

        // 255 limit workaround
        numclients=numcl;
        if(maxclients>MAX_PLAYER_NUMB) maxclients=MAX_PLAYER_NUMB;
        if(numplayers>MAX_PLAYER_NUMB) numplayers=numclients;
    }
// -- ZDaemon packet - new (zdver>0) --------------------
    else if(gmflag==GM_ZDAEMON && proto_zdver)
    {
        acluser=MSG_ReadByte(with_exception);
        version=MSG_ReadShort(with_exception);
        hostname=SetString(MSG_ReadString(with_exception));
        numclients=MSG_ReadByte(with_exception);
        if(numclients<0 || numclients>MAX_PLAYER_NUMB) return false;
        maxclients=MSG_ReadByte(with_exception);
        mapname=SetString(Strlwr(MSG_ReadString(with_exception)));
        wadnumb=MSG_ReadByte(with_exception);
        if(wadnumb<0 || wadnumb>MAX_WAD_NUMB) return false;
        wadcheckenabled=proto_zdver==1?false:true;
        for(j=0;j<wadnumb;j++)
        {
            sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            wadname[j]=SetString(Strlwr(tmpbuf));
            wadoptional[j]=MSG_ReadByte(with_exception);
            if(proto_zdver==1)
            {
                sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
                wadnamealt[j]=SetString(Strlwr(tmpbuf));
            }
            else
            {
                wadchecksum[j]=SetString(ReadChecksum());

                bool i1=true;
                char wa[256]={0}, wc[256]={0};
                sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
                while(tmpbuf[0])
                {

                    if(i1)
                    {
                        strcat(wa,tmpbuf);
                        strcat(wc,ReadChecksum());
                        i1=false;
                    }
                    else
                    {
                        strcat(strcat(wa,"|"),tmpbuf);
                        strcat(strcat(wc,"|"),ReadChecksum());
                    }
                    sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
                }
                wadnamealt[j]=SetString(Strlwr(wa));
                wadaltchecksum[j]=SetString(wc);
            }
        }
        for(;j<MAX_WAD_NUMB;j++)
        {
            wadname[j]=empty_string;
            wadoptional[j]=false;
            wadnamealt[j]=empty_string;
        }
        gametype=MSG_ReadByte(with_exception);
        if(gametype<0 || gametype>=GameTypesNumb[gmflag]) return false;
        gametype=GameTypes[gmflag][gametype];
        gamename=SetString(MSG_ReadString(with_exception));
        gameiwad=SetString(Strlwr(MSG_ReadString(with_exception)));
        if(proto_zdver==1)
            gameiwadalt=SetString(Strlwr(MSG_ReadString(with_exception)));
        else
        {
            gameiwadchecksum=SetString(ReadChecksum());

            bool i1=true;
            char wa[256]={0}, wc[256]={0};
            sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            while(tmpbuf[0])
            {

                if(i1)
                {
                    strcat(wa,tmpbuf);
                    strcat(wc,ReadChecksum());
                    i1=false;
                }
                else
                {
                    strcat(strcat(wa,"|"),tmpbuf);
                    strcat(strcat(wc,"|"),ReadChecksum());
                }
                sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            }
            gameiwadalt=SetString(Strlwr(wa));
            gameiwadaltchecksum=SetString(wc);
        }
        gameskill=MSG_ReadByte(with_exception)+1;
        wadurl=SetString(MSG_ReadString(with_exception));
        email=SetString(MSG_ReadString(with_exception));
        dmflags=MSG_ReadLong(with_exception);
        dmflags2=MSG_ReadLong(with_exception);
        forcepassword=MSG_ReadByte(with_exception);
        acl=MSG_ReadByte(with_exception);
        numplayers=MSG_ReadByte(with_exception);
        maxplayers=MSG_ReadByte(with_exception);
        if(maxplayers==2 && gametype==gtDeathmatch) gametype=gtDuel;
        timelimit=MSG_ReadShort(with_exception);
        timeleft=MSG_ReadShort(with_exception);
        fraglimit=MSG_ReadShort(with_exception);
        float f=MSG_ReadFloat(with_exception);
        gravity=(int)(0.5+100.0*f/800.0);
        f=MSG_ReadFloat(with_exception);
        if((int)(f*256)==1)
            aircontrol=-1;
        else
            aircontrol=(int)(0.5+100.0*f);
        minplayers=MSG_ReadByte(with_exception);
        removebotswhenhumans=MSG_ReadByte(with_exception);
        if(proto_zdver==1)
            voting=MSG_ReadByte(with_exception);
        else
            voting=MSG_ReadLong(with_exception);
        sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
        sversion=SetString(tmpbuf);
        startuptime=MSG_ReadLong(with_exception);
        for (int i=0;i<numclients;i++)
        {
            sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            playertmp[i].name=SetString(tmpbuf);
            playertmp[i].fragcount=fragtmp[i]=(short)MSG_ReadShort(with_exception);
            playertmp[i].deathcount=MSG_ReadShort(with_exception);
            playertmp[i].ping=MSG_ReadShort(with_exception);
            playertmp[i].minutesingame=MSG_ReadShort(with_exception);
            playertmp[i].isbot=MSG_ReadByte(with_exception);
            playertmp[i].isspectator=MSG_ReadByte(with_exception);
            if(playertmp[i].isspectator) fragtmp[i]-=200; // sort them down
            int team=MSG_ReadByte(with_exception);
            playertmp[i].team=(team>=0 && team<teamMAX[gmflag])?team:teamNONE;
            tmpbuf[0]=MSG_ReadByte(with_exception);
            tmpbuf[1]=MSG_ReadByte(with_exception);
            tmpbuf[2]=0;
            playertmp[i].countrycode=SetString(tmpbuf);
        }
        teamnumb=MSG_ReadByte(with_exception);
        if(teamnumb<2 || teamnumb>teamMAX[gmflag]) return false;
        teamscorelimit=MSG_ReadShort(with_exception);
        teamdamage=MSG_ReadFloat(with_exception);
        scoremax=0;
        for (int i=0;i<teamMAX[gmflag];i++)
        {
            sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            teamname[i]=SetString(tmpbuf);
            teamcolor[i]=MSG_ReadLong(with_exception);
            teamscore[i]=(short)MSG_ReadShort(with_exception);
            if(scoremax<teamscore[i]) scoremax=teamscore[i];
        }
        if(proto_zdver>=3)
        {
            maxlives=MSG_ReadShort(with_exception);
            splashfactor=MSG_ReadFloat(with_exception);
        }
    }
// -- ZDaemon packet ------------------------------------
    else if(gmflag==GM_ZDAEMON)
    {
        hostname=SetString(MSG_ReadString(with_exception));
        numclients=MSG_ReadByte(with_exception);
        if(numclients<0 || numclients>MAX_PLAYER_NUMB) return false;
        maxclients=MSG_ReadByte(with_exception);
        mapname=SetString(Strlwr(MSG_ReadString(with_exception)));
        wadnumb=MSG_ReadByte(with_exception);
        if(wadnumb<0 || wadnumb>MAX_WAD_NUMB) return false;
        for(j=0;j<wadnumb;j++)
        {
            sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            wadname[j]=SetString(Strlwr(tmpbuf));
        }
        for(;j<MAX_WAD_NUMB;j++) wadname[j]=empty_string;
        gametype=MSG_ReadByte(with_exception);
        if(gametype<0 || gametype>=GameTypesNumb[gmflag]) return false;
        gametype=GameTypes[gmflag][gametype];
        gamename=SetString(MSG_ReadString(with_exception));
        gameiwad=SetString(Strlwr(MSG_ReadString(with_exception)));
        gameskill=MSG_ReadByte(with_exception)+1;
        wadurl=SetString(MSG_ReadString(with_exception));
        email=SetString(MSG_ReadString(with_exception));
        dmflags=MSG_ReadLong(with_exception);
        dmflags2=MSG_ReadLong(with_exception);
        numclients=MSG_ReadByte(with_exception);
        for (int i=0;i<numclients;i++)
        {
            sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
            playertmp[i].name=SetString(tmpbuf);
            playertmp[i].fragcount=fragtmp[i]=(short)MSG_ReadShort(with_exception);
            playertmp[i].ping=MSG_ReadShort(with_exception);
            playertmp[i].level=MSG_ReadByte(with_exception);
            playertmp[i].minutesingame=MSG_ReadShort(with_exception);
            playertmp[i].isbot=(playertmp[i].ping==0);
        }
        version=MSG_ReadShort(with_exception);
        if(version>100 && MSG_ReadLong(with_exception)==0x01020304) // Extended IDE features
        {
            forcepassword=MSG_ReadByte(with_exception);
            numplayers=MSG_ReadByte(with_exception);
            maxplayers=MSG_ReadByte(with_exception);
            if(maxplayers==2 && gametype==gtDeathmatch) gametype=gtDuel;
            timelimit=MSG_ReadShort(with_exception);
            timeleft=MSG_ReadShort(with_exception);
            fraglimit=MSG_ReadShort(with_exception);
            for (int i=0;i<numclients;i++)
            {
                playertmp[i].isbot=MSG_ReadByte(with_exception);
                playertmp[i].isspectator=MSG_ReadByte(with_exception);
                if(playertmp[i].isspectator) fragtmp[i]-=200; // sort them down
                int team=MSG_ReadByte(with_exception);
                playertmp[i].team=(team>=0 && team<teamMAX[gmflag])?team:teamNONE;
            }
            teamnumb=MSG_ReadByte(with_exception);
            if(teamnumb<2 || teamnumb>teamMAX[gmflag]) return false;
	        teamscorelimit=MSG_ReadShort(with_exception);
            teamdamage=MSG_ReadFloat(with_exception);
            scoremax=0;
            for (int i=0;i<teamMAX[gmflag];i++)
            {
                teamscore[i]=(short)MSG_ReadShort(with_exception);
                if(scoremax<teamscore[i]) scoremax=teamscore[i];
            }
            for(j=0;j<wadnumb;j++) wadoptional[j]=MSG_ReadByte(with_exception);
            if(MSG_ReadLong(with_exception)==0x01020304)
            {
                sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
                sversion=SetString(tmpbuf);
                if(MSG_ReadLong(with_exception)==0x01020304)
                {
                    float f=MSG_ReadFloat(with_exception);
                    gravity=(int)(0.5+100.0*f/800.0);
                    f=MSG_ReadFloat(with_exception);
                    aircontrol=(int)(0.5+100.0*f);
                }
            }
            if(MSG_ReadLong(with_exception)==0x01020304)
            {
                for (int i=0;i<numclients;i++)
                {
                    tmpbuf[0]=MSG_ReadByte(with_exception);
                    tmpbuf[1]=MSG_ReadByte(with_exception);
                    tmpbuf[2]=0;
                    playertmp[i].countrycode=SetString(tmpbuf);
                }
                minplayers=MSG_ReadByte(with_exception);
                removebotswhenhumans=MSG_ReadByte(with_exception);
                startuptime=MSG_ReadLong(with_exception);
            }
        }
        else
        {
            numplayers=numclients;
            maxplayers=maxclients;
        }
    }
// -- Skulltag packet -----------------------------------
    else
    {
        sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
        sversion=SetString(tmpbuf);
        int bits=MSG_ReadLong(with_exception);
        if(bits & SQF_NAME)
            hostname=SetString(MSG_ReadString(with_exception));
        if(bits & SQF_URL)
            wadurl=SetString(MSG_ReadString(with_exception));
        if(bits & SQF_EMAIL)
            email=SetString(MSG_ReadString(with_exception));
        if(bits & SQF_MAPNAME)
            mapname=SetString(Strlwr(MSG_ReadString(with_exception)));
        if(bits & SQF_MAXCLIENTS)
            maxclients=MSG_ReadByte(with_exception);
        if(bits & SQF_MAXPLAYERS)
            maxplayers=MSG_ReadByte(with_exception);
        if(bits & SQF_PWADS)
        {
            wadnumb=MSG_ReadByte(with_exception);
            if(wadnumb<0) return false;
            int wadmax=wadnumb<=MAX_WAD_NUMB?wadnumb:MAX_WAD_NUMB;
            for(j=0;j<wadmax;j++)
            {
                sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
                wadname[j]=SetString(Strlwr(tmpbuf));
            }
            for(;j<MAX_WAD_NUMB;j++) wadname[j]=empty_string;
            for(;j<wadnumb;j++) MSG_ReadString(with_exception);   // if >8
            wadnumb=wadmax;
        }
        if(bits & SQF_GAMETYPE)
        {
            gametype=MSG_ReadByte(with_exception);
            if(gametype<0 || gametype>=GameTypesNumb[gmflag]) return false;
            gametype=GameTypes[gmflag][gametype];
            gt_instagib=MSG_ReadByte(with_exception);
            gt_buckshot=MSG_ReadByte(with_exception);
        }
        teamflag=IsTeamGame(gametype);

        if(bits & SQF_GAMENAME)
            gamename=SetString(MSG_ReadString(with_exception));
        if(bits & SQF_IWAD)
            gameiwad=SetString(Strlwr(MSG_ReadString(with_exception)));
        if(bits & SQF_FORCEPASSWORD)
            forcepassword=MSG_ReadByte(with_exception);
        if(bits & SQF_FORCEJOINPASSWORD)
            forcejoinpassword=MSG_ReadByte(with_exception);
        if(bits & SQF_GAMESKILL)
            gameskill=MSG_ReadByte(with_exception)+1;
        if(bits & SQF_BOTSKILL)
            botskill=MSG_ReadByte(with_exception);
        if(bits & SQF_DMFLAGS)
        {
            dmflags=MSG_ReadLong(with_exception);
            dmflags2=MSG_ReadLong(with_exception);
            compatflags=MSG_ReadLong(with_exception);
        }
        if(bits & SQF_LIMITS)
        {
            fraglimit=MSG_ReadShort(with_exception);
            timelimit=MSG_ReadShort(with_exception);
            if(timelimit)
                timeleft=MSG_ReadShort(with_exception);
            duellimit=MSG_ReadShort(with_exception);
            pointlimit=MSG_ReadShort(with_exception);
            winlimit=MSG_ReadShort(with_exception);
            if(gametype==gtLMS) fraglimit=winlimit;
        }

        if(bits & SQF_TEAMDAMAGE)
            teamdamage=MSG_ReadFloat(with_exception);

        scoremax=0;
        if(bits & SQF_TEAMSCORES)
        {
            for (int i=0;i<2;i++)   // !!! old ST 0.97d2 team number !!!
            {
                teamscore[i]=(short)MSG_ReadShort(with_exception);
                if(scoremax<teamscore[i]) scoremax=teamscore[i];
            }
        }

        if(bits & SQF_NUMPLAYERS)
        {
            numclients=MSG_ReadByte(with_exception);
            if(numclients<0 || numclients>MAX_PLAYER_NUMB) return false;
        }
        numspectators=0;
        if(bits & SQF_PLAYERDATA)
        {
            for (int i=0;i<numclients;i++)
            {
                sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
                playertmp[i].namecolor=SetString(StripColorName(tmpbuf));
                playertmp[i].name=SetString(tmpbuf);
                playertmp[i].fragcount=fragtmp[i]=(short)MSG_ReadShort(with_exception);
                playertmp[i].ping=MSG_ReadShort(with_exception);
                playertmp[i].isspectator=MSG_ReadByte(with_exception);
                playertmp[i].isbot=MSG_ReadByte(with_exception);
                if(playertmp[i].isspectator)
                {
                    fragtmp[i]-=200; // sort down
                    numspectators++;
                }
                if(teamflag)
                {
                    int team=MSG_ReadByte(with_exception);
                    playertmp[i].team=(team>=0 && team<teamMAX[gmflag])?team:teamNONE;
                }
                playertmp[i].minutesingame=MSG_ReadByte(with_exception);
            }
        }
        numplayers=numclients-numspectators;
//------------ SQF_TEAMINFO
        teamnumb=2;
        int teamnumb1=teamnumb;
        if(bits & SQF_TEAMINFO_NUMBER)
            teamnumb1=MSG_ReadByte(with_exception);
        if(teamnumb1<0) return false;
        teamnumb=(teamnumb1>teamMAX[gmflag])?teamMAX[gmflag]:teamnumb1;

        if(bits & SQF_TEAMINFO_NAME)
        {
            for(j=0;j<teamnumb;++j)
            {
                sprintf(tmpbuf,"%.*s",MAXTMPBUF,MSG_ReadString(with_exception));
                teamname[j]=SetString(tmpbuf);
            }
            for(;j<teamnumb1;++j) MSG_ReadString(with_exception);     // if teamnumb1>max
        }

        if(bits & SQF_TEAMINFO_COLOR)
        {
            for(j=0;j<teamnumb;++j)
                teamcolor[j]=MSG_ReadLong(with_exception);
            for(;j<teamnumb1;++j) MSG_ReadLong(with_exception);       // if teamnumb1>max
        }

        if(bits & SQF_TEAMINFO_SCORE)
        {
            scoremax=0;
            for(j=0;j<teamnumb;++j)
            {
                teamscore[j]=(short)MSG_ReadShort(with_exception);
                if(scoremax<teamscore[j]) scoremax=teamscore[j];
            }
            for(;j<teamnumb1;++j) MSG_ReadShort(with_exception);      // if teamnumb1>max
        }
//------------
        teamscorelimit=(gametype==gtTeamDeathmatch)?fraglimit:pointlimit;
        if(gametype==gtTeamLMS) teamscorelimit=winlimit;
//------------SQF_TESTING_SERVER
        if(bits & SQF_TESTING_SERVER)
        {
            sttestenabled=MSG_ReadByte(with_exception);
            sttestclient=SetString(MSG_ReadString(with_exception));
        }
    }
// -- Common things -------------------------------------
    // if something is wrong with packet/protocol...
    if(    maxclients>MAX_PLAYER_NUMB
        || maxplayers>MAX_PLAYER_NUMB
        || numplayers>MAX_PLAYER_NUMB
        || minplayers>MAX_PLAYER_NUMB
        || gameskill>5
        || botskill>4
       ) return false;

    if(timeleft>timelimit) timeleft=timelimit;
    if(maxplayers>maxclients) maxplayers=maxclients;

    teamscoreleft=teamscorelimit-scoremax;
    if(teamscoreleft<0) teamscoreleft=teamscorelimit;

    teamflag=IsTeamGame(gametype);
    if(teamflag) // sort by teamscore first
        for(int i=0;i<numclients;i++)
        {
            if(playertmp[i].team==teamNONE) continue;
            fragtmp[i]+=(teamnumb-playertmp[i].team)*1000
                       +teamscore[playertmp[i].team]*10000;
        }
    NumbSort(fragtmp,numclients,indtmp,DOWN);
    numbots=numspectators=0;
    int fragmax=0;
    for(int i=0;i<numclients;i++)
    {
        player[i].name=playertmp[indtmp[i]].name;
        player[i].namecolor=playertmp[indtmp[i]].namecolor;
        player[i].fragcount=playertmp[indtmp[i]].fragcount;
        if(fragmax<player[i].fragcount) fragmax=player[i].fragcount;
        player[i].deathcount=playertmp[indtmp[i]].deathcount;
        player[i].ping=playertmp[indtmp[i]].ping;
        player[i].isbot=playertmp[indtmp[i]].isbot;
        if(player[i].isbot) numbots++;
        player[i].isspectator=playertmp[indtmp[i]].isspectator;
        if(player[i].isspectator) numspectators++;
        player[i].level=playertmp[indtmp[i]].level;
        player[i].minutesingame=playertmp[indtmp[i]].minutesingame;
        player[i].team=playertmp[indtmp[i]].team;
        player[i].countrycode=playertmp[indtmp[i]].countrycode;
    }
    fragleft=fraglimit-fragmax;
    if(fragleft<0) fragleft=0;//fraglimit;
// end try_____________________________________________
    }
    catch(int) { return false; }
    return true;
}
//----------------------------------------------------------------------------
// Format IP-address for better sorting:
//  111.11.1.11:101 -> 111. 11.  1. 11.:  101
//-------------------------------------------
char* FormatAddress(char* adr,char* fadr)
{
    char* p2=fadr+MAXADR;
    char* p1=adr+strlen(adr)-1;
    *p2--=0;
    if(*p1=='c' || *p1=='t') *p2--=*p1--;
    else *p2--=0;
    int k;
    for(k=0;k<5;k++)
        if(*p1!=':') *p2--=*p1--;
        else *p2--=32;
    *p2--=*p1--;          // ':'
    for(int l=0;l<3;l++)
    {
        for(k=0;k<3;k++)
            if(*p1!='.') *p2--=*p1--;
            else *p2--=32;
        *p2--=*p1--;      // '.'
    }
    for(k=0;k<3;k++)
        if(p1>=adr) *p2--=*p1--;
        else *p2--=32;

    return fadr;
}
//----------------------------------------------------------------------------
// Current time in millisecond
//----------------------------
#ifdef __WIN32__
int I_GetTime (void)
{
    return timeGetTime();
}
#else
int I_GetTime(void)
{
    struct timeval tv;
    struct timezone tz;
    long tm;

    gettimeofday(&tv,&tz);
    tm=tv.tv_sec*1000+tv.tv_usec/1000;
    return tm;
}
#endif
//----------------------------------------------------------------------------
void QueryServer(char* svadr,int gmflag,int challenge,bool master)
{
    netadr_t serveraddr;
    buf_t message;
    byte message_buffer[MAX_UDP_PACKET];

    message.data = message_buffer;
    message.maxsize = sizeof(message_buffer);

    NET_StringToAdr(svadr,&serveraddr);

    SZ_Clear(&message);
    MSG_WriteLong(&message,challenge);
    if(!master)
    {
        if(gmflag==GM_ZDAEMON)
        {
            MSG_WriteLong(&message,Lnch2SrvProtocol_zdver);
            MSG_WriteLong(&message,Lnch2SrvVersion_zdver);
        }
        else if(gmflag==GM_SKULLTAG)
            MSG_WriteLong(&message,SQF_ALL);
        MSG_WriteLong(&message,I_GetTime());
        if(gmflag==GM_ZDAEMON)
            MSG_WriteString(&message,"");
    }
    else if(gmflag==GM_SKULLTAG)
        MSG_WriteShort(&message,STMasterServerVersion);

    NET_SendPacket(message.cursize, message.data, serveraddr,gmflag);
    LastQueriedServer=serveraddr;
}
//----------------------------------------------------------------------------
// Get list of servers from master server
//----------------------------------------
int GetMasterInfo(char* servers[],int gmflag)
{
    if(gmflag==GM_ZDAEMON)
        return ZDGetMasterServerList(servers);
    else if(gmflag==GM_SKULLTAG)
        return STGetMasterServerList(servers);
    else if(gmflag==GM_ODAMEX)
        return ODGetMasterServerList(servers);
    else
        return 0;
}
//----------------------------------------------------------------------------
int ODGetMasterServerList(char* servers[])
{
    GmMaster=false;
    int n=0;
    int numtry=Numtry;
    NET_Clear();
    while(numtry--)
    {
        if(MustTerminated) return 0;
        QueryServer(MasterSv[GM_ODAMEX],GM_ODAMEX,ODLnch2MstChallenge,true);
        ShowNetStep();

        int pktready=I_DoSelect();
        if(pktready==SOCKET_ERROR) return 0;
        if(   !pktready
           || !NET_GetPacket()
           || !NET_CompareAdr(LastQueriedServer,net_from)) continue;

        if(MSG_ReadLong() != ODMst2LnchChallenge) return 0;

        GmMaster=true;
        char tmpaddress[MAXADR+1];
        int svnumb=MSG_ReadShort();
        for(int i=0;i<svnumb;i++)
        {
            if(!MSG_ReadAdr(tmpaddress)) continue;
            servers[n+1]=Stpcpy(servers[n],tmpaddress)+1;
            if(++n==MAXSERVERS-1) break;
        }
        return n;
    }
    return 0;
}
//----------------------------------------------------------------------------
int STGetMasterServerList(char* servers[])
{
    GmMaster=false;
    int n;
    int pktnumb=0;
    int totnumb=0;
    int curnumb;
    int svnumb=0;
    int numtry=Numtry;
    char tmpaddress[MAXADR+1];
    NET_Clear();
    while(numtry--)
    {
        n=0;
        if(MustTerminated) break;
        if(!pktnumb)
            QueryServer(MasterSv[GM_SKULLTAG],GM_SKULLTAG,STLnch2MstChallenge,true);
        ShowNetStep();

        int pktready=I_DoSelect();
        if(pktready==SOCKET_ERROR) break;
        if(!pktready)   // timeout (1 sec)
        {
            pktnumb=0;
            totnumb=0;
            svnumb=0;
            continue;
        }

        if(!NET_GetPacket() || !NET_CompareAdr(LastQueriedServer,net_from))
            continue;

        NET_HuffDecodePacket();
        n=MSG_ReadLong();
        if(n != ST_MASTER_BEGINLISTPART) break;

        try
        {
            curnumb=MSG_ReadByte(with_exception);
            MSG_ReadByte(with_exception);     // MSC_SERVERBLOCK - not used
            pktnumb++;
            numtry++;

            while((n=MSG_ReadByte(with_exception))!=0)
            {
                unsigned ip[4];
                for(int i=0;i<4;i++) ip[i]=MSG_ReadByte(with_exception);
                for(int i=0;i<n && svnumb<MAXSERVERS-1;i++,svnumb++)
                {
                    sprintf(tmpaddress,"%u.%u.%u.%u:%u"
                        ,ip[0],ip[1],ip[2],ip[3],MSG_ReadShort(with_exception));
                    servers[svnumb+1]=Stpcpy(servers[svnumb],tmpaddress)+1;
                }
            }

            if(MSG_ReadByte(with_exception)==ST_MASTER_ENDLIST)
                totnumb=curnumb+1;
        }
        catch(int) { n=IDE_BAD_PACKET; break; }

        if(pktnumb==totnumb)
        {
            GmMaster=true;
            n=svnumb;
            break;
        }
    }
    return n;
}
//----------------------------------------------------------------------------
// Put sort info to index array
// Dir - sort direction
//------------------------------
void StringSort(char** s,int numb,int* ind,int Dir)
{
    int i,j;

    for(i=0;i<numb;i++) ind[i]=i;

    for(i=numb;i;i--)
        for(j=1;j<i;j++)
            if(strcmp(s[ind[j-1]],s[ind[j]])>0)
                exchange(ind[j],ind[j-1]);

    if(Dir==DOWN)
        for(i=0;i<numb/2;i++)
            exchange(ind[i],ind[numb-i-1]);
}
//----------------------------------------------------------------------------
void NumbSort(int* n,int numb,int* ind,int Dir)
{
    int i,j;

    for(i=0;i<numb;i++) ind[i]=i;

    for(i=numb;i;i--)
        for(j=1;j<i;j++)
            if(n[ind[j-1]]>n[ind[j]])
                exchange(ind[j],ind[j-1]);

    if(Dir==DOWN)
        for(i=0;i<numb/2;i++)
            exchange(ind[i],ind[numb-i-1]);
}
//---------------------------------------------------------------------------
bool IsTeamGame(int gametype)
{
    return  gametype==gtTeamDeathmatch
         || gametype==gtTeamPossession
         || gametype==gtTeamLMS
         || gametype==gtTeamgame
         || gametype==gtCTF
         || gametype==gtOneFlagCTF
         || gametype==gtSkulltag
         || gametype==gtDomination;
}
//----------------------------------------------------------------------------
// Strip colors from player names (Skulltag)
//----------------------------------------
#define CCODE 0x1c
// - == B color
// + == A color
char* StripColorName(char* name)
{
    static char colors[MAXTMPBUF+1];
    char newname[MAXTMPBUF+1];
    int l=strlen(name);
    char curcolor='@';  // will be first index, i.e.: i='@'-'A'
    int i=0,j=0;
    char c;
    bool colorpresent=false;
    while(i<l)
    {
        if((unsigned char)name[i]==CCODE)
        {
            if(i>=l-2) break; //too few symbols in the end
            if(name[++i]==CCODE) continue; // next one or two sym.
            if(name[++i]==CCODE) continue; // are CCODE - ignore them
            c=name[i-1];
            if(c>='A' && c<='U')      colors[j]=c;
            else if(c>='a' && c<='u') colors[j]=c-32;
            else if(c=='-')           colors[j]='B';
            else if(c=='+')           colors[j]='A';
            else                      colors[j]=curcolor;
            curcolor=colors[j];
            newname[j++]=name[i++];
            colorpresent=true;
        }
        else
        {
            colors[j]=curcolor;
            newname[j++]=name[i++];
        }
    }
    newname[j]=0;
    colors[j]=0;
    if(colorpresent)
        for(int i=0;i<j;i++)
            if(colors[i]=='@') colors[i]='D';
    strcpy(name,newname);
    return colors;
}
//----------------------------------------------------------------------------

