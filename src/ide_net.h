#ifndef __IDE_NET_H__
#define __IDE_NET_H__

#if defined _MSC_VER
#define __WIN32__
#endif

#ifdef  __WIN32__
 #ifdef __IDE_DLL__
   #define DLL_EXP __declspec(dllexport) __stdcall
 #else
   #define DLL_EXP __declspec(dllimport) __stdcall
 #endif
#else
  #define DLL_EXP
#endif

//----------------------------------------------------------------------------
typedef struct {
    char* name;         // player name
    char* namecolor;    // player name colors (ST)
    int fragcount;      // player frags
    int deathcount;     // player deaths (OD,ZD>1.08.05)
    int ping;           // player ping
    int minutesingame;  // player time in game
    bool isbot;         // is player bot (ZD,ST)
    bool isspectator;   // is player spectator
    int team;           // player team number
    char* countrycode;  // player country code, 2 symbols: RU,US,... (ZD)
} PlayerData;

typedef struct {
    int gmflag;         // 0-OD, 1-ZD, 2-ST
    int result;         // connection status (SVMSG_*, in ide_const.h)
    int ping;           // server ping
    char address[MAXADR+1]; // server address:port
    char* hostname;     // server name
    char* wadurl;       // PWADs url
    char* email;        // server email
    char* mapname;      // map name
    char* gameiwad;     // game IWAD
    char* gameiwadalt;  // alternative IWAD (ZD)
    int wadnumb;        // number of additional pwads
    char* wadname[MAX_WAD_NUMB]; // names of pwads
    bool wadoptional[MAX_WAD_NUMB]; // true if pwad is optional (ZD)
    char* wadnamealt[MAX_WAD_NUMB]; // alternative PWADs (ZD)
    bool wadcheckenabled;            // server sends wad checksums (md5) (OD,ZD)
    char* gameiwadchecksum;          // IWAD checksum (OD,ZD)
    char* gameiwadaltchecksum;       // alternative IWAD checksums (ZD)
    char* wadchecksum[MAX_WAD_NUMB]; // PWAD checksums (OD,ZD)
    char* wadaltchecksum[MAX_WAD_NUMB]; // alternative PWAD checksums (ZD)
    int numclients;     // number of clients
    int maxclients;     // max. number of clients
    int numplayers;     // number of players w/o spectators
    int maxplayers;     // max. number of players can join
    int numbots;        // number of bots (ZD,ST)
    int botskill;       // bot skill (ST)
    int numspectators;  // number of spectators
    int gameskill;      // skill (1-5)
    int gametype;       // game type
    bool gt_instagib;   // game type modifier - instagib (railgun only) (ST)
    bool gt_buckshot;   // game type modifier - buckshot (SSG only) (ST)
    char* gamename;     // game name: Doom, Doom II, etc..
    long dmflags;       // dmflags (ZD,ST)
    long dmflags2;      // dmflags2 (ZD,ST)
    long compatflags;   // compatflags (ST)
    bool forcepassword; // is server password-protected
    bool forcejoinpassword; // is server require password to join game (ST)
    int timelimit;      // timelimit
    int timeleft;       // time left to level end
    int fraglimit;      // fraglimit
    int fragleft;       // frags left to level end
    int pointlimit;     // pointlimit (ST teamgames only)
    int duellimit;      // duellimit (ST, duels)
    int winlimit;       // winlimit (ST, LMS mode)
    int minplayers;     // add temporary bots (max = minplayers-1) (ZD)
    bool removebotswhenhumans; // remove all bots when 2nd player join(ZD)
    int teamscorelimit; // max. teamscore for teamgames
    int teamscoreleft;  // teamscore left to end of level
    int teamnumb;       // number of teams allowed
    int teamscore[teamNUMB]; // current teamscores
    int teamcolor[teamNUMB]; // custom team colors (ZD)
    char* teamname[teamNUMB];// custom team names (ZD)
    double teamdamage;  // damage to teammates (0-1) (ZD,ST)
    int version;        // server program version - integer (OD,ZD)
    char* sversion;     // server program version - string
    double gravity;     // gravity (0 - 800) (ZD,ST)
    double aircontrol;  // air control (0 - 1) (ZD,ST)
    bool odafeatures[odfNUMB]; // Odamex setup flags (OD)
    int startuptime;    // server startup time (ZD)
    bool acl;           // is server invite-only (ZD)
    int acluser;        // is user invited to play on this server (ZD)
    int voting;         // does server enable voting (ZD)
    bool sttestenabled; // server run special "testing" version (ST)
    char* sttestclient; // address of testing binary (partial) (ST)
    int maxlives;       // maxlives in survival game mode (ZD)
    float splashfactor; // if <0, rockets kick you back, but don't hurt you (ZD)

    PlayerData player[MAX_PLAYER_NUMB];

// internal
    char buf[MAX_UDP_PACKET];
} ServerData;

typedef void (*ShowStepFcn)();

//----------------------------------------------------------------------------
extern "C" {
int  DLL_EXP IDE_GetVersion(void);
bool DLL_EXP IDE_Init(void);
void DLL_EXP IDE_Close(void);
int  DLL_EXP IDE_GetLocalPort(void);
void DLL_EXP IDE_SetLocalPort(int port);
void DLL_EXP IDE_GetMasterAddress(int gmflag,char* s);
void DLL_EXP IDE_SetMasterAddress(int gmflag,char* s);
bool DLL_EXP IDE_GetWarningMessage(char* msg);
void DLL_EXP IDE_SetShowStep(ShowStepFcn fn);
void DLL_EXP IDE_MustTerminated(bool m);

int  DLL_EXP IDE_GetMasterList(int gmflag);
bool DLL_EXP IDE_GetMasterListServer(int i,char* sv);
bool DLL_EXP IDE_GetServerInfo(char* address,int gmflag,ServerData* svi);
bool DLL_EXP IDE_GetServerGroup(ServerData *svi[],int svnumb,int senddelay,int recvdelay);
}
//----------------------------------------------------------------------------

#endif
