//----------------------------------------------------------------------------
// Class ServerInfo definition
//----------------------------------------------------------------------------
#ifndef ide_svinfoH
#define ide_svinfoH

#define UP   0
#define DOWN 1

#include "ide_const.h"
//----------------------------------------------------------------------------
typedef struct {
    char* name;
    char* namecolor;
    int fragcount;
    int deathcount;
    int ping;
    int level;
    int minutesingame;
    bool isbot;
    bool isspectator;
    int team;
    char* countrycode;
} PlayerInfo;

class ServerInfo
{
public:
    bool connected;
    int gmflag;     // Odamex, ZDaemon, Skulltag
    int result;
    char address[MAXADR+1];
    int iping;
    char* hostname;
    int numclients;
    int maxclients;
    char* mapname;
    int wadnumb;
    char* wadname[MAX_WAD_NUMB];
    bool wadoptional[MAX_WAD_NUMB];
    int gameskill;
    int gametype;
    bool gt_instagib;
    bool gt_buckshot;
    char* gamename;
    char* gameiwad;
    long dmflags;
    long dmflags2;
    long compatflags; //skulltag
    char* wadurl;
    char* email;
    int version;
    char* sversion;
    bool forcepassword;
    bool forcejoinpassword;
    int numplayers;
    int maxplayers;
    int timelimit;
    int timeleft;
    int fraglimit;
    int fragleft;
    int pointlimit; //skulltag
    int numbots;
    int numspectators;
    int botskill;
    bool teamflag;
    int teamscorelimit;
    int teamscoreleft;
    int teamnumb;
    float teamdamage;
    int teamscore[teamNUMB];
    float gravity;
    float aircontrol;
    int minplayers;
    bool removebotswhenhumans;
    int duellimit;
    int winlimit;

    netadr_t naddress;

    bool odafeatures[odfNUMB];

    int proto_zdver;
    int startuptime;
    bool acl;
    int acluser;
    int voting;
    char* gameiwadalt;
    char* wadnamealt[MAX_WAD_NUMB];
    int teamcolor[teamNUMB];
    char* teamname[teamNUMB];

    int sendtime;
    int recvtime;

    bool sttestenabled;
    char* sttestclient;

    bool wadcheckenabled;
    char* gameiwadchecksum;
    char* gameiwadaltchecksum;
    char* wadchecksum[MAX_WAD_NUMB];
    char* wadaltchecksum[MAX_WAD_NUMB];
    int maxlives;
    float splashfactor;

    PlayerInfo player[MAX_PLAYER_NUMB];

    ServerInfo();
    ServerInfo(char* adr,int gmflag,bool queryserver=true);
    void FillInfo(void);
    void ReadPacket(void);

private:
    char buf[MAX_UDP_PACKET];
    int cursize;

    void Init(void);
    void InitPacket(void);
    void InitPlayer(PlayerInfo* player,int n);
    void InitTeams();
    bool ParsePacket(void);
    char* SetString(char*);
    char* ReadChecksum();
};
//----------------------------------------------------------------------------
void QueryServer(char* svadr,int gmflag,int challenge,bool master=false);
int GetMasterInfo(char* servers[],int gmflag);
int STGetMasterServerList(char* servers[]);
int ODGetMasterServerList(char* servers[]);
inline void exchange(int &a,int &b){register int tmp=a;a=b;b=tmp;}
char* FormatAddress(char* adr,char* fadr);
void UpdateZDaemonVersion(void);
int I_GetTime (void);
void StringSort(char** s,int numb,int* ind,int Dir=UP);
void NumbSort(int* n,int numb,int* ind,int Dir);
int TeamIndex(int gmflag,int team);
bool IsTeamGame(int gametype);
char* StripColorName(char* name);
//----------------------------------------------------------------------------

#endif
