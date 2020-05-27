#include "SerialService.h"

void bloquearSign(void)
{
    sigset_t set;
   // int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void desbloquearSign(void)
{
    sigset_t set;
 //   int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void sigterm_handler(int sig)
{
	printf("SIGTERM..\n\r");
	close(fd1);
	close(fd2);
	exit(1);
}

void sigint_handler(int sig)
{
	printf("CRTL C - SIGINT... \n\r");
	close(fd1);
	close(fd2);
	exit(1);
}

void * TxRxCIAA(void* message){

int len;
size_t bytesSerial;	

  while(1){
	   	if((bytesSerial = serial_receive(RxSerial_TxServer,TOTAL_SIZE_BUFFER*sizeof(char))) >	 0 ){
							
			RxSerial_TxServer[bytesSerial]='\0';					           		
			len = strlen(RxSerial_TxServer);

			pthread_mutex_lock(&mutexFd2);	 			
			write(fd2,RxSerial_TxServer,len);
			pthread_mutex_unlock(&mutexFd2);	
		}else{
			if(bytesSerial == -1){
				perror("Error en read()..!!");
				exit(1);	
			}
		}			
	sleep(1);
	}
}

void * ConnectionTCPServer(void * message){

struct sockaddr_in serverAddr;
struct sockaddr_in clientAddr;
int bytes;
socklen_t addr_len;
char ipClient[32];


	fd1 = socket (AF_INET,SOCK_STREAM,0);
	bzero((char *)&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT); 
	printf("Run Thread Socket Server..\n\r");
	if(inet_pton(AF_INET,"127.0.0.1",&(serverAddr.sin_addr)) <= 0){
		fprintf(stderr,"ERROR invlid Server IP\n\r");
		exit(1);		
	}	
	if(bind(fd1,(struct sockaddr *)&serverAddr,sizeof(serverAddr)) == -1){
		close(fd1);
		perror("listner: bind");
		exit(1);				
	}
	// ponemos al socket en modo pasivo poniendolo a escuchar nuevas conexiones
	if(listen(fd1,BACKLOG) == -1){
		printf("error en listen()");	
		exit(1);		
	}
	while(1){
			addr_len = sizeof(struct sockaddr_in);
			printf("Esperando por una Conexion..\n\r");
			pthread_mutex_lock(&mutexFd2);
			if((fd2 = accept(fd1,(struct sockaddr *) &clientAddr,&addr_len)) == -1 ){
				perror("Error en accept()\n\r");
				close(fd1);
				exit(1);			
			}else{ 	pthread_mutex_unlock(&mutexFd2);
				while(1){			
						inet_ntop(AF_INET, &(clientAddr.sin_addr),ipClient,sizeof(ipClient));
						printf("Server: conexion desde %s\n",ipClient);				
									
						if((bytes = read(fd2,TxSerial_RxServer,TOTAL_SIZE_BUFFER)) == -1 ){
						perror("Error al leer el mensaje en socket Server");
						
						exit(1);
							
						}						
						if( bytes == 0){
							
							break; //si es 0=EOF es poque el cliente se desconecto  
						}								
						TxSerial_RxServer[bytes] = 0x00;         
						printf("Recibi %d bytes %s\n",bytes,TxSerial_RxServer);
						serial_send(TxSerial_RxServer,bytes);	
					}
					close(fd2);
			}
					
		}// end while 
}

int main(void)
{
int baudRate = 115200; 
int ttyUBS1 = 1;
int serial,tcpServer;
pthread_t SerialUsbPlaca, TCPServer;
        
	//configuro Handlers de señales
	printf("Asigno handler SIGINT\n");

	// signal ctrl+c
	struct sigaction sa1,sa2;
	
	sa1.sa_handler = sigint_handler;
	sa1.sa_flags = SA_RESTART;
	sigemptyset(&sa1.sa_mask);
	if (sigaction(SIGINT, &sa1, NULL) == -1) {
		perror("sigaction1");
		return -1;
	}

        printf("Asigno handler SIGTERM\n");  
	sa2.sa_handler = sigterm_handler;
	sa2.sa_flags = SA_RESTART;
	sigemptyset(&sa2.sa_mask);
	if (sigaction(SIGTERM, &sa2, NULL) == -1) {
		perror("sigaction2");
		return -1;
	}
	bloquearSign();

	//incicializo el puerto serial de la pc por donde se comunicará con la placa
	if(1 == serial_open(ttyUBS1,baudRate)){
		printf("Error al abrir el puerto serial..\n\r");
		return -1;	
	}
		
	serial = pthread_create (&SerialUsbPlaca,NULL,TxRxCIAA,NULL);
	if(serial!=0){
		errno =	serial;
		perror("pthread_create_Serial");
		return -1;
	}
	
 	tcpServer = pthread_create (&TCPServer,NULL,ConnectionTCPServer,NULL);
	if(tcpServer!=0){
		errno =	tcpServer;
		perror("pthread_create_Server");
		return -1;
	}
	desbloquearSign();

	pthread_join (TCPServer,NULL);
	pthread_join (SerialUsbPlaca,NULL);	
	
exit(EXIT_SUCCESS);
return 0;
}

