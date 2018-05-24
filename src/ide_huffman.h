#ifndef ide_huffmanH
#define ide_huffmanH

void HuffDecode(unsigned char *in,unsigned char *out,int inlen,int *outlen);
void HuffEncode(unsigned char *in,unsigned char *out,int inlen,int *outlen);
bool HuffInit(void);

#endif
