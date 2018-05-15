/**
 * \file addr_and_hash.c
 * \author Lucas Schott et Juan Manuel Torres Garcia
 */

#include <regex.h>
#include "addr_and_hash.h"


/**
 * \var hash_max[HASH_LENGTH]
 * \brief la valeur maximal d'un hash
 */
uint16_t hash_max[HASH_LENGTH] = {UINT16_MAX, UINT16_MAX, UINT16_MAX,
    UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX,
    UINT16_MAX, UINT16_MAX, UINT16_MAX,UINT16_MAX, UINT16_MAX, UINT16_MAX,
    UINT16_MAX, UINT16_MAX, UINT16_MAX,UINT16_MAX, UINT16_MAX, UINT16_MAX,
    UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX,
    UINT16_MAX, UINT16_MAX, UINT16_MAX,UINT16_MAX, UINT16_MAX, UINT16_MAX,
    UINT16_MAX, UINT16_MAX, UINT16_MAX};


/**
 * \var hash_min[HASH_LENGTH]
 * \brief la valeur minimal d'un hash
 */
uint16_t hash_min[HASH_LENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


/****************************COMPARAISON**************************************/
/*****************************************************************************/

/**
 * \fn int compare_hash(uint16_t * h1, uint16_t * h2)
 * \param h1 pointeur sur un hash
 * \param h2 pointeur sur un hash
 * \brief comparaison de deux hashs
 *
 * \return renvoi 0 si h1 = h2
 * renvoi -1 si h1 < h2
 * renvoi +1 si h1 > h2
 */
int compare_hash(uint16_t * h1, uint16_t * h2)
{
    int i,res=0;
    for(i=0;i<HASH_LENGTH;i++)
    {
        if(h1[i]<h2[i])
        {
            res = -1;
            break;
        }
        else if(h1[i]>h2[i])
        {
            res = 1;
            break;
        }
    }
    return res;
}

/**
 * \fn int compare_addr(struct in6_addr a1, struct in6_addr a2)
 * \param a1 une adresse IPV6
 * \param a2 une adresse IPv6
 * \brief comparaison de deux adresses
 *
 * \return renvoi 0 si a = a2
 * renvoi -1 si a1 < a
 * renvoi +1 si a1 > a2
 */
int compare_addr(struct in6_addr a1, struct in6_addr a2)
{
    int i,res=0;
    for(i=0;i<ADDR_LENGTH;i++)
    {
        if(a1.s6_addr[i]<a2.s6_addr[i])
        {
            res = -1;
            break;
        }
        else if(a1.s6_addr[i]>a2.s6_addr[i])
        {
            res = 1;
            break;
        }
    }
    return res;
}



/****************************CONVERSION***************************************/
/*****************************************************************************/

/*************ADDR**********/

/**
 * \fn char * in6_addr_to_char_addr(struct in6_addr ip)
 * \param ip addresse ip
 * \brief conversion de l'adresse ip stocké sous forme de nombre en
 * une addresse ip stocké sous forme de chaine de caractere
 *
 * \return la chaine de caractere representant l'addresse IPv6
 * \post le pointeur retourné est a liberer apres utilisation
 */
char * in6_addr_to_char_addr(struct in6_addr ip)
{
    char * addr = (char*) malloc(CHAR_ADDR_LENGTH*sizeof(char));
    //utilisation de inet_ntop pour ecire les addresse au format standard IP
    inet_ntop(AF_INET6, &ip, addr, CHAR_ADDR_LENGTH*sizeof(char));
    return addr;
}

/**
 * \fn struct in6_addr char_to_in6_addr(char * addr)
 * \param addr chaine de caractere representant l'addresse IPv6
 * \brief donne l'addresse IPv6 a partir d'un chaine de caractere la
 * representant
 *
 * \return l'address IPv6 correspondant
 */
struct in6_addr * char_to_in6_addr(char * addr)
{

    int reg_err;
    regex_t ipv6_regex;

    reg_err = regcomp(&ipv6_regex,"(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}\%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))",REG_EXTENDED | REG_ICASE | REG_NOSUB);
    if(reg_err==0)
    {
        int match;
        match = regexec(&ipv6_regex,addr,0,NULL,0);
        if(match==0)
        {
            struct in6_addr * ip = (struct in6_addr*) malloc(sizeof(struct in6_addr));
            //utilisation de la fonction inet_pton pour comprendre tout les formats
            //standard d'ecriture d'un addresse IP
            if(inet_pton(AF_INET6,addr,ip) == -1)
            {
                free(ip);
                regfree(&ipv6_regex);
                perror("inet_pton");
                exit(EXIT_FAILURE);
            }
            regfree(&ipv6_regex);
            return  ip;
        }
    }
    regfree(&ipv6_regex);
    return NULL;
}


/*************HASH**********/

/**
 * \fn char * uint16_t_to_char_hash(uint16_t * hash)
 * \param hash pointeur sur un hash
 * \brief conversion d'un hahs stocké sous forme de nombre en
 * un hash stocké sous forme de chaine de caractere
 *
 * \return chaine de caractere representant le hash en hexadecimal
 * \post le pointeur retourné est à liberer apres utilisation
 */
char * uint16_t_to_char_hash(uint16_t * hash)
{
    char * h = (char*) malloc(sizeof(char)*145);
    h[0]='\0';
    char * tmp = (char*) malloc(sizeof(char)*5);
    int i;
    for(i=0;i<HASH_LENGTH;i++)
    {
        snprintf(tmp,5,"%04x",hash[i]);
        h = strncat(h,tmp,5);
    }
    free(tmp);
    return h;
}

/**
 * \fn uint16_t * char_to_uint16_t_hash(char * hash)
 * \param hash pointeur sur un hash
 * \brief convertit une chaine de caracteres representant un hashs en un
 * nombre socké dans un tableau de 36 entiers sur 16 bits
 *
 * \return pointeur sur le hash
 * \post le pinteur retourné est à liberer après utilisation
 */
uint16_t * char_to_uint16_t_hash(char * hash)
{
    int reg_err;
    regex_t ipv6_regex;

    reg_err = regcomp(&ipv6_regex,"[^0-9a-fA-F]",
            REG_EXTENDED | REG_ICASE | REG_NOSUB);
    if(reg_err==0)
    {
        int match;
        match = regexec(&ipv6_regex,hash,0,NULL,0);
        if(match==REG_NOMATCH)
        {
            int len = strlen(hash);
            int i,j;
            char * tmp = (char*) malloc(5*sizeof(char));
            
            uint16_t * h = (uint16_t*) malloc(sizeof(uint16_t)*HASH_LENGTH);
            if(len%4==0)
            {
                for(i=0;i<HASH_LENGTH-(len/4);i++)
                {
                    h[i]=0;
                }
                j=0;
                for(i=HASH_LENGTH-(len/4);i<HASH_LENGTH;i++)
                {
                    strncpy(tmp,&hash[j*4],4);
                    tmp[4]='\0';
                    h[i]=strtol(tmp,NULL,16);
                    j++;
                }
            }
            else
            {
                for(i=0;i<HASH_LENGTH-(len/4+1);i++)
                {
                    h[i]=0;
                }
                j=0;
                for(i=HASH_LENGTH-(len/4+1);i<HASH_LENGTH;i++)
                {
                    if(j==0)
                    {
                        strncpy(tmp,&hash[0],len%4);
                        tmp[len%4]='\0';
                    }
                    else
                    {
                        strncpy(tmp,&hash[len%4+(j-1)*4],4);
                        tmp[4]='\0';
                    }
                    h[i]=strtol(tmp,NULL,16);
                    j++;
                }
            }
            return  h;
        }
        else
            return NULL;
    }
    else
        return NULL;
}




/****************************PRINT********************************************/
/*****************************************************************************/

/**
 * \fn void print_hash(uint16_t * hash)
 * \brief affiche a l'ecran le hash
 */
void print_hash(uint16_t * hash)
{
    if(hash==NULL)
        return;
    int i;
    for(i=0;i<HASH_LENGTH;i++)
    {
        printf("%04x",hash[i]);
    }
    printf("\n");
}

/**
 * \fn void print_address(struct in6_addr addr)
 * \brief affiche a l'ecran une address IPv6
 */
void print_address(struct in6_addr addr)
{
    char * str = (char*) malloc(CHAR_ADDR_LENGTH*sizeof(char));
    str = in6_addr_to_char_addr(addr);
    printf("%s\n", str);
    free(str);
}
