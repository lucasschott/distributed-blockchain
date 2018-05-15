/**
 * \file server.c
 * \author Lucas Schott et Juan Manuel Torres Garcia
 */

#include "distributed_server.h"

//variable err pour tester le retour des fonctions fthread
int err;

//variable end_signal modifié lors de la reception du signal SIGINT
volatile bool end_signal = false;

//variable indiquant si l'option verbose a été activé lors du lancement du
//programme ou non
bool verbose = false;

//adresse de la racine de la table de hachage
Blockchain * blockchain = NULL;

//adresse du premier element de la table des servers connectés
connected_server * server_table = NULL;

string my_account;


/**
 * \brief les variables globales d'un server:
 * son descripteur de socket
 * son adresse IPv6
 * son numero de port
 */
int my_sockfd;
struct in6_addr my_addr;
in_port_t my_port;

/**
 * les semaphore permettant a un server d'effectuer les operations sur sa table
 * contenant les hash, et sa tables des servers connectés en toute sécurité
 */
sem_t sem_blockchain;
sem_t sem_server;

//interface distribué
class NoeudBloc
{
    public:
        void sendBlockchain(Blockchain blockchain_a)
        {
            Blockchain * newChain = blockchain_a.copyConstructor();
            if(newChain->isChainValid() && blockchain->chain.size()<newChain->chain.size())
            {
                delete(blockchain);
                blockchain = newChain;
            }
        }

        void sendBlock(Block block_a)
        {
            Block * newBlock = block_a.copyConstructor();
            Blockchain * newChain = blockchain->copyConstructor();
            newChain->chain.push_back(newBlock);
            if(newChain->isChainValid() && blockchain->chain.size()<newChain->chain.size())
            {
                delete(blockchain);
                blockchain = newChain;
            }
        }
};



/**
 * usage du programme server:
 * ./server <adresse du server> <no de port du server>
 * ou
 * ./server <adresse du server> <no de port du server>
 * <adresse d'un sever auquel se connecter< <port de ce server>
 */

void exit_usage_server(void)
{
    fprintf(stderr,"usage: bloc <compte> <ip> <port> [<ip connexion>\
<port connexion>]\n");
    exit(EXIT_FAILURE);
}


/***********************************signaux***********************************/

/**
 * \fn void terminer_processus( __attribute__((unused)) int sig )
 * \brief met la valeur de end_signal a "true" quand le programme recoit le
 * signal SIGINT, pour terminer proprement le programme et liberant toute 
 * la mémoire alloué
 */
void terminer_processus(__attribute__((unused)) int sig)
{
    end_signal=true;
}

/**
 * \fn void terminer_thread()
 * \brief fermeture du thread a la reception du signal SIGUSR1
 */
void terminer_thread(__attribute__((unused)) int sig)
{
    pthread_exit(EXIT_SUCCESS);;
}

/**
 * \fn void init_signal(void)
 * \brief initialisation des signaux attrapés par le programme
 */
void init_signal(void)
{
    struct sigaction s;

    s.sa_handler = terminer_processus;
    s.sa_flags = SA_RESTART;
    sigemptyset(&s.sa_mask);
    PRIM(sigaction(SIGINT,&s,NULL),"sigaction");
}

/**
 * \fn void init_thread_signal(void)
 * \brief initialisation des signaux attrapés par le thread
 */
void init_thread_signal(void)
{
    struct sigaction s;
    sigemptyset(&s.sa_mask);
    sigaddset(&(s.sa_mask),SIGINT);
    THREAD(pthread_sigmask(SIG_BLOCK,&(s.sa_mask),NULL),"pthread_sigmask");
    s.sa_handler = terminer_thread;
    PRIM(sigaction(SIGUSR1,&s,NULL),"sigaction");
}


/*******************************init socket***********************************/

/**
 * \fn int open_my_socket(int local_port, struct in6_addr ip_addr)
 * \brief ouverture de la socket adapté à l'IPv6 et UDP
 */
int open_my_socket(int local_port, struct in6_addr ip_addr)
{
    int sockfd;

    socklen_t addrlen = sizeof(struct sockaddr_in6);

    struct sockaddr_in6 my_sockaddr;

    // creation de la socket
    if((sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // initialisation des parametres voulu
    my_sockaddr.sin6_family      = AF_INET6;
    my_sockaddr.sin6_port        = htons(local_port);
    int i;
    for(i=0;i<16;i++)
        my_sockaddr.sin6_addr.s6_addr[i] = htons(ip_addr.s6_addr[i]);
    my_sockaddr.sin6_flowinfo    = 0;
    my_sockaddr.sin6_scope_id    = 0;

    // lier la structure sockaddr_in6 avec sa socket
    if(bind(sockfd,(struct sockaddr *) &my_sockaddr,addrlen) == -1)
    {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}



/****************************receive********************************/


/**
 * \fn msg_t * receive_m(int my_sockfd,
 * struct sockaddr_storage ** client_sockaddr, socklen_t ** client_addrlen)
 * \brief attend 1 secondes la reception d'un message sur la socket
 * \return le pointeur sur le message recu
 * \pre pointeur sur un pointeur de structure sockaddr_storage pour pouvoir
 * stocker les coordonnée d'un client pour pouvoir lui répondre ulterieurement
 * \post le pointeur sur le message recu est à liberer après utiliation
 */
msg_t * receive_m(struct sockaddr_storage ** client_sockaddr,
        socklen_t ** client_addrlen)
{
    msg_t * msg = (msg_t*) calloc(1,MSG_MAX_LENGTH);

    **client_addrlen = sizeof(**client_sockaddr);

    struct timeval timeVal;
    fd_set readSet;

    FD_ZERO(&readSet);
    FD_SET(my_sockfd, &readSet);
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;

    //attente une seconde le reception d'un message
    if (select(my_sockfd+1, &readSet, NULL, NULL, &timeVal))
    {
        // reception de la chaine de caracteres
        recvfrom(my_sockfd,msg,MSG_MAX_LENGTH,MSG_DONTWAIT,
                (struct sockaddr*) *client_sockaddr, *client_addrlen);
    }
    else
    {
        //si aucun message n'est recu au bout d'une seconde, le header du
        //message est défini comme vide
        msg->header=EMPTY_HEADER;
    }

    return msg;
}



/****************************send***********************************/


/**
 * \fn int responde_to(int my_sockfd, struct sockaddr_storage * client_sockaddr,
 * socklen_t * client_addrlen, msg_t * msg_snd)
 * \brief envoyer le message msg_snd a l'address client_sockaddr de taille
 * client_addr_len par my_socket
 */
int responde_to(struct sockaddr_storage * client_sockaddr,
        socklen_t * client_addrlen, msg_t * msg_snd)
{
    *client_addrlen = sizeof(*client_sockaddr);

    //definition de la taille du message a envoyer en fonction
    //du header du message
    int size;
    if(msg_snd->header==TRANSACTION_HEADER)
    {size=sizeof(transaction_t);}
    else if(msg_snd->header==ASK_BALANCE_HEADER)
    {size=sizeof(ask_balance_t);}
    else if(msg_snd->header==SEND_BALANCE_HEADER)
    {size=sizeof(send_balance_t);}
    else if(msg_snd->header==CONNECT_HEADER)
    {size=sizeof(connect_t);}
    else if(msg_snd->header==KEEP_HEADER)
    {size=sizeof(keep_t);}
    else if(msg_snd->header==SHARE_BLOCK_HEADER)
    {size=sizeof(share_block_t);}
    else if(msg_snd->header==SHARE_SERVER_HEADER)
    {size=sizeof(share_server_t);}
    else if(msg_snd->header==ASK_BLOCKCHAIN_HEADER)
    {size=sizeof(ask_blockchain_t);}
    else if(msg_snd->header==SHARE_BLOCKCHAIN_HEADER)
    {size=sizeof(share_blockchain_t);}

    //envoi du message 
    if(sendto(my_sockfd,msg_snd, size, 0,
                (const struct sockaddr*) client_sockaddr,
                *client_addrlen) == -1)
    {
        perror("sendto");
        close(my_sockfd);
        exit(EXIT_FAILURE);
    }
    return 0;
}


/**
 * \fn struct addrinfo get_address_info(char * addr,char * port)
 * \brief obtenir les informatiques sur un hote
 * \return struct addrinfo accessible de l'hote
 */
struct addrinfo * get_address_info(char * addr,char * port)
{
    int sockfd;
    int status;
    bool sockSuccess = false;
    struct addrinfo hints,*host_info, *host;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    //obtenir les info a partir de son addres (ou nom de domaine) et du port
    if ((status = getaddrinfo(addr, port,&hints, &host)) != 0)
    {
        return NULL;
    }

    //trouver les infos sur la struct addrinfo accessible
    host_info = host;
    while((host_info != NULL) && !sockSuccess)
    {
        if ((sockfd = socket(host_info->ai_family, host_info->ai_socktype,
                        host_info->ai_protocol)) == -1)
        {
            perror("socket:");
            sockSuccess = false; // Echec ouverture socket
            host_info = host_info->ai_next;// Enregistrement d'adresse suivant
        }
        else // La prise réseau est valide
            sockSuccess = true;
    }
    if (host_info == NULL)
    {
        VERB(fputs("Création de socket impossible", stderr));
        return NULL;
    }

    struct addrinfo * new_host = (struct addrinfo*) malloc(sizeof(struct addrinfo));
    memcpy(new_host,host_info,sizeof(struct addrinfo));
    new_host->ai_next=NULL;

    freeaddrinfo(host);

    return new_host;
}



/**
 * \fn void send_to_server(int my_sockfd,struct in6_addr server_addr,
 * in_port_t server_port,msg_t * msg, int msg_len)
 * \brief envoi d'une message a un autre sever dont on connais l'ip et le port
 */
void send_to_server(struct in6_addr server_addr,
        in_port_t server_port,msg_t * msg, int msg_len)
{
    if(memcmp(&server_addr,&my_addr,sizeof(struct in6_addr))==0
            && server_port==my_port)
    {
        return;
    }

    char * ip = in6_addr_to_char_addr(server_addr);
    char * port = (char*) malloc(6*sizeof(char));
    snprintf(port,6,"%d",server_port);


    struct addrinfo * server_info = get_address_info(ip,port);
    if(server_info==NULL)
    {
        VERB(printf("envoi impossible"));
    }

    if(sendto(my_sockfd,msg,msg_len,0,server_info->ai_addr,
                server_info->ai_addrlen)==-1)
    {
        perror("sendto");
        close(my_sockfd);;
        exit(EXIT_FAILURE);
    }

    free(ip);
    free(port);
    free(server_info);
}



/*************************answer to client*******************************/

/**
 * \fn void answer_to_client(get_t get_rcvd,
 * struct sockaddr_storage * client_sockaddr,
 * socklen_t * client_addrlen)
 * \brief répond a un client dont on connait sa struct sockaddr_storage
 * envoi des ip associés au hash deandé par le client
 */
void answer_to_client(ask_balance_t get_rcvd,
        struct sockaddr_storage * client_sockaddr,
        socklen_t * client_addrlen)
{
    send_balance_t * response_snd = (send_balance_t*) malloc(sizeof(send_balance_t));

    response_snd->header = SEND_BALANCE_HEADER;
    response_snd->amount = blockchain->getBalanceOfAddress(get_rcvd.from_addr);

    //au client, une adresse par message
    fflush(NULL);
    VERB(printf("envoi\n"));
    fflush(NULL);
    responde_to(client_sockaddr,client_addrlen,(msg_t*) response_snd);
    free(response_snd);
}


/**
 * \fn void answer_to_server(get_t get_rcvd,
 * struct sockaddr_storage * client_sockaddr,
 * socklen_t * client_addrlen)
 * \brief repondre a un sever quand celui-ci demande de lui envoyer les
 * adresses associés a un hash, lui envoyer les addresses correspondante
 */
void answer_to_server(
        struct sockaddr_storage * client_sockaddr,
        socklen_t * client_addrlen)
{
    share_block_t * block_snd = (share_block_t*) malloc(sizeof(share_block_t));
    transaction_t * transaction_snd = (transaction_t*) malloc(sizeof(transaction_t));
    share_blockchain_t * blockchain_snd = (share_blockchain_t*) malloc(sizeof(share_blockchain_t));

    transaction_snd->header = TRANSACTION_HEADER;
    block_snd->header = SHARE_BLOCK_HEADER;
    blockchain_snd->header = SHARE_BLOCKCHAIN_HEADER;


    //send blocks
    for(Block * block : blockchain->chain)
    {
        responde_to(client_sockaddr,client_addrlen,
                (msg_t*) blockchain_snd);

        strcpy(block_snd->previous_hash,block->previousHash.c_str());
        block_snd->timestamp=block->timestamp;
        block_snd->nonce=block->nonce;

        fflush(NULL);
        VERB(printf("envoi\n"));
        fflush(NULL);
        responde_to(client_sockaddr,client_addrlen,
                (msg_t*) block_snd);

        //send transations
        for(Transaction * trans : block->transactions)
        {
            strcpy(transaction_snd->from_addr,trans->fromAddress.c_str());
            strcpy(transaction_snd->to_addr,trans->toAddress.c_str());
            transaction_snd->amount = trans->amount;
            
            fflush(NULL);
            VERB(printf("envoi\n"));
            fflush(NULL);
            responde_to(client_sockaddr,client_addrlen,
                    (msg_t*) transaction_snd);
        }
    }

    free(block_snd);
    free(transaction_snd);
}




/**
 * \fn void share_all_servers(int my_sockfd,struct in6_addr addr,
 * in_port_t port)
 * \brief envoyer sa liste de server connu à un autre server
 */
void share_all_servers(struct in6_addr addr, in_port_t port)
{
    if(memcmp(&addr,&my_addr,sizeof(struct in6_addr))==0
            && port==my_port)
    {
        return;
    }
    connected_server * tmp = server_table;
    share_server_t msg;
    while(tmp!=NULL)
    {
        msg.header = SHARE_SERVER_HEADER;
        msg.addr = tmp->addr;
        msg.port = tmp->port;
        send_to_server(addr,port,(msg_t*) &msg,
                sizeof(share_server_t));
        tmp=tmp->next;
    }
}

void * threadMine(void __attribute__((unused)) * arg)
{
    Blockchain * newBlockchain = blockchain->copyConstructor();
    VERB(printf("minage d'un nouveau bloc\n"));
    newBlockchain->minePendingTransactions(my_account);

    PRIM(sem_wait(&sem_blockchain),"sem_wait(sem_blockchain)");
    if(newBlockchain->isChainValid() &&
            (blockchain->chain.size()<newBlockchain->chain.size() 
             || (blockchain->chain.size()==newBlockchain->chain.size()
                 && blockchain->chain.back()->transactions.size()
                 < newBlockchain->chain.back()->transactions.size())))
    {
        VERB(printf("nouveau bloc valide\n"));
        delete(blockchain);
        blockchain = newBlockchain;

        //envoie du block au autres servers
        share_block_t msg;
        msg.header = SHARE_BLOCK_HEADER;
        strncpy(msg.previous_hash,blockchain->chain.back()->previousHash.c_str(),BLOCK_HASH_SIZE);
        msg.timestamp = blockchain->chain.back()->timestamp;
        msg.nonce = blockchain->chain.back()->nonce;

        transaction_t msg_trans;
        msg_trans.header = SHARE_TRANSACTION_HEADER;

        connected_server * server = server_table;
        struct in6_addr client_addr;
        in_port_t client_port;

        while(server!=NULL)
        {
            VERB(printf("envoi block\n"));
            client_addr = server->addr;
            client_port = server->port;
            send_to_server(client_addr,client_port,
                    (msg_t*) &msg,sizeof(share_block_t));

            //envoi de chaque transactions du block
            for(Transaction * trans : blockchain->chain.back()->transactions)
            {
                VERB(printf("envoi transaction\n"));
                strncpy(msg_trans.from_addr,trans->fromAddress.c_str(),PSEUDO_SIZE);
                strncpy(msg_trans.from_addr,trans->toAddress.c_str(),PSEUDO_SIZE);
                msg_trans.amount=trans->amount;
                send_to_server(client_addr,client_port,
                        (msg_t*) &msg_trans,sizeof(transaction_t));
            }
            server = server->next;
        }
    }
    PRIM(sem_post(&sem_blockchain),"sem_post(sem_blockchain)");
    pthread_exit(EXIT_SUCCESS);
}


//thread numero 2 qui va verifier toutes les 20 secondes, si les données de la 
//table dont obsoletes, si les servers connectés donnent toujours des signes
//de vie, et envoi toute les 10 secondes et messages "keep_alive" a tous les
//autres servers connectés
void * fthread(void __attribute__((unused)) * arg)
{
    //iitialisation des signaux pour le thread
    init_thread_signal();

    struct in6_addr client_addr;
    in_port_t client_port;

    keep_t msg_keep;
    msg_keep.header = KEEP_HEADER;
    msg_keep.addr = my_addr;
    msg_keep.port = my_port;

    connected_server * server;

    char * char_addr;

    while(true)
    {

        //envoi des keep alive à tous les servers connectés
        PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
        server = server_table;
        while(server!=NULL)
        {
            char_addr = in6_addr_to_char_addr(server->addr);
            VERB(printf("send keep_alive to addr %s\n", char_addr));
            free(char_addr);
            VERB(printf("port %d\n", server->port));

            client_addr = server->addr;
            client_port = server->port;
            send_to_server(client_addr,client_port,
                    (msg_t*) &msg_keep,sizeof(keep_t));
            server = server->next;
        }
        PRIM(sem_post(&sem_server),"sem_post(sem_server)");

        //supprimer tous les servers morts (ce qu'in n'ont pas renvoyé de
        //keep_alive depuis plus de 30 secondes
        VERB(printf("suppression des servers morts\n"));

        PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
        server_table=suppression_dead_server(server_table);
        PRIM(sem_post(&sem_server),"sem_post(sem_server)");

        sleep(10);
    }

    VERB(printf("fin thread boucle\n"));

    pthread_exit(EXIT_SUCCESS);
}



bool receiv_block(share_block_t * share_block_rcvd)
{
    bool rcvd=false;
    fd_set read_set;
    struct timeval time_val;

    FD_ZERO(&read_set);
    FD_SET(my_sockfd, &read_set);
    time_val.tv_sec = 0;
    time_val.tv_usec = 10000;

    bool not_share=false;

    Block * newBlock = new Block(
            share_block_rcvd->previous_hash,
            share_block_rcvd->timestamp,
            share_block_rcvd->nonce);

    transaction_t * transaction = (transaction_t *) malloc(sizeof(transaction_t));
    while (select(my_sockfd+1, &read_set, NULL, NULL, &time_val))
    {
        //tant qu'on recoit des messages on affiche le resultat
        if (recv(my_sockfd, transaction, MSG_MAX_LENGTH, 0) == -1)
        {
            perror("recv:");
            close(my_sockfd);
            exit(EXIT_FAILURE);
        }
        if(transaction->header!=SHARE_TRANSACTION_HEADER)
        {
            VERB(printf("reception transaction externe"));
            not_share=true;
            break;
        }
        VERB(printf("réception transaction block\n"));
        rcvd = true;
        Transaction * trans = new Transaction(
                transaction->from_addr,
                transaction->to_addr,
                transaction->amount);
        newBlock->transactions.push_back(trans);
    }
    if(not_share)
        return false;
    if(!rcvd)
        //si on n'a recu aucun message on le signale
        VERB(printf("aucune adresse disponible\n"));

    VERB(printf("verif  du nouveau bloc\n"));
    newBlock->calculateHash();

    //verifier l'integrité de la donne et inserer
    //la donnée recu dans la blockchain

    Blockchain * newBlockchain = blockchain->copyConstructor();
    newBlockchain->chain.push_back(newBlock);

    PRIM(sem_wait(&sem_blockchain),"sem_wait(sem_blockchain)");
    if(newBlockchain->isChainValid() &&
            (blockchain->chain.size()<newBlockchain->chain.size() 
             || (blockchain->chain.size()==newBlockchain->chain.size()
                 && blockchain->chain.back()->transactions.size()
                 < newBlockchain->chain.back()->transactions.size())))
    {
        VERB(printf("nouveau bloc valide\n"));
        delete(blockchain);
        blockchain = newBlockchain;
    }
    return true;
}




/********************************main********************************/

int main(int argc, char ** argv)
{
    init_signal();

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

    //lecture des arguments
    if( argc-optind+1 < 4 || argc-optind+1 == 5 || argc-optind+1 >6 )
        exit_usage_server();

    my_account = argv[1+optind-1];

    char *  my_addr_name = argv[2+optind-1];
    
    char * my_char_port = argv[3+optind-1];
    my_port = atoi(my_char_port);

    //obtenir les info sur son adresse (utile si l'on utilise un nom de domaine
    //ou localhost pour identifier son adresse
    struct addrinfo * server_info = get_address_info(my_addr_name,my_char_port);
    if(server_info==NULL)
        exit_usage_server();
    char my_ip[256];
    getnameinfo(server_info->ai_addr, server_info->ai_addrlen, my_ip,
            sizeof(my_ip), NULL, 0, NI_NUMERICHOST);
    free(server_info);

    struct in6_addr * tmp_my_addr = char_to_in6_addr(my_ip);
    my_addr = * tmp_my_addr;
    free(tmp_my_addr);

    my_sockfd = open_my_socket(my_port, my_addr);

    msg_t * msg_rcvd = NULL;

    blockchain = new Blockchain(2,50);

    char * char_addr = in6_addr_to_char_addr(my_addr);
    VERB(printf("mon addresse est : %s\n", char_addr));
    VERB(printf("mon port est : %d\n", my_port));

    //initialisation des semaphores
    PRIM(sem_init(&sem_server,1,1),"sem_init(sem_server)");
    PRIM(sem_init(&sem_blockchain,1,1),"sem_init(sem_blockchain)");


    try
    {
        // Initialize RCF.
        RCF::RcfInitDeinit rcfInit;
        // Instantiate a RCF server.
        RCF::RcfServer server(RCF::TcpEndpoint(char_addr, my_port+1));
        // Bind the I_PrintService interface.
        NoeudBloc noeudBloc;
        server.bind<I_NoeudBloc>(noeudBloc);
        // Start the server.
        server.start();
        cout << char_addr << " " << my_port+1 << endl;
    }
    catch ( const RCF::Exception & e )
    {
        std::cout << "Error: " << e.getErrorString() << std::endl;
    }



    //s'il y a 6 arguments c'est que l'on souhaite que le server se connecte
    //un a autre sever
    if(argc-optind+1 == 6)
    {
        char * connected_server_ip = argv[4+optind-1];
        char * connected_server_port = argv[5+optind-1];
        if(strtol(connected_server_port,NULL,10)==0)
            exit_usage_server();

        //obtenir les infos sur le server auquel se conencter
        server_info = get_address_info(connected_server_ip,
                connected_server_port);
        if(server_info==NULL)
            exit_usage_server();
        char ip_server[256];
        getnameinfo(server_info->ai_addr, server_info->ai_addrlen, ip_server,
                sizeof(ip_server),NULL, 0, NI_NUMERICHOST);

        free(server_info);
        struct in6_addr * tmp_server_ip = char_to_in6_addr(ip_server);
        struct in6_addr server_ip = * tmp_server_ip;
        free(tmp_server_ip);
        in_port_t server_port = strtol(connected_server_port,NULL,10);
        char * char_other_addr = in6_addr_to_char_addr(server_ip);

        VERB(printf("demande de connection a l'adresse %s\n", char_other_addr));
        VERB(printf("port %d\n", server_port));

        connect_t msg_connect;
        msg_connect.header = CONNECT_HEADER;
        msg_connect.addr = my_addr;
        msg_connect.port = my_port;

        //envoyer un message connect a ce server
        send_to_server(server_ip,server_port,
                (msg_t *) &msg_connect,sizeof(connect_t));

        //on l'ajoute dans notre table des servers connectés
        PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
        server_table = insert_server(server_table,
                server_ip,server_port);
        PRIM(sem_post(&sem_server),"sem_post(sem_server)");

        free(char_other_addr);
    }

    //stockage des sockaddr des clients et server lors de la receptiond d'un
    //message,
    //utilisé pour pour pouvoir répondre au client
    struct sockaddr_storage * client_sockaddr =
        (struct sockaddr_storage*) malloc(sizeof(struct sockaddr_storage));
    socklen_t * client_addrlen = (socklen_t*) malloc(sizeof(socklen_t));


    /**********mise en place du nouveau thread:**********/
    //nouveau thread : boucle jusqu'a l'arret du server, envoi des keep_alive,
    //vérifi l'obsolescence des donnée, et des servers.

    pthread_t * tid = (pthread_t*) malloc(sizeof(pthread_t));
    THREAD(pthread_create(tid,NULL,fthread,NULL),"pthread_create");

    pthread_t * mine = (pthread_t*) malloc(sizeof(pthread_t));

    //thread courant: repond aux requetes


    msg_rcvd=receive_m(&client_sockaddr,&client_addrlen);

    //boucle d'actions et de réponse aux requetes de clients et de servers
    while(!end_signal)
    {
        //si le message recu est un message put 
        if(msg_rcvd->header==TRANSACTION_HEADER)
        {
            transaction_t * get_rcvd = (transaction_t *) msg_rcvd;
            VERB(printf("reception transaction de %s à %s, montant %lf\n",
                        get_rcvd->from_addr,get_rcvd->to_addr,get_rcvd->amount));
            //ajouter la transaction a la liste d'attente de la blockchain
            blockchain->createTransaction(
                    new Transaction(
                        get_rcvd->from_addr,get_rcvd->to_addr,get_rcvd->amount
                        )
                    );
            if(blockchain->pendingTransactions.size()>=10)
            {
                //thread mine pending transactions
                mine = (pthread_t*) malloc(sizeof(pthread_t));
                THREAD(pthread_create(mine,NULL,threadMine,NULL),"pthread_create");
            }
        }
        else if(msg_rcvd->header==ASK_BALANCE_HEADER)
        {
            ask_balance_t * get_rcvd = (ask_balance_t *) msg_rcvd;

            VERB(printf("reception ask balance %s\n",get_rcvd->from_addr));
            
            //repondre a la requete get d'un client et lui envoyer des adresses
            PRIM(sem_wait(&sem_blockchain),"sem_wait(sem_blockchain)");
            answer_to_client(*get_rcvd,client_sockaddr,client_addrlen);
            PRIM(sem_post(&sem_blockchain),"sem_post(sem_blockchain)");
        }
        else if(msg_rcvd->header==CONNECT_HEADER)
        {
            //message d'un server qui tente de se connecter
            connect_t * connect_rcvd = (connect_t*) msg_rcvd;

            VERB(printf("reception connect\n"));

            if(memcmp(&connect_rcvd->addr,&my_addr,sizeof(struct in6_addr))==0
                    && connect_rcvd->port==my_port)
            {
                VERB(printf("auto_connect\n"));
            }
            else
            {
                PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                bool connue = is_known(connect_rcvd->addr,
                        connect_rcvd->port,server_table);
                PRIM(sem_post(&sem_server),"sem_post(sem_server)");

                if(!connue)
                {
                    // si le server est inconnu
                    VERB(print_connection(connect_rcvd->addr,
                                connect_rcvd->port));

                    //on lui partage notre liste de server connecté pour qu'il
                    //puisse s'y connecter
                    PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                    share_all_servers(connect_rcvd->addr,
                            connect_rcvd->port);
                    PRIM(sem_post(&sem_server),"sem_post(sem_server)");

                    VERB(printf("insertion du nouveau server dans le base\n"));

                    //on l'ajoute dans notre table des servers connectés
                    PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                    server_table = insert_server(server_table,
                            connect_rcvd->addr,connect_rcvd->port);
                    PRIM(sem_post(&sem_server),"sem_post(sem_server)");

                    VERB(printf("se connecter à ce nouveau server\n"));

                    //lui repondre en lui envoyant aussi un message connect
                    connect_t msg_connect;
                    msg_connect.header = CONNECT_HEADER;
                    msg_connect.addr = my_addr;
                    msg_connect.port = my_port;
                    responde_to(client_sockaddr,client_addrlen,
                            (msg_t*) &msg_connect);

                    VERB(printf("lui partager la blockchain\n"));

                    connected_server * server_c = get_server(connect_rcvd->addr,connect_rcvd->port,server_table);

                    char * addr_c = in6_addr_to_char_addr(server_c->addr);
                    try
                    {
                        cout << 1 << endl;
                        RCF::RcfInitDeinit rcfInit;
                        cout << 2 << endl;
                        RcfClient<I_NoeudBloc> client(RCF::TcpEndpoint(addr_c,((int)server_c->port)+1));
                        cout << 3 << endl;
                        client.sendBlockchain(RCF::Oneway,*blockchain);
                        cout << 4 << endl;
                    }
                    catch ( const RCF::Exception & e )
                    {
                        cout << addr_c << " " << server_c->port+1 <<endl;
                        std::cout << "Error: " << e.getErrorString() << std::endl;
                    }

                }
                else
                {
                    VERB(printf("server déja connecté\n"));
                }
            }
        }
        else if(msg_rcvd->header==KEEP_HEADER)
        {
            keep_t * keep_rcvd = (keep_t*) msg_rcvd;

            VERB(print_keep_recv(keep_rcvd->addr,keep_rcvd->port));
            
            if(memcmp(&keep_rcvd->addr,&my_addr,sizeof(struct in6_addr))==0
                    && keep_rcvd->port==my_port)
            {
                fprintf(stderr,"self keep\n");
            }
            else
            {
                //mettre a jour la date de dernier keep_alive du server
                //connecté qui nous envoi ce message
                PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                keep_alive_server(server_table,keep_rcvd->addr,keep_rcvd->port);
                PRIM(sem_post(&sem_server),"sem_post(sem_server)");
            }
        }
        else if(msg_rcvd->header==SHARE_SERVER_HEADER)
        {
            share_server_t * share_rcvd = (share_server_t *) msg_rcvd;

            //recevoir des coordonnées d'un server auquel se connecter

            VERB(printf("reception share server\n"));

            PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
            bool connue = is_known(share_rcvd->addr,
                    share_rcvd->port,server_table);
            PRIM(sem_post(&sem_server),"sem_post(sem_server)");

            if(!connue)
            {
                VERB(printf("insertion du server dans la base\n"));

                //ajouter le server dans la tabel des severs
                PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                server_table = insert_server(server_table,share_rcvd->addr,
                        share_rcvd->port);
                PRIM(sem_post(&sem_server),"sem_post(sem_server)");

                //lui envoyer un message pour se connecter
                connect_t msg_connect;
                msg_connect.header = CONNECT_HEADER;
                msg_connect.addr = my_addr;
                msg_connect.port = my_port;
                send_to_server(share_rcvd->addr,share_rcvd->port,
                        (msg_t*) &msg_connect,sizeof(connect_t));
            }
        }
        else if(msg_rcvd->header==ASK_BLOCKCHAIN_HEADER)
        {
            //TODO
        }
        else if(msg_rcvd->header==EMPTY_HEADER)
        {
        }

        free(msg_rcvd);

        msg_rcvd=receive_m(&client_sockaddr,&client_addrlen);
    }

    //fin de l'execution de la boucle si le processus recoit le signal SIGINT

    //envoi du signal SIGUSR1 au 2eme thread pour lui dire de s'arreter
    THREAD(pthread_kill(*tid,SIGUSR1),"pthread_kill");;
    THREAD(pthread_kill(*mine,SIGUSR1),"pthread_kill");;
    THREAD(pthread_join(*tid,NULL),"pthread_join");

    //liberer toute la mémoire encore alloué
    free(client_addrlen);
    free(client_sockaddr);
    free_server_tab(server_table);
    free(tid);
    free(msg_rcvd);;
    free(char_addr);

    PRIM(sem_destroy(&sem_server),"sem_destroy(sem_server)");
    PRIM(sem_destroy(&sem_blockchain),"sem_destroy(sem_blockchain)");

    printf("\n");
    VERB(printf("arrêt du server\n"));

    exit(EXIT_SUCCESS);
}
