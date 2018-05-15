/**
 * \file addr_and_hash.h
 * \author Lucas Schott et Juan Manuel Torres Garcia
 * \brief Bibliotheque de fonctions sur les hashs et les addresses
 */

#ifndef ADDR_AND_HASH_H
#define ADDR_AND_HASH_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/** 
 * \def HASH_LENGTH
 * \brief la taille d'un hash en nombre d'entiers de 16 bits
 */
#define HASH_LENGTH 36
/**
 * \def CHAR_HASH_LENGTH
 * \brief le nombre de caractères maximal pour ecrire un hash en hexadecimal
 */
#define CHAR_HASH_LENGTH 144
/**
 * \def ADDR_LENGTH 16
 * \brief la taille d'une addresse en nombre d'octets (ou d'entier de 8 bits) 
 */
#define ADDR_LENGTH 16
/**
 * \def CHAR_ADDR_LENGTH
 * \brief le nombre de caractères maximal pour ecrire une address au format
 * standard IPv6
 */
#define CHAR_ADDR_LENGTH 40

/****************************COMPARAISON***************************************/


int compare_hash(uint16_t * h1, uint16_t * h2);
int compare_addr(struct in6_addr a1, struct in6_addr a2);


/****************************CONVERSION***************************************/


/*************ADDR**********/

char * in6_addr_to_char_addr(struct in6_addr ip);
struct in6_addr * char_to_in6_addr(char * addr);

/*************HASH**********/

char * uint16_t_to_char_hash(uint16_t * hash);
uint16_t * char_to_uint16_t_hash(char * hash);


/*****************************AFFICHAGES**************************************/


void print_hash(uint16_t * hash);
void print_address(struct in6_addr adr);


#endif
