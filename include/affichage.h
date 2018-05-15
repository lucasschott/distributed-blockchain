/**
 * \affichage.h
 * \author Lucas Schott et Juan Manuel Torres Garcia
 * \brief affichages a l'ecran d'informations sur la synchronisation
 * des servers
 */

#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include "addr_and_hash.h"

void print_put_reception(uint16_t * hash, struct in6_addr addr);
void print_connection(struct in6_addr addr, in_port_t port);
void print_keep_recv(struct in6_addr addr, in_port_t port);
void print_share_data_rcv(uint16_t * hash, struct in6_addr addr);
void print_share_put_rcv(uint16_t * hash, struct in6_addr addr);

#endif
