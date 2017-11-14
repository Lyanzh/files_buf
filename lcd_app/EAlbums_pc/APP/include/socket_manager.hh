#ifndef __SOCKET_MANAGER__
#define __SOCKET_MANAGER__
#include <sys/time.h>
#include <pthread.h>

typedef struct Socket_Operation
{
	const char * c_pcName;
	int iClientSocketFd;
	int iIsConnected;
	pthread_t tSendTreadID;
	pthread_t tRecvTreadID;
	pthread_t tHeartbeatTreadID;
	int (*Socket_Init)(char *pcServerAddr);
	void (*Socket_Exit)(void);
	int (*Socket_Send_Data)(char *pcDataSend);
	int (*Socket_Recv_Data)(char *pcDataRecv);
	struct Socket_Operation *ptNext;
} T_Socket_Opr, *PT_Socket_Opr;

#define DATA_MAX_LEN	4096

int UDP_Socket_Init(void);
int TCP_Socket_Init(void);

extern int Socket_Opr_Regisiter(PT_Socket_Opr ptSocketOpr);
extern void Show_Socket_Opr(void);
extern PT_Socket_Opr Get_Socket_Opr(char *pcName);

extern int Socket_Opr_Init(void);
extern int All_Socket_Init(void);
extern int Socket_Select_And_Init(char *pcSocketType, char *pcServerAddr);
extern int Socket_Send(char *pcDataSend);

#endif

