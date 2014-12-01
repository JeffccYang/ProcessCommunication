#ifndef __PROCESSCOM_H__
#define __PROCESSCOM_H__
 
 #define SERVER_CHECK_TIMEOUT  3

#ifdef __cplusplus
extern "C"{
#endif
	
	typedef void (*funCB)(unsigned char*, int );
	/* 
	Server 
	*/
	int server_create_listener(char* name, funCB funp );
	int server_create_listener_net(char* port, funCB funp );
	int getUniqueName( char *name );
	void server_stop();
	/* 
	Client 
	*/
	int client_send_to_listener(char* app_name, void* buf, int buf_size);
	int client_send_to_listener_net(char* server_ip, char* port, void* buf, int buf_size);

#ifdef __cplusplus
}
#endif

#endif /* __PROCESSCOM_H__ */