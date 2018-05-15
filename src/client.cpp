/**
 * \file client.c
 * \author Lucas Schott et Juan Manuel Torres Garcia
 * \brief fichier principale contenant le main du programme client
 */

#include "client_server.h"

//variable indiquant si l'option verbose a été activé lors du lancement du
//programme ou non
bool verbose = false;


/**
 * usage du programme client:
 *
 * ./client <adresse du server> <no de port du server> put <hash a envoyer>
 * <adresse associé>
 *
 * ./cleint <adresse du server> <no de port du server> get <hash>
 */
void exit_usage_client(void)
{
    fprintf(stderr,"usage: participant <adresse_serveur> <port_serveur>\
 transaction <depuis_compte> <vers_compte> <montant>\n\
        participant <adresse_serveur> <port_serveur> balance <depuis_compte>\n");
    exit(EXIT_FAILURE);
}



/**************************PUT************************************************/



/**
 * \fn put_t write_put_request(uint16_t * hash, struct in6_addr hash_addr)
 * \brief ecrire un message "put" contenant le hash et l'adresse donnés
 * en parametre
 */
transaction_t write_transaction(string fromAddr, string toAddr,double amount)
{
    transaction_t msg;
    msg.header = TRANSACTION_HEADER;
    strncpy(msg.from_addr,fromAddr.c_str(),PSEUDO_SIZE);
    strncpy(msg.to_addr,toAddr.c_str(),PSEUDO_SIZE);
    msg.amount = amount;

    return msg;
}



/**
 * \fn void put(int sockfd, struct addrinfo * server_info, uint16_t * h,
 * struct in6_addr a)
 * \brief envoyer une message put a partir de sa socket, a l'adresse
 * server_info et contenant les hash "h" et l'adresse "a"
 */
void put_transaction(int sockfd, struct addrinfo * server_info, string fromAddr, string toAddr, double amount)
{
    transaction_t msg = write_transaction(fromAddr,toAddr,amount);

    if (sendto(sockfd, &msg,sizeof(transaction_t), 0,
                server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("sendto");
        close(sockfd);;
        exit(EXIT_FAILURE);
    }
}



/************************GET**************************************************/


/**
 * \fn get_t write_get_request(uint16_t * hash)
 * \brief ecrire un message "get" contenant le hash donné en parametres
 */
ask_balance_t write_ask_balance(string fromAddr)
{
    ask_balance_t msg;
    msg.header = ASK_BALANCE_HEADER;
    strncpy(msg.from_addr,fromAddr.c_str(),PSEUDO_SIZE);
    return msg;
}

/**
 * \fn void get(int sockfd, struct addrinfo * server_info, uint16_t * h)
 * struct in6_addr a)
 * \brief envoyer une message "get" a partir de sa socket, a l'adresse
 * server_info et contenant le hash "h" puis attendre la réponse
 * (max 1 seconde) et afficher le résultat
 */
void get_balance(int sockfd, struct addrinfo * server_info, string fromAddr)
{
    bool rcvd = false;
    //send msg

    ask_balance_t msg = write_ask_balance(fromAddr);

    if (sendto(sockfd, &msg, sizeof(ask_balance_t), 0,
                server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //receive answer

    struct timeval time_val;
    fd_set read_set;

    FD_ZERO(&read_set);
    FD_SET(sockfd, &read_set);
    time_val.tv_sec = 1;
    time_val.tv_usec = 0;

    send_balance_t * answer = (send_balance_t*) malloc(sizeof(send_balance_t));

    //attendre une seconde la reception d'un message
    //si au bout de une seconde on ne recoit plus de message, on arrete d'en
    //attendre
    while (select(sockfd+1, &read_set, NULL, NULL, &time_val))
    {
        //tant qu'on recoit des messages on affiche le resultat
        if (recv(sockfd, answer, MSG_MAX_LENGTH, 0) == -1)
        {
            perror("recv:");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        rcvd = true;
        printf("balance %s : %lf\n", msg.from_addr,answer->amount);
    }
    if(!rcvd)
        //si on n'a recu aucun message on le signale
        VERB(printf("aucune donnée disponible pour cette adresse\n"));
}


int main(int argc, char ** argv)
{

    //verification des options utilisées, -v
    int c_option;
    while( (c_option = getopt(argc, argv, "v")) != EOF )
    {
        switch(c_option)
        {
            case 'v':
                verbose = true;
                break;
            case '?':
                break;
            default:
                break;
        }
    }

    if( argc-optind+1 < 5 || argc-optind+1 > 7 || argc-optind+1==6)
    {
        fprintf(stderr,"nb arguments\n");
        exit_usage_client();
    }

    //récuperation des arguments du programme

    char * server_ip = argv[1+optind-1];
    char *  server_port = argv[2+optind-1];
    char * cmd = argv[3+optind-1];
    string fromAddr = argv[4+optind-1];
    string toAddr;
    double amount;
    if(strncmp(cmd,"transaction",11)==0)
    {
        toAddr = argv[5+optind-1];
        amount = atof(argv[6+optind-1]);
    }
    else if(strncmp(cmd,"balance",7)!=0)
    {
        fprintf(stderr,"bad key word\n");
        exit_usage_client();
    }


    int sockfd;
    int status;
    bool sock_success = false;
    struct addrinfo hints, *server, *server_info;
    memset(&hints, 0, sizeof hints);

    //utilisation d'IPv6 et UDP
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    //recuperation des informations sur le server
    if ((status = getaddrinfo(server_ip, server_port,
                    &hints, &server)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    //ouverture d'une socket pour communiquer avec le server
    server_info = server;
    while((server_info != NULL) && !sock_success)
    {
        if ((sockfd = socket(server_info->ai_family, server_info->ai_socktype,
                        server_info->ai_protocol)) == -1)
        {
            perror("socket:");
            sock_success = false;
            server_info = server_info->ai_next;
        }
        else
            sock_success = true;
    }
    if (server_info == NULL)
    {
        fputs("Création de socket impossible", stderr);
        return 2;
    }

    /******programme*****/


    if(strncmp(cmd,"transaction",11)==0)
    {
        //envoi du message put
        put_transaction(sockfd, server_info, fromAddr, toAddr, amount);
    }
    else if(strncmp(cmd,"balance",7)==0)
    {
        //envoi du message get, attente de réponse, et affichage de résultat
        get_balance(sockfd, server_info, fromAddr);
    }

    freeaddrinfo(server);
    close(sockfd);
    exit(EXIT_SUCCESS);
}
