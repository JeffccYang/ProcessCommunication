 

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "server.h"
 
 void CB(sVAParas *vaParas, int vaParasCount)
 {
 	if(vaParasCount>0)
 		printf("<<<<%d,%d>>>>>\n", vaParasCount, vaParas[0].bottom);
 }

/*---------------------------------------------------------------------*/
/*--- main - 											            ---*/
/*---------------------------------------------------------------------*/
int main(int count, char *strings[])
{    

	regCB(CB);
	
	runServer();

	getchar();

 	stopServer();
 	
	getchar();

    return 0;
}

 
 