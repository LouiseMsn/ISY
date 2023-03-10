// lancer avec ./server 32000

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
int fsmServer;											   // état du serveur 0 = connexion 1 = jeu
int deck[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}; // tableau des cartes
int tableCartes[4][8];									   // tableau d'affichage des cartes (dans l'interface graphique)
char *nomcartes[] =
	{"Sebastian Moran", "irene Adler", "inspector Lestrade",
	 "inspector Gregson", "inspector Baynes", "inspector Bradstreet",
	 "inspector Hopkins", "Sherlock Holmes", "John Watson", "Mycroft Holmes",
	 "Mrs. Hudson", "Mary Morstan", "James Moriarty"}; // nom des cartes
int joueurCourant;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void melangerDeck()
{ // swap deux cartes au hasard cent fois
	int i;
	int index1, index2, tmp;

	for (i = 0; i < 1000; i++)
	{
		index1 = rand() % 13;
		index2 = rand() % 13;

		tmp = deck[index1];
		deck[index1] = deck[index2];
		deck[index2] = tmp;
	}
}

void createTable()
{ // distribution des cartes ( et donc des objets associés) aux joueurs
	// Le joueur 0 possede les cartes d'indice 0,1,2
	// Le joueur 1 possede les cartes d'indice 3,4,5
	// Le joueur 2 possede les cartes d'indice 6,7,8
	// Le joueur 3 possede les cartes d'indice 9,10,11
	// Le coupable est la carte d'indice 12 :O
	int i, j, c;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 8; j++)
			tableCartes[i][j] = 0; // initialisation à 0 du tableau

	for (i = 0; i < 4; i++) // pour tous les joueurs
	{
		for (j = 0; j < 3; j++) // pour toutes les cartes du joueur
		{
			c = deck[i * 3 + j];
			switch (c) // remplissage du tableau des objets (petites icones)
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
	int i, j;

	for (i = 0; i < 13; i++)
		printf("%d %s\n", deck[i], nomcartes[deck[i]]);

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 8; j++)
			printf("%2.2d ", tableCartes[i][j]);
		puts("");
	}
}

void printClients()
{ // imprime les ips et les ports
	int i;

	for (i = 0; i < nbClients; i++)
		printf("%d: %s %5.5d %s\n", i, tcpClients[i].ipAddress,
			   tcpClients[i].port,
			   tcpClients[i].name);
}

int findClientByName(char *name)
{
	int i;

	for (i = 0; i < nbClients; i++)
		if (strcmp(tcpClients[i].name, name) == 0)
			return i;
	return -1;
}

void sendMessageToClient(char *clientip, int clientport, char *mess)
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server = gethostbyname(clientip);
	if (server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		  (char *)&serv_addr.sin_addr.s_addr,
		  server->h_length);
	serv_addr.sin_port = htons(clientport);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("ERROR connecting\n");
		exit(1);
	}

	sprintf(buffer, "%s\n", mess);
	n = write(sockfd, buffer, strlen(buffer));

	close(sockfd);
}

void broadcastMessage(char *mess) // message mess en entrée, l'envoie a tous les clients
{
	int i;

	for (i = 0; i < nbClients; i++)
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
	int i, j;

	char com;
	char clientIpAddress[256], clientName[256];
	int clientPort;
	int id;
	char reply[256];

	// erreur si pas assez d'arguments
	if (argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	// mise en place du socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	// initialisation des cartes, de la table et du joueur courant à 0
	printDeck();
	melangerDeck();
	createTable();
	printDeck();
	joueurCourant = 0;

	for (i = 0; i < 4; i++)
	{ // préremplissage du tableau tcpClients
		strcpy(tcpClients[i].ipAddress, "localhost");
		tcpClients[i].port = -1;
		strcpy(tcpClients[i].name, "-");
	}

	// boucle principale
	while (1)
	{
		// attente d'une connexion
		newsockfd = accept(sockfd,
						   (struct sockaddr *)&cli_addr,
						   &clilen);

		if (newsockfd < 0)
			error("ERROR on accept"); // erreur de connexion

		bzero(buffer, 256);
		n = read(newsockfd, buffer, 255);
		if (n < 0)
			error("ERROR reading from socket");

		printf("Received packet from %s:%d\nData: [%s]\n\n",
			   inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer);

		if (fsmServer == 0) // fsmServer==0  : attente des connexions de tous les joueurs
		{
			switch (buffer[0])
			{
				// Message 'C' : Demande de connection de la part d'un client
			case 'C':
				sscanf(buffer, "%c %s %d %s", &com, clientIpAddress, &clientPort, clientName); // récupération de l'Ip du joueur, de son Port et de son nom
				printf("COM=%c ipAddress=%s port=%d name=%s\n", com, clientIpAddress, clientPort, clientName);

				// stockage local de l'Ip, port et nom dans liste
				strcpy(tcpClients[nbClients].ipAddress, clientIpAddress);
				tcpClients[nbClients].port = clientPort;
				strcpy(tcpClients[nbClients].name, clientName);
				nbClients++;

				printClients();

				// recherche de l'id du joueur qui vient de se connecter
				id = findClientByName(clientName);
				printf("id=%d\n", id);

				// Message 'I' : envoi d'un message personnel pour lui communiquer son id au client
				sprintf(reply, "I %d", id); //"I" = identifiant
				sendMessageToClient(tcpClients[id].ipAddress,
									tcpClients[id].port,
									reply);

				// Message 'L' : Envoi d'un message broadcast pour communiquer a tout le monde la liste des joueurs actuellement
				// connectes
				sprintf(reply, "L %s %s %s %s", tcpClients[0].name, tcpClients[1].name, tcpClients[2].name, tcpClients[3].name); // les champs de cette liste sont remplacés par des tirets si personne n'est co sur les places restantes
				broadcastMessage(reply);

				// Si le nombre de joueurs atteint 4, alors on peut lancer le jeu
				if (nbClients == 4)
				{
					// On envoie ses cartes au joueur 0, ainsi que la ligne qui lui correspond dans tableCartes

					sprintf(reply, "D %d %d %d", deck[0], deck[1], deck[2]);
					sendMessageToClient(tcpClients[0].ipAddress, tcpClients[0].port, reply);

					for (i = 0; i < 8; i++)
					{
						sprintf(reply, "V 0 %d %d", i, tableCartes[0][i]);
						sendMessageToClient(tcpClients[0].ipAddress, tcpClients[0].port, reply);
					}

					// On envoie ses cartes au joueur 1, ainsi que la ligne qui lui correspond dans tableCartes

					sprintf(reply, "D %d %d %d", deck[3], deck[4], deck[5]);
					sendMessageToClient(tcpClients[1].ipAddress, tcpClients[1].port, reply);

					for (i = 0; i < 8; i++)
					{
						sprintf(reply, "V 1 %d %d", i, tableCartes[1][i]);
						sendMessageToClient(tcpClients[1].ipAddress, tcpClients[1].port, reply);
					}

					// On envoie ses cartes au joueur 2, ainsi que la ligne qui lui correspond dans tableCartes

					sprintf(reply, "D %d %d %d", deck[6], deck[7], deck[8]);
					sendMessageToClient(tcpClients[2].ipAddress, tcpClients[2].port, reply);
					for (i = 0; i < 8; i++)
					{
						sprintf(reply, "V 2 %d %d", i, tableCartes[2][i]);
						sendMessageToClient(tcpClients[2].ipAddress, tcpClients[2].port, reply);
					}

					// On envoie ses cartes au joueur 3, ainsi que la ligne qui lui correspond dans tableCartes

					sprintf(reply, "D %d %d %d", deck[9], deck[10], deck[11]);
					sendMessageToClient(tcpClients[3].ipAddress, tcpClients[3].port, reply);

					for (i = 0; i < 8; i++)
					{
						sprintf(reply, "V 3 %d %d", i, tableCartes[3][i]);
						sendMessageToClient(tcpClients[3].ipAddress, tcpClients[3].port, reply);
					}

					// On envoie enfin un message a tout le monde pour definir qui est le joueur courant = 0

					sprintf(reply, "M %d", joueurCourant);
					printf("Joueur Courant : %d\n", joueurCourant);
					broadcastMessage(reply);

					fsmServer = 1; // tous les joueurs sont connectés
				}

				break;
			}
		}
		else if (fsmServer == 1) // reception d'un message
		{
			switch (buffer[0])
			{
			// Message 'G' : accusation d'un personnage par un joueur, le serveur va répodnre vrai(1) ou faux(0)
			case 'G':
				sscanf(buffer, "G %d %d", &id, &i); // id = joueur qui accuse, i = personnage accusé
				if (i == deck[12])					// si accusation bonne
				{
					sprintf(reply, "A %d 1", id); // réponse avec un 1
					broadcastMessage(reply);
				}
				else
				{								  // si accusation fausse
					sprintf(reply, "A %d 0", id); // réponse avec un 0
					broadcastMessage(reply);

					if (joueurCourant++ == 3) // Passage au joueur suivant
					{
						joueurCourant = 0; // remise à zéro si numéro de joueur = 4
					}
				}
				break;

			// Message 'O' : demande ouverte pour un objet (qui a tel objet)
			case 'O':
				sscanf(buffer, "O %d %d", &id, &i); // récupération du joueur qui fait la demande et de la demande

				for (j = 0; j < 4; j++) // parcours de tous les joueurs
				{
					if (j == id)
					{
						// on ignore si le joueur demandé est le joueur qui fait la demande
					}
					else
					{
						if (tableCartes[j][i] == 0) // si le joueur n'a pas l'objet
						{
							sprintf(reply, "V %d %d 0", j, i); // envoi d'une réponse négative
						}
						else // si le joueur a l'objet
						{
							sprintf(reply, "V %d %d 100", j, i); // envoi d'une réponse positive 
						}
						broadcastMessage(reply);
					}
				}
				if (joueurCourant++ == 3) // passage au joueur suivant
				{
					joueurCourant = 0;
				}
				break;

			// Message 'S": demande spécifique (combien joueur n°_ a de __)
			case 'S':												   
				sscanf(buffer, "S %d %d %d", &id, &i, &j);			   // id = joueur qui fait la demande, i = joueur selectionné, j = obj selectionné
				sprintf(reply, "V %d %d %d", i, j, tableCartes[i][j]); // envoi de l'information demandée au joueur
				broadcastMessage(reply);
				if (joueurCourant++ == 3)
				{
					joueurCourant = 0;
				}
				break;

			default:
				break;
			}
			sprintf(reply, "M %d", joueurCourant); // annonce du joueur courant a tous les joueurs
			printf("Joueur Courant : %d\n", joueurCourant);
			broadcastMessage(reply);
		}
		close(newsockfd);
	}
	close(sockfd);
	return 0;
}
