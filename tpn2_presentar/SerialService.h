#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
# include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include "SerialManager.h"
#include "rs232.h"
 
#define  PORTNUMBER ttyUSB1  
#define  PORT 10000
#define  BACKLOG 5
#define  TOTAL_SIZE_BUFFER 128

char TxSerial_RxServer[TOTAL_SIZE_BUFFER];
char RxSerial_TxServer[TOTAL_SIZE_BUFFER];	
static int fd2,fd1;

pthread_mutex_t mutexFd2 = PTHREAD_MUTEX_INITIALIZER;


void bloquearSign(void);
void desbloquearSign(void);
void sigint_handler(int sig);
void sigterm_handler(int sig);
void * TxRxCIAA(void* message);
void * ConnectionTCPServer(void * message);
