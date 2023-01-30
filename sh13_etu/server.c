//lancer avec ./server 32000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <arpa/inet.h>

struct _client
{
        char ipAddress[40];
        int port;
        char name[40];
} tcpClients[4];
int nbClients;
int fsmServer; //état du serveur
int deck[13]={0,1,2,3,4,5,6,7,8,9,10,11,12}; //tableau des cartes
int tableCartes[4][8]; //tableau d'affichage des cartes (dans l'interface graphique)
char *nomcartes[]=
{"Sebastian Moran", "irene Adler", "inspector Lestrade",
  "inspector Gregson", "inspector Baynes", "inspector Bradstreet",
  "inspector Hopkins", "Sherlock Holmes", "John Watson", "Mycroft Holmes",
  "Mrs. Hudson", "Mary Morstan", "James Moriarty"}; //nom des cartes
int joueurCourant;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void melangerDeck()
{ //swap deux cartes au hasard cent fois
        int i;
        int index1,index2,tmp;

        for (i=0;i<1000;i++)
        {
                index1=rand()%13;
                index2=rand()%13;

                tmp=deck[index1];
                deck[index1]=deck[index2];
                deck[index2]=tmp;
        }
}

void incrJoueurCourant()
{
	do
	{
		joueurCourant = (joueurCourant+1)%4;
	}
	while(elimines[joueurCourant]==1);
	
}

void createTable()
{// dsitribution des cartes aux joueurs
	// Le joueur 0 possede les cartes d'indice 0,1,2
	// Le joueur 1 possede les cartes d'indice 3,4,5
	// Le joueur 2 possede les cartes d'indice 6,7,8
	// Le joueur 3 possede les cartes d'indice 9,10,11
	// Le coupable est la carte d'indice 12 :O
	int i,j,c;

	for (i=0;i<4;i++)
		for (j=0;j<8;j++)
			tableCartes[i][j]=0; // mise des élements à zéro

	for (i=0;i<4;i++) // pour tous les joueurs
	{
		for (j=0;j<3;j++) // pour toutes les cartes du joueur
		{
			c=deck[i*3+j];
			switch (c) // remplissage du tableau des indices (petites icones)
			{
				case 0: // Sebastian Moran
					tableCartes[i][7]++;
					tableCartes[i][2]++;
					break;
				case 1: // Irene Adler
					tableCartes[i][7]++;
					tableCartes[i][1]++;
					tableCartes[i][5]++;
					break;
				case 2: // Inspector Lestrade
					tableCartes[i][3]++;
					tableCartes[i][6]++;
					tableCartes[i][4]++;
					break;
				case 3: // Inspector Gregson
					tableCartes[i][3]++;
					tableCartes[i][2]++;
					tableCartes[i][4]++;
					break;
				case 4: // Inspector Baynes
					tableCartes[i][3]++;
					tableCartes[i][1]++;
					break;
				case 5: // Inspector Bradstreet
					tableCartes[i][3]++;
					tableCartes[i][2]++;
					break;
				case 6: // Inspector Hopkins
					tableCartes[i][3]++;
					tableCartes[i][0]++;
					tableCartes[i][6]++;
					break;
				case 7: // Sherlock Holmes
					tableCartes[i][0]++;
					tableCartes[i][1]++;
					tableCartes[i][2]++;
					break;
				case 8: // John Watson
					tableCartes[i][0]++;
					tableCartes[i][6]++;
					tableCartes[i][2]++;
					break;
				case 9: // Mycroft Holmes
					tableCartes[i][0]++;
					tableCartes[i][1]++;
					tableCartes[i][4]++;
					break;
				case 10: // Mrs. Hudson
					tableCartes[i][0]++;
					tableCartes[i][5]++;
					break;
				case 11: // Mary Morstan
					tableCartes[i][4]++;
					tableCartes[i][5]++;
					break;
				case 12: // James Moriarty
					tableCartes[i][7]++;
					tableCartes[i][1]++;
					break;
			}
		}
	}
} // le jeu est fini du côté du serveur

void printDeck()
{
        int i,j;

        for (i=0;i<13;i++)
                printf("%d %s\n",deck[i],nomcartes[deck[i]]);

	for (i=0;i<4;i++)
	{
		for (j=0;j<8;j++)
			printf("%2.2d ",tableCartes[i][j]);
		puts("");
	}
}

void printClients()
{ // imprime les ips et les ports
        int i;

        for (i=0;i<nbClients;i++)
                printf("%d: %s %5.5d %s\n",i,tcpClients[i].ipAddress,
                        tcpClients[i].port,
                        tcpClients[i].name);
}

int findClientByName(char *name)
{
        int i;

        for (i=0;i<nbClients;i++)
                if (strcmp(tcpClients[i].name,name)==0)
                        return i;
        return -1;
}

void sendMessageToClient(char *clientip,int clientport,char *mess)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server = gethostbyname(clientip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(clientport);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
                printf("ERROR connecting\n");
                exit(1);
        }

        sprintf(buffer,"%s\n",mess);
        n = write(sockfd,buffer,strlen(buffer));

    close(sockfd);
}

void broadcastMessage(char *mess) // message en entrée, envoie a tous les clients
{
        int i;

        for (i=0;i<nbClients;i++)
                sendMessageToClient(tcpClients[i].ipAddress,
                        tcpClients[i].port,
                        mess);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
	int i;

    char com;
    char clientIpAddress[256], clientName[256];
    int clientPort;
    int id;
    char reply[256];
	int accusedCard // accusé d'être coupable par current joueur
	int elimines[4];

	for(int i=0;i<=0;i++){ //initialisation de la liste des éliminés
		elimines[i]=0;
	}

    if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);

	printDeck();
	melangerDeck();
	createTable();
	printDeck();
	joueurCourant=0;

	for (i=0;i<4;i++)
	{ // préremplissage du tableau tcpClients
        	strcpy(tcpClients[i].ipAddress,"localhost");
        	tcpClients[i].port=-1;
        	strcpy(tcpClients[i].name,"-");
	}

     while (1)
     {
     	newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen); //attente d'une connexion
     	if (newsockfd < 0)
          	error("ERROR on accept");

     	bzero(buffer,256);
     	n = read(newsockfd,buffer,255);
     	if (n < 0)
		error("ERROR reading from socket");

        printf("Received packet from %s:%d\nData: [%s]\n\n",
                inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer);

        if (fsmServer==0)  //initialiser fsmserver à 0 [à rajouter]
        {
        	switch (buffer[0])
        	{
                	case 'C':
                        	sscanf(buffer,"%c %s %d %s", &com, clientIpAddress, &clientPort, clientName);
                        	printf("COM=%c ipAddress=%s port=%d name=%s\n",com, clientIpAddress, clientPort, clientName);

                        	// fsmServer==0 alors j'attends les connexions de tous les joueurs
                                strcpy(tcpClients[nbClients].ipAddress,clientIpAddress);
                                tcpClients[nbClients].port=clientPort;
                                strcpy(tcpClients[nbClients].name,clientName);
                                nbClients++;

                                printClients();

				// rechercher l'id du joueur qui vient de se connecter

                                id=findClientByName(clientName);
                                printf("id=%d\n",id);

				// lui envoyer un message personnel pour lui communiquer son id

                                sprintf(reply,"I %d",id); //"I" = identifiant
                                sendMessageToClient(tcpClients[id].ipAddress,
                                       tcpClients[id].port,
                                       reply);

				// Envoyer un message broadcast pour communiquer a tout le monde la liste des joueurs actuellement
				// connectes

                                sprintf(reply,"L %s %s %s %s", tcpClients[0].name, tcpClients[1].name, tcpClients[2].name, tcpClients[3].name); //"L" = liste des joueurs connectés, au départ chaqun des champs de cette liste est remplacée par des tirets si personne n'est co sur les places restantes
                                broadcastMessage(reply);

				// Si le nombre de joueurs atteint 4, alors on peut lancer le jeu

                                if (nbClients==4)
				{
					//RAJOUTER DU CODE ICI
					// Le joueur 0 possede les cartes d'indice 0,1,2
					// Le joueur 1 possede les cartes d'indice 3,4,5
					// Le joueur 2 possede les cartes d'indice 6,7,8
					// Le joueur 3 possede les cartes d'indice 9,10,11
					// Le coupable est la carte d'indice 12 :O
					// On envoie ses cartes au joueur 0, ainsi que la ligne qui lui correspond dans tableCartes

					sprintf(reply, "D %d %d %d %d",deck[0],deck[1],deck[2]);
					sendMessageToClient(tcpClients[0].ipAddress,tcpClients[0].port,reply);
					for(i=0;i<7;i++)
					{
						sprintf(reply, "V %d %d",i,tableCartes[0][i]);
						sendMessageToClient(tcpClients[0].ipAddress,tcpClients[0].port,reply);
					}
					// On envoie ses cartes au joueur 1, ainsi que la ligne qui lui correspond dans tableCartes
					sprintf(reply, "D %d %d %d",deck[3],deck[4],deck[5]);
					sendMessageToClient(tcpClients[1].ipAddress,tcpClients[1].port,reply);
					for(i=0;i<7;i++)
					{
						sprintf(reply, "V %d %d",i,tableCartes[1][i]);
						sendMessageToClient(tcpClients[1].ipAddress,tcpClients[1].port,reply);
					}

					// On envoie ses cartes au joueur 2, ainsi que la ligne qui lui correspond dans tableCartes
					sprintf(reply, "D %d %d %d",deck[6],deck[7],deck[8]);
					sendMessageToClient(tcpClients[2].ipAddress,tcpClients[2].port,reply);
					for(i=0;i<7;i++)
					{
						sprintf(reply, "V %d %d",i,tableCartes[2][i]);
						sendMessageToClient(tcpClients[2].ipAddress,tcpClients[2].port,reply);
					}

					// On envoie ses cartes au joueur 3, ainsi que la ligne qui lui correspond dans tableCartes
					sprintf(reply, "D %d %d %d",deck[9],deck[10],deck[11]);
					sendMessageToClient(tcpClients[3].ipAddress,tcpClients[3].port,reply);
					for(i=0;i<7;i++)
					{
						sprintf(reply, "V %d %d",i,tableCartes[3][i]);
						sendMessageToClient(tcpClients[3].ipAddress,tcpClients[3].port,reply);
					}

					// On envoie enfin un message a tout le monde pour definir qui est le joueur courant=0
					sprintf(reply, "M %d",joueurCourant);
					broadcastMessage(reply);

                     fsmServer=1;
				}
				break;
                }
	}
	else if (fsmServer==1)
	{
		switch (buffer[0])
		{
                	case 'G': // pour guilty le joueur a perdu et est déconnecté, le serveur doit répondre aux joueurs à sa place
					sscanf(buffer+2," %d %d", accusedCard);
					if(accusedCard==deck[12])
					{
						printf("Bravo! %s a trouvé le coupable, c'était %s",tcpClients.name[joueurCourant],nomcartes[deck[accusedCard]]);
					}
					else
					{
						elimines[joueurCourant] = 1;
					}

					incrJoueurCourant();
				break;
                	case 'O': //"qui a tel item"
				// RAJOUTER DU CODE ICI
				incrJoueurCourant();
				break;
			case 'S'://"toi spécifiquement joueur n° machin combien tu as de item (ex crâne) machin"
				// RAJOUTER DU CODE ICI

				incrJoueurCourant();
				sprintf(sendBuffer,"S %d %d %d",gId, joueurSel,objetSel); //message demande fermée à un joueur
				break;
                	default:
                        	break;
		}
        }
     	close(newsockfd);
     }
     close(sockfd);
     return 0;
}
