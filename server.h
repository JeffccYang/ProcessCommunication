/*

	typedef struct s_UserInfo{
 		int userID;
 		unsigned char va[1]; // 0x00000001:FD  0x00000010:OD ...

 	}sUserInfo;


*/
#ifndef __SERVER_H__
#define __SERVER_H__

#include "va_interface.h"
 

#ifdef __cplusplus
extern "C"{
#endif
/*

*/
	typedef void (*vaCB)(sVAParas *vaParas, int vaParasCount);

	void regCB(vaCB funp);
 
 	void runServer(); 	

 	void stopServer();
 


#ifdef __cplusplus
}
#endif

 

#endif /* __SERVER_H__ */