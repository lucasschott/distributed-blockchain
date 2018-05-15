/**
 * \file distributed_server.h
 * \author Lucas Schott et Juan Manuel Torres Garcia
 * \brief définition des fonctinos gérant la connexion des servers
 * entre eux, ainsi que de la distribution de la table
 */

#ifndef DISTRIBUTED_SERVER_H
#define DISTRIBUTED_SERVER_H

#include "client_server.h"
#include "blockchain.h"
#include "block.h"
#include "transaction.h"
#include "addr_and_hash.h"
#include "affichage.h"
#include <iterator>
#include <iostream>
#include <RCF/RCF.hpp>
#include <SF/vector.hpp>
#include <SF/string.hpp>
#include <SF/Archive.hpp>



/*
 * headers des messages envoyés entre servers
 */
//message pour se connecter à un server
#define CONNECT_HEADER 4
//message pour partager un block
#define SHARE_BLOCK_HEADER 5
//message pour envoyer à un server une adresses d'un autre servers
#define SHARE_SERVER_HEADER 6
//message pour garder a connexion entre deux server
#define KEEP_HEADER 7
//message pour demander à un server sa blockchain
#define ASK_BLOCKCHAIN_HEADER 8
//envoyer une transacation deja traité dans un block
#define SHARE_TRANSACTION_HEADER 9
#define SHARE_BLOCKCHAIN_HEADER 10


//structures de messages envoyé par le réseau
//*******************************************

// Define the I_PrintService RCF interface.
RCF_BEGIN(I_NoeudBloc, "I_NoeudBloc")
    RCF_METHOD_V1(void, sendBlockchain, Blockchain)
    RCF_METHOD_V1(void, sendBlock, Block)
RCF_END(I_NoeudBloc)




//structure des messages ou un server A envoi les coodonnées d'un server B à un
//server C
typedef struct syn_t{
    uint16_t header;
    struct in6_addr addr;
    in_port_t port;
}share_server_t, connect_t, keep_t, ask_blockchain_t;

typedef struct block_t{
    uint16_t header;
    char previous_hash[BLOCK_HASH_SIZE];
    time_t timestamp;
    long nonce;
}share_block_t;


//stockage de données pour le maintien des
//connections server**********************


//structrure contenant les informations sur les autres servers qu'un server
//possède (n'est pas à envoyer par le réseau)
typedef struct connected_server{
    struct in6_addr addr;
    in_port_t port;
    time_t last_keep_alive;
    struct connected_server * next;
}connected_server;


//table de server

connected_server * next_server(connected_server * serv);
connected_server * insert_server(connected_server * list_head,
        struct in6_addr addr, in_port_t port);
int count_server(connected_server * list);
connected_server *  suppression_server(connected_server * list_head,
        struct in6_addr addr, in_port_t port);
void  keep_alive_server(connected_server * list_head,
        struct in6_addr addr, in_port_t port);
connected_server * suppression_dead_server(connected_server * server_table);
bool is_known(struct in6_addr addr, in_port_t port,
        connected_server * server_table);
void free_server_tab(connected_server * list);
connected_server * get_server(struct in6_addr addr, in_port_t port, connected_server * server_table);


#endif
