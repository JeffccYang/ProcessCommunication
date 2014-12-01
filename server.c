#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "processcom.h"
#include "ffdecoder.h"

#include <pthread.h>
#include "server.h"
 
#define VA

vaCB gFunp = NULL;
 
//------------
AVDHANDLE	hDecoder;
pthread_t thread_id;
sVAParas vaParas[MAX_VAPARAS];

void decodedFrame(void *pContext, int nChannel, unsigned char *yData, unsigned int nWidth, unsigned int nHeight, 
	unsigned int nPitch, unsigned int	nFrameType)
{
 	int fdCount=0,i;

#ifdef VA 
 

#endif

 	printf("%s,%s: %d\n", __FILE__, __FUNCTION__,  fdCount);

}

void my_recv(unsigned char* buf ,int bufsize)
{
 	printf("%s,%s: %d\n", __FILE__, __FUNCTION__,  bufsize);
	AVS_DecodeVideo(hDecoder , buf , bufsize, 0);
}

void* SocketHandler(void* lp)
{
 	server_create_listener("MONEY", my_recv);
   	printf("%s,%s: %d\n", __FILE__, __FUNCTION__,  1);
}
   
/*

*/ 	
void regCB(vaCB funp)
{
	gFunp = funp;

}

void stopServer()
{
	server_stop(); 

	usleep(SERVER_CHECK_TIMEOUT*1000); 
 
 	AVS_CloseAVDecoder(&hDecoder);

 	printf("%s,%s: %d\n", __FILE__, __FUNCTION__,  0);
}


void runServer(unsigned char *vaType)
{
	int temp,t,ret=-1;
  
	ret = 1;
 
	if(ret>=0){
 
		hDecoder = AVS_OpenAVDecoder(0, AVD_CODECID_H264, decodedFrame);
 		//create_listener("my_server", my_recv);
 		if((temp = pthread_create(&thread_id,0,&SocketHandler, (void*)&t))==0){
 				pthread_detach(thread_id);
 		}
 

	}
	else
	{
		printf("%s \n", "runServer FAIL!!");
		exit(1);
	}
 
}
 