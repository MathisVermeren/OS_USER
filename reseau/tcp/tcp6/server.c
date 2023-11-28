#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <arpa/inet.h>

int envoyerMessageServeurDGSE(char *servername,int portno, char *mess)
{
	int sockfdDGSE,n;
        struct sockaddr_in serv_addrDGSE;
        struct hostent *serverDGSE;
        char buffer[256];

        sockfdDGSE = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfdDGSE < 0)
        {
                fprintf(stderr,"Impossible d'ouvrir la socket. On arrête tout.\n");
                return 1;
        }

        serverDGSE = gethostbyname(servername);
        if (serverDGSE == NULL)
        {
                fprintf(stderr,"Impossible de récupérer les infos sur le serveur\n");
                return 1;
        }

        // On donne toutes les infos sur le serveur
        bzero((char *) &serv_addrDGSE, sizeof(serv_addrDGSE));
        serv_addrDGSE.sin_family = AF_INET;
        bcopy((char *)serverDGSE->h_addr, (char *)&serv_addrDGSE.sin_addr.s_addr, serverDGSE->h_length);
        serv_addrDGSE.sin_port = htons(portno);

        // On se connecte. L'OS local nous trouve un numéro de port, grâce auquel le serveur
        // peut nous renvoyer des réponses

        if (connect(sockfdDGSE,(struct sockaddr *) &serv_addrDGSE,sizeof(serv_addrDGSE)) < 0)
        {
                fprintf(stderr,"Impossible de faire l'appel system connect().\n");
                return 1;
        }

        sprintf(buffer,"%s\n",mess);

        n = write(sockfdDGSE,buffer,strlen(buffer));

        // On ferme la socket

        close(sockfdDGSE);

	return 0;
}

void traitement(char *mess)
{
	printf("Traitement de la requête '%s'\n",mess);	
	char votant=mess[0];
	char vote=mess[1];

	printf("%c a voté pour %c\n",votant,vote);
	if (vote!='1')
		envoyerMessageServeurDGSE("localhost",32001,mess);
}

int main(int argc, char *argv[])
{
     	int sockfd, newsockfd, portno;
     	socklen_t clilen;
     	char buffer[256];
     	char sendBuffer[256];
     	struct sockaddr_in serv_addr, cli_addr;
     	int n;

     	if (argc < 2) 
	{
         	fprintf(stderr,"./server <numport>\n");
         	fprintf(stderr,"ex: ./server 32000\n");
		return 1;
    	}

	// 1) on crée la socket, SOCK_STREAM signifie TCP
     	sockfd = socket(AF_INET, SOCK_STREAM, 0);
     	if (sockfd < 0) 
	{
         	fprintf(stderr,"Impossible d'ouvrir la socket.\n");
		return 1;
	}

	// 2) on réclame au noyau l'utilisation du port passé en paramètre 
	// INADDR_ANY dit que la socket va être affectée à toutes les interfaces locales

     	bzero((char *) &serv_addr, sizeof(serv_addr));
     	portno = atoi(argv[1]);
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons(portno);
     	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
         	fprintf(stderr,"Impossible de faire l'appel système bind().\n");
		return 1;
	}


	// On commence à écouter sur la socket. Le 5 est le nombre max
	// de connexions pendantes

     	listen(sockfd,5);
     	while (1)
     	{    
     		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     		if (newsockfd < 0) 
		{
         		fprintf(stderr,"Impossible de faire l'appel système accept().\n");
			return 1;
		}

     		bzero(buffer,256);
     		n = read(newsockfd,buffer,255);
     		if (n < 0) 
		{
         		fprintf(stderr,"Impossible de faire l'appel système read().\n");
			return 1;
		}

        	printf("Received packet from %s:%d\nData: [%s]\n\n",
                	inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer);

		traitement(buffer);

		sprintf(sendBuffer,"a voté\n");
		n=write(newsockfd,sendBuffer,strlen(sendBuffer));	

     		close(newsockfd);
     	}

     	close(sockfd);
     	return 0; 
}
