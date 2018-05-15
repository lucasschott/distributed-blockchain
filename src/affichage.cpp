/**
 * \file affichage.c
 * \author Lucas Schott et Juan Manuel Torres Garcia
 */

#include "affichage.h"

//affichage de la reception d'une requete put, ainsi que de qui on l'a recu
void print_put_reception(uint16_t * hash, struct in6_addr addr)
{
    char * char_addr;
    char * char_hash;
    printf("reception put:\n");
    char_hash = uint16_t_to_char_hash(hash);
    char_addr = in6_addr_to_char_addr(addr);
    printf("%s\n", char_hash);
    printf("%s\n", char_addr);
    free(char_hash);
    free(char_addr);
}


//affichage de la reception d'une requete connect, ainsi que de qui on l'a recu
void print_connection(struct in6_addr addr, in_port_t port)
{
    char * char_addr;
    printf("server inconnu jusqu'a present\n");
    char_addr = in6_addr_to_char_addr(addr);
    printf("connection de l'address: %s\n",char_addr);
    printf("de port: %d\n", port);
    free(char_addr);
    printf("partage des infos servers\n");
}

//affichage de la reception d'une requete keep_alive, ainsi que de qui on
//l'a recu
void(print_keep_recv(struct in6_addr addr, in_port_t port))
{
    char * char_addr;
    char_addr = in6_addr_to_char_addr(addr);
    printf("reception message keep alive de %s\n%d\n",
            char_addr,port);
    free(char_addr);
}

//affichage de la reception d'une requete share data, ainsi que de qui
//on l'a recu
void print_share_data_rcv(uint16_t * hash, struct in6_addr addr)
{
    char * char_hash = uint16_t_to_char_hash(hash);
    char * char_addr = in6_addr_to_char_addr(addr);
    printf("reception share data:\n");
    printf("%s\n", char_hash);
    printf("%s\n", char_addr);
    free(char_hash);
    free(char_addr);
}

//affichage de la reception d'une requete share_put, ainsi que de qui
//on l'a recu
void print_share_put_rcv(uint16_t * hash, struct in6_addr addr)
{
    char * char_hash = uint16_t_to_char_hash(hash);
    char * char_addr = in6_addr_to_char_addr(addr);
    printf("reception share put:\n");
    printf("%s\n", char_hash);
    printf("%s\n", char_addr);
    free(char_hash);
    free(char_addr);
}
