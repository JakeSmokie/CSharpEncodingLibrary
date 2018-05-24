#ifndef IDE_Const_H
#define IDE_Const_H

#define MAX_UDP_PACKET  8192
#define MAXADR          22
#define MAXSERVERS      1000
#define MAXSERVADR      ( ( MAXADR+1 ) * MAXSERVERS )
#define MAX_PLAYER_NUMB 32
#define MAX_WAD_NUMB    32

#define SVMSG_OK        0
#define SVMSG_INIT      1
#define SVMSG_BADCL     2
#define SVMSG_NCON      3
#define SVMSG_MAXBUF    4
#define SVMSG_ST_BAN    5
#define SVMSG_ST_IGNORE 6
#define SVMSG_BADPKT    7

// number and types of games: odamex, zdaemon, skulltag
#define GM_NUMB     3
#define GM_ODAMEX   0
#define GM_ZDAEMON  1
#define GM_SKULLTAG 2

#define IDE_NOT_INITIALIZED  -2
#define IDE_NOT_CONNECTED    -1
#define IDE_ST_BAN           -3
#define IDE_ST_IGNORE        -4
#define IDE_BAD_PACKET       -5

// zdaemon score limitations - bond
#define ZD_MAX_NAME  32
#define ZD_MAX_EXTRA 256
#define ZDMAXPLAYERS 255

#define IDE_ZD_OK                0
#define IDE_ZD_NEED_UPDATE       1
#define IDE_ZD_ACCOUNT_NOT_EXIST 2
#define IDE_ZD_BAD_PASSWORD      3
#define IDE_ZD_ACCOUNT_EXIST     4

enum GameTypes {
 gtCooperative
,gtDeathmatch
,gtTeamDeathmatch
,gtCTF
,gtDuel
,gtSurvival
,gtInvasion
,gtTerminator
,gtLMS
,gtTeamLMS
,gtPossession
,gtTeamPossession
,gtTeamgame
,gtOneFlagCTF
,gtSkulltag
,gtDomination
,gtNUMB
};

enum TeamType {
 teamRED
,teamBLUE
,teamGREEN
,teamWHITE
,teamGOLD
,teamNONE
,teamNUMB
};

enum OdamexFeatureFlags {
 odfItemsrespawn
,odfWeaponstay
,odfFriendlyfire
,odfAllowexit
,odfInfiniteammo
,odfNomonsters
,odfMonstersrespawn
,odfFastmonsters
,odfAllowjump
,odfAllowfreelook
,odfWaddownload
,odfEmptyreset
,odfCleanmaps
,odfFragexitswitch
,odfNUMB
};

#endif
