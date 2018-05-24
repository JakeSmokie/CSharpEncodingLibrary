#ifndef ide_netbasicH
#define ide_netbasicH
//---------------------------------------------------------------------------
typedef unsigned char byte;

typedef struct
{
   byte    ip[4];
   unsigned short  port;
   unsigned short  pad;
} netadr_t;

typedef struct buf_s
{
   byte    *data;
   int      maxsize;
   int      cursize;
   bool     overflowed;  // set to true if the buffer size failed
} buf_t;

extern netadr_t net_from;  // address of who sent the packet
//---------------------------------------------------------------------------
void CloseNetwork(void);
bool InitNetwork(void);
int I_DoSelect(void);
void NET_Clear(void);

void SockadrToNetadr(struct sockaddr_in *s, netadr_t *a);
void NetadrToSockadr(netadr_t *a, struct sockaddr_in *s);
char *NET_AdrToString(netadr_t a);
bool NET_StringToAdr(char *s, netadr_t *a);
bool NET_CompareAdr(netadr_t a,netadr_t b);

int  NET_GetPacket(void);
void NET_SendPacket(int length, byte *data, netadr_t to,int gm);
//int NET_ReadPacket(void);
void NET_HuffDecodePacket(void);

void SZ_Clear(buf_t *buf);
byte *SZ_GetSpace(buf_t *buf, int length);
void SZ_Write(buf_t *b, void *data, int length);

void MSG_WriteByte(buf_t *b, int c);
void MSG_WriteShort(buf_t *b, int c);
void MSG_WriteLong(buf_t *b, int c);
void MSG_WriteString(buf_t *b, char *s);
void MSG_WriteFloat(buf_t *b, float f);

int MSG_ReadByte(bool do_exception=false);
int MSG_ReadShort(bool do_exception=false);
int MSG_ReadLong(bool do_exception=false);
char *MSG_ReadString(bool do_exception=false);
float MSG_ReadFloat(bool do_exception=false);

bool MSG_ReadAdr(char* sadr);

// ide_dll addon - vc & unix
char* Strlwr(char* s);
char* Stpcpy(char* s1,char* s2);
//---------------------------------------------------------------------------
#endif
