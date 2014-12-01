#include <stdio.h>
#include <stdlib.h>
#include "processcom.h"

#include "nalu.h"

#if 0

int main()
{
/*
	char szSrcFile[256];
	sprintf(szSrcFile,"./face.264");
	send_to_listener_net("127.0.0.1", "1234" ,szSrcFile, sizeof(szSrcFile));
*/
 	z_client();

	return 0;	
}

#else
int main()
{
//-----------------------------------------
	FILE		*inFile;
	char szSrcFile[256];
	sprintf(szSrcFile,"./testfiles/b.264");
	inFile = fopen(szSrcFile, "rb");
	if(inFile==NULL){
		printf("[FAIL OPEN FILE!!!]\n");
		return 1;
	}
	printf("================================================\n%s\n",szSrcFile);

	NALU_t nalunit;
	IniDecSyntax( &(nalunit) ); 

	unsigned char *FrameBuffer = (unsigned char*)malloc(512*1024);
	int	nFrameNum = 0;
//-----------------------------------------	
 /// char buf[] = "hello unix socket~";
 /// send_to_listener("my_server", buf, sizeof(buf));
	int Framesize;	 
	Framesize = -1;

	while (1)
	{
		Framesize = ReadNextNalu(&(nalunit), FrameBuffer , inFile);//sps

		if( Framesize < 0 || nFrameNum > 250  ) 
		{
			//printf("[@@ %d @@]\m",i);
			break;
		}
		else
		{
 		 	client_send_to_listener("MONEY", FrameBuffer, Framesize);
		//	client_send_to_listener_net("127.0.0.1", "1234" ,FrameBuffer, Framesize);
		
			printf("%d[@@ %d @@]\n",nFrameNum,Framesize);
		}
		nFrameNum++;

		usleep(3000);
	}	

//-----------------------------------------  
	fclose(inFile);
//-----------------------------------------

	return 0;
}

#endif