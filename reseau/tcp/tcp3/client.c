#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char *argv[])
{
    	int sockfd, portno, n;
    	struct sockaddr_in serv_addr;
    	struct hostent *server;

    	char buffer[256];
    	char receiveBuffer[256];

	if (argc<4)
	{
		fprintf(stderr,"./client <server_address> <server_numport> <message>\n");
		fprintf(stderr,"ex: ./client localhost 32000 \"joe\"\n");
		return 1;
	}

    	portno = atoi(argv[2]);
	// 1) Création de la socket, INTERNET et TCP

    	sockfd = socket(AF_INET, SOCK_STREAM, 0);

    	if (sockfd < 0) 
	{
		fprintf(stderr,"Impossible d'ouvrir la socket. On arrête tout.\n");
		return 1;
	}
    
	server = gethostbyname(argv[1]);
    	if (server == NULL) 
	{
        	fprintf(stderr,"Impossible de récupérer les infos sur le serveur\n");
		return 1;
    	}

	// On donne toutes les infos sur le serveur
    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    	serv_addr.sin_port = htons(portno);

	// On se connecte. L'OS local nous trouve un numéro de port, grâce auquel le serveur
	// peut nous renvoyer des réponses

    	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
		fprintf(stderr,"Impossible de faire l'appel system connect().\n");
		return 1;
	}

	sprintf(buffer,"%s\n",argv[3]);
        n = write(sockfd,buffer,strlen(buffer));

        n = read(sockfd,receiveBuffer,256);

	printf("receiveBuffer='%s'",receiveBuffer);

	// On ferme la socket

    	close(sockfd);

    	return 0;
}
