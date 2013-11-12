//server.h - server header file
//Author: Ali.B
#ifndef _SERVER_H
#define	_SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif//function prototypes
int bt_recv();
int logger(char *log_prefix,char *buffer);
int bt_send(char *COMMAND);
int payload_assembler(char * line);


#ifdef	__cplusplus
}
#endif

#endif	


