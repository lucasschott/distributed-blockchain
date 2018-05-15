/**
 * \file client_server.h
 * \author Lucas Schott et Juan Manuel Torres Garcia
 * \brief fichier contenant les stuctures et d'autres informations dont les
 * clients et les servers vont se servir. Comme les header des messages qu'ils
 * s'envoient, les structures de ces messages etc.
 */

#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#include <unistd.h>
#include <netdb.h>

#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

#include <string>

using namespace std;

//taille max des message sur le réseaux, en plafonnnant le nombre d'adresse
//envoyé par le server a 20
#define MSG_MAX_LENGTH 150
#define PSEUDO_SIZE 50
#define BLOCK_HASH_SIZE 65


/**
 * definitions des headers des messages qui circulent par le réseau entre
 * les clients et les servers
 */

//vide 
#define EMPTY_HEADER 0
//quand un client envoie une transaction a une server
#define TRANSACTION_HEADER 1
//quand un client demande sa balance à une server
#define ASK_BALANCE_HEADER 2
//and un server envoi la balance du client a celui ci
#define SEND_BALANCE_HEADER 3


//macro pour tester le retour des primitives systeme
#define PRIM(r, msg) do{if((r)==-1){perror(msg);exit(EXIT_FAILURE);}}while(0)
//marco pour tester le retour des fonctions pthread
#define THREAD(r,msg) do{if((err = r)!=0){fprintf(stderr,strerror(err));\
    exit(EXIT_FAILURE);}}while(0)
//maco pour effectuer une action uniquement si l'option verbose à été donnée en 
//parametre au programme
#define VERB(action) do{if(verbose){action;}}while(0)



//structures de messages envoyé par le réseau
//*******************************************

//strucure d'un message contenant juste le header
typedef struct msg_t{
    uint16_t header;
}msg_t, ask_blockchain_t,share_blockchain_t;

//structure d'un message contenant une addresse source une addresse destination
//et un montant
typedef struct transaction_t{
    uint16_t header;
    char from_addr[PSEUDO_SIZE];
    char to_addr[PSEUDO_SIZE];
    double amount;
}transaction_t;

//structure d'un message contenant un hash et une adresse
typedef struct ask_balance_t{
    uint16_t header;
    char from_addr[PSEUDO_SIZE];
}ask_balance_t;

//structure d'un message contenant un hash et une adresse
typedef struct send_balance_t{
    uint16_t header;
    double amount;
}send_balance_t;


#endif
