//---------------------------------------------------------------------------
#if defined _MSC_VER
 #define __WIN32__
 #define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <sys/ioctl.h>
 #include <sys/time.h>
 #include <unistd.h>
 #include <netdb.h>
 #include <string.h>
#endif

#ifdef __WIN32__
 typedef int socklen_t;
#else
 typedef int SOCKET;
 #define SOCKET_ERROR -1
 #define INVALID_SOCKET -1
 #define closesocket close
 #define ioctlsocket ioctl
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ide_const.h"
#include "ide_huffman.h"
#include "ide_netbasic.h"
//--------------------------------------------------------------------------
int         net_socket;
netadr_t    net_from;
buf_t       net_message;
int         msg_readcount;
int         msg_badread;

byte* net_message_buffer;
#define NETMESSAGEBUFFERSIZE MAX_UDP_PACKET

byte* huffbuff;
#define HUFFBUFFSIZE 65536

extern int LocalPort;
//--------------------------------------------------------------------------
// Socket functions
//--------------------------------------------------------------------------
int BindToLocalPort(SOCKET s, u_short port)
{
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(port);

    return bind(s,(sockaddr *)&address,sizeof(address));
}
//--------------------------------------------------------------------------
void CloseNetwork(void)
{
    delete[] net_message_buffer;
    delete[] huffbuff;
    closesocket(net_socket);
#ifdef __WIN32__
    WSACleanup();
#endif
}
//--------------------------------------------------------------------------
bool InitNetwork(void)
{
#ifdef __WIN32__
    WSADATA wsad;
    if(WSAStartup(0x0101,&wsad)) return false;
#endif

    unsigned long t=1;
    if( (net_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP))==INVALID_SOCKET
      || BindToLocalPort(net_socket,LocalPort)
      || ioctlsocket(net_socket,FIONBIO,&t))
        return false;

    net_message_buffer=new byte[NETMESSAGEBUFFERSIZE];
    net_message.data = net_message_buffer;
    net_message.maxsize = NETMESSAGEBUFFERSIZE;
    SZ_Clear(&net_message);

    huffbuff=new byte[HUFFBUFFSIZE];
    if(!HuffInit())
    {
        CloseNetwork();
        return false;
    }

    return true;
}
//--------------------------------------------------------------------------
int I_DoSelect(void)
{
    struct timeval timeout;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET((u_int)net_socket, &fdset);
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    return select(net_socket+1, &fdset, NULL, NULL, &timeout);
}
//--------------------------------------------------------------------------
// netaddress functions
//--------------------------------------------------------------------------
void SockadrToNetadr(struct sockaddr_in *s, netadr_t *a)
{
    *(int *)&a->ip = *(int *)&s->sin_addr;
    a->port = s->sin_port;
}
//--------------------------------------------------------------------------
void NetadrToSockadr(netadr_t *a, struct sockaddr_in *s)
{
    memset(s,0,sizeof(*s));
    s->sin_family = AF_INET;

    *(int *)&s->sin_addr = *(int *)&a->ip;
    s->sin_port = a->port;
}
//--------------------------------------------------------------------------
char *NET_AdrToString(netadr_t a)
{
    static char s[64];
    sprintf(s,"%i.%i.%i.%i:%i",a.ip[0],a.ip[1],a.ip[2],a.ip[3],ntohs(a.port));
    return s;
}
//--------------------------------------------------------------------------
bool NET_StringToAdr(char *s, netadr_t *a)
{
    struct hostent *h;
    struct sockaddr_in sadr;
    char *colon;
    char copy[128];
    bool isdigit;

    memset(&sadr,0,sizeof(sadr));
    sadr.sin_family = AF_INET;
    sadr.sin_port = 0;

    strcpy(copy,s);
    // strip off a trailing :port if present
    isdigit=true;
    for(colon = copy ; *colon ; colon++)
        if (*colon == ':')
        {
            *colon = 0;
            sadr.sin_port = htons(atoi(colon+1));
        }
        else if((*colon<'0' || *colon>'9') && *colon!='.')
            isdigit=false;

    if(isdigit)
        *(int *)&sadr.sin_addr = inet_addr(copy);
    else
    {
        if((h = gethostbyname(copy)) == NULL) return false;
        *(int *)&sadr.sin_addr = *(int *)h->h_addr_list[0];
    }

    SockadrToNetadr(&sadr, a);
    return true;
}
//--------------------------------------------------------------------------
bool NET_CompareAdr(netadr_t a,netadr_t b)
{
    if( a.ip[0]==b.ip[0] &&
        a.ip[1]==b.ip[1] &&
        a.ip[2]==b.ip[2] &&
        a.ip[3]==b.ip[3] &&
        a.port==b.port)
        return true;

        return false;
}
//--------------------------------------------------------------------------
// Send/Receive functions
//--------------------------------------------------------------------------
int NET_GetPacket(void)
{
    int ret;
    struct sockaddr_in from;
    socklen_t fromlen;

    fromlen=sizeof(from);
    ret=recvfrom(net_socket,(char *)net_message_buffer,NETMESSAGEBUFFERSIZE,0
                ,(struct sockaddr *)&from,&fromlen);

    if(ret == -1) return 0;

    net_message.cursize = ret;
    SockadrToNetadr(&from, &net_from);

    msg_readcount = 0;
    msg_badread = false;

    return ret;
}
//--------------------------------------------------------------------------
void NET_SendPacket(int length, byte *data, netadr_t to,int gmflag)
{
    int outlen;
    struct sockaddr_in  addr;

    if(gmflag==GM_SKULLTAG)
    {
        HuffEncode((unsigned char *)data, huffbuff, length, &outlen);
        data=huffbuff;
        length=outlen;
    }
    NetadrToSockadr (&to, &addr);
    sendto(net_socket,(const char*)data,length,0,(struct sockaddr *)&addr,sizeof(addr));
}
//--------------------------------------------------------------------------
// netbuffer read/write functions
//---------------------------------------------------------------------------
void SZ_Clear(buf_t *buf)
{
    buf->cursize = 0;
    buf->overflowed = false;
}
//--------------------------------------------------------------------------
byte *SZ_GetSpace(buf_t *buf, int length)
{
    byte *data;

    if(buf->cursize+length > buf->maxsize)
    {
        SZ_Clear(buf);
        buf->overflowed = true;
    }

    data = buf->data + buf->cursize;
    buf->cursize += length;
    return data;
}
//--------------------------------------------------------------------------
void SZ_Write(buf_t *b, void *data, int length)
{
    memcpy(SZ_GetSpace(b, length), data, length);
}
//--------------------------------------------------------------------------
void MSG_WriteByte(buf_t *b, int c)
{
    byte *buf;
    buf = SZ_GetSpace(b, 1);
    buf[0] = c;
}
//--------------------------------------------------------------------------
void MSG_WriteShort(buf_t *b, int c)
{
    byte *buf;
    buf = SZ_GetSpace(b, 2);
    buf[0] = c&0xff;
    buf[1] = c>>8;
}
//--------------------------------------------------------------------------
void MSG_WriteLong(buf_t *b, int c)
{
    byte *buf;
    buf = SZ_GetSpace(b, 4);
    buf[0] = c&0xff;
    buf[1] = (c>>8)&0xff;
    buf[2] = (c>>16)&0xff;
    buf[3] = c>>24;
}
//--------------------------------------------------------------------------
void MSG_WriteString(buf_t *b, char *s)
{
    if (!s)
        MSG_WriteByte(b, 0);
    else
    {
        SZ_Write(b, s, strlen(s));
        MSG_WriteByte(b, 0);
    }
}
//--------------------------------------------------------------------------
void MSG_WriteFloat(buf_t *b, float f)
{
    union { float f; int l; } dat;
	dat.f = f;
	SZ_Write (b, &dat.l, 4);
}
//--------------------------------------------------------------------------
int MSG_ReadByte(bool do_exception)
{
    int c;
    if (msg_readcount+1 > net_message.cursize)
    {
        msg_badread = true;
        if(do_exception) throw 1;
        return -1;
    }

    c = (unsigned char)net_message.data[msg_readcount];
    msg_readcount++;
    return c;
}
//--------------------------------------------------------------------------
int MSG_ReadShort(bool do_exception)
{
    int c;
    if (msg_readcount+2 > net_message.cursize)
    {
        msg_badread = true;
        if(do_exception) throw 1;
        return -1;
    }

    c = (unsigned short)(net_message.data[msg_readcount]
             + (net_message.data[msg_readcount+1]<<8));

    msg_readcount += 2;
    return c;
}
//--------------------------------------------------------------------------
int MSG_ReadLong(bool do_exception)
{
    int c;

    if (msg_readcount+4 > net_message.cursize)
    {
        msg_badread = true;
        if(do_exception) throw 1;
        return -1;
    }

    c = net_message.data[msg_readcount]
     + (net_message.data[msg_readcount+1]<<8)
     + (net_message.data[msg_readcount+2]<<16)
     + (net_message.data[msg_readcount+3]<<24);

    msg_readcount += 4;
    return c;
}
//--------------------------------------------------------------------------
char *MSG_ReadString(bool do_exception)
{
    static char string[2048];
    signed char c;
    unsigned int l=0;
    do
    {
        c = (signed char)MSG_ReadByte(do_exception);
        if(c == -1) c=0; // bond - if read after the end of packet
        if(c == 0) break;
        string[l] = c;
        l++;
    } while (l < sizeof(string)-1);

    string[l] = 0;
    return string;
}
//--------------------------------------------------------------------------
float MSG_ReadFloat(bool do_exception)
{
	union { byte b[4]; float f; int	l; } dat;

    if (msg_readcount+4 > net_message.cursize)
    {
        msg_badread = true;
        if(do_exception) throw 1;
		dat.f = -1;
    }
	else
	{
		dat.b[0] = net_message.data[msg_readcount];
		dat.b[1] = net_message.data[msg_readcount+1];
		dat.b[2] = net_message.data[msg_readcount+2];
		dat.b[3] = net_message.data[msg_readcount+3];
	}
	msg_readcount += 4;
	return dat.f;
}
//--------------------------------------------------------------------------
bool MSG_ReadAdr(char* sadr)
{
    if(msg_readcount+6 > net_message.cursize) return false;
    struct svadr {byte ip[4]; unsigned short port;};
    svadr* sv=(svadr*)&net_message.data[msg_readcount];
    sprintf(sadr,"%u.%u.%u.%u:%u",sv->ip[0],sv->ip[1],sv->ip[2],sv->ip[3],sv->port);
    msg_readcount+=6;
    return true;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void NET_Clear(void)
{
	struct sockaddr_in from;
	socklen_t fromlen;

    fromlen=sizeof(from);
    while(recvfrom(net_socket,(char *)net_message_buffer,NETMESSAGEBUFFERSIZE,0,
                  (struct sockaddr *)&from,&fromlen)>0);
}
//--------------------------------------------------------------------------
void NET_HuffDecodePacket(void)
{
    memcpy(huffbuff,net_message_buffer,net_message.cursize);
    HuffDecode(huffbuff,(unsigned char *)net_message_buffer
              ,net_message.cursize,&net_message.cursize);
}
//--------------------------------------------------------------------------

