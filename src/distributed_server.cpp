/**
 * \file distributed_server.c
 * \author Lucas Schott et Juan Manuel Torres Garcia
 */

#include "distributed_server.h"


/**
 * \brief les variables globales d'un server:
 * son descripteur de socket
 * son adresse IPv6
 * son numero de port
 */
extern int my_sockfd;
extern struct in6_addr my_addr;
extern in_port_t my_port;

/**
 * les semaphore permettant a un server d'effectuer les operations sur sa table
 * contenant les hash, et sa tables des servers connectés en toute sécurité
 */
extern sem_t sem_blockchain;
extern sem_t sem_server;



/***********************CONNECTED SERVER TAB**********************************/
/*****************************************************************************/

/**
 * fonctions de manipulation de la table des servers connectés:
 * ajout de server
 * suppression de server
 * verification du maintient de la connexion des server
 * compter le nombre de servers connectés
 * mettre a jour la date de dernier message de maintient de la connexion
 * etc.
 */


/**
 * \fn connected_server * next_server(connected_server * serv)
 * \brief obtenir le server suivant dans le liste
 */
connected_server * next_server(connected_server * serv)
{
    if(serv==NULL)
        return NULL;
    return serv->next;
}


/**
 * \fn connected_server * insert_server(connected_server * list_head,
 * struct in6_addr addr, in_port_t port)
 * \brief insertion d'un nouveau server dans la liste s'il n'y est pas déjà
 * \post le pointeur retourné est a liberer à la fin de son utilisation
 */
connected_server * insert_server(connected_server * list_head,
        struct in6_addr addr, in_port_t port)
{
    connected_server * new_server  = (connected_server*) malloc(sizeof(connected_server));
    new_server->next = list_head;
    list_head = new_server;
    list_head->addr = addr;
    list_head->last_keep_alive = time(NULL);
    list_head->port = port;
    return list_head;
}

/**
 * \fn int count_server(connected_server * list)
 * \brief compter le nombre de servers connectés
 */
int count_server(connected_server * list)
{
    if(list==NULL)
        return 0;
    return 1 + count_server(list->next);
}

/**
 * \fn connected_server *  suppression_server(connected_server * list_head,
 * struct in6_addr addr, in_port_t port)
 * \brief suppression du server donné, dans la liste des servers connectés
 */
connected_server *  suppression_server(connected_server * list_head,
        struct in6_addr addr, in_port_t port)
{
    if(list_head==NULL)
        return NULL;
    if(memcmp(&list_head->addr,&addr,sizeof(struct in6_addr))==0
            && list_head->port==port)
    {
        connected_server * tmp = list_head->next;
        free(list_head);
        return tmp;
    }
    else
    {
        list_head->next =
            suppression_server(list_head->next,addr,port);
        return list_head;
    }
}

/**
 * \fn void  keep_alive_server(connected_server * server,
 * struct in6_addr addr, in_port_t port)
 * \brief met a jour la date de derner maintient de la connexion avec le server
 * donné
 */
void  keep_alive_server(connected_server * server,
        struct in6_addr addr, in_port_t port)
{
    if(server==NULL)
    {
        return;
    }
    if(memcmp(&server->addr,&addr,sizeof(struct in6_addr))==0
                && server->port==port)
    {
        server->last_keep_alive = time(NULL);
        return;
    }
    else
    {
        keep_alive_server(server->next,addr,port);
        return;
    }
}


/**
 * \fn connected_server * suppression_dead_server(
 * connected_server * server_table)
 * \brief supression dans la lsite des servers connectés de tous les server
 * n'ayant plus données signe de vie depuis plus de 30 secondes
 */
connected_server * suppression_dead_server(connected_server * server_table)
{
    time_t sysdate = time(NULL);

    connected_server * server;
    connected_server * tmp;

    server = server_table;
    while(server!= NULL)
    {
        tmp = server->next;
        if( (memcmp(&server->addr,&my_addr,sizeof(struct in6_addr))==0
                && server->port==my_port)
                || sysdate-server->last_keep_alive>2)
        {
            server_table =
                suppression_server(server_table,server->addr,server->port);
            server = server_table;
        }
        server = tmp;
    }
    return server_table;
}

/**
 * \fn bool is_known(struct in6_addr addr, in_port_t port,
 * connected_server * server_table)
 * \brief retourne si oui ou non le server donné en argument est dja dans la
 * liste des servers connectés
 */
bool is_known(struct in6_addr addr, in_port_t port,
        connected_server * server_table)
{
    if(server_table==NULL)
    {
        return false;
    }
    else if(memcmp(&server_table->addr,&addr,sizeof(struct in6_addr))==0
            && server_table->port==port)
    {
        return true;
    }
    else
    {
        return is_known(addr,port,server_table->next);
    }
}

connected_server * get_server(struct in6_addr addr, in_port_t port, connected_server * server_table)
{
    if(server_table==NULL)
    {
        return NULL;
    }
    else if(memcmp(&server_table->addr,&addr,sizeof(struct in6_addr))==0
            && server_table->port==port)
    {
        return server_table;
    }
    else
    {
        return get_server(addr,port,server_table->next);
    }
}


/**
 * \fn void free_server_tab(connected_server * list)
 * \brief liberation de la table des servers connectés
 */
void free_server_tab(connected_server * list)
{
    if(list==NULL)
        return;
    if(list->next!=NULL)
    {
        free_server_tab(list->next);
    }
    free(list);
}
