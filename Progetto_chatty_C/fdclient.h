/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file fdclient.h
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */
 /**
 * @file  fdclient.c
 * @brief gestisce la lista dei clienti connessi
 * 		   
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "message.h"


#define CONTROL_NULL(PUNTATORE, STRINGA)	\
 if ((PUNTATORE) == NULL ) { \
	 perror(#STRINGA); \
 }	 
 
/**
 * @struct user_on 
 * 
 * @brief struttura che rappresenta un cliente online
 * 
 * @param fd descrittore della connessione
 * @param name nome del cliente online
 * @param next puntatore al successivo cliente online
 */
typedef struct user_on {
	int fd;
	char *name;
	struct user_on* next;
} user_on;


/**
 * @struct users_online 
 * 
 * @brief struttura che rappresenta lista clienti online
 * 
 * @param user_head puntatore al primo elemento della lista
 * @param num_online numero di clienti online
 * @param max_online numero di clienti massimo che possono essere online
 * @param fdm variabile per la lock della lista dei clienti online
 */
typedef struct users_online {
	user_on* user_head;
	int num_online;
	int max_online;
	pthread_mutex_t fdm;
} users_online;	


/**
 * @function users_online_lock 
 * 
 * @brief prende la lock 
 * 
 * @param q struttura su cui vogliamo operare in mutua esclusione 
 */
void users_online_lock(users_online* q) ;

/**
 * @function users_online_unlock 
 * 
 * @brief rilascia la lock 
 * 
 * @param q struttura su cui abbiamo operato in mutua esclusione 
 */
void users_online_unlock(users_online* q) ;


/**
 * 
 * @function init_users_online 
 * 
 * @brief inizializza la struttura dei clienti online
 * @param max numero massimo di clienti online
 * 
 * @return struttura inizializzata
 */
users_online* init_users_online (int max);


/**
 * @function remove_user_online
 * 
 * @brief mette offline un cliente
 * 
 * @param us struttura dei clienti online
 * @param fd descrittore di connessione del cliente che si vuole mettere offline
 */
int remove_user_online (users_online* us, int fd);



/**
 * @function remove_all_user_online
 * 
 * 
 * @brief rimuove tutti gli utenti connessi e dealloca la struttura che li contiene
 * 
 * @param us struttura da liberare
 * 
 */
void remove_all_user_online ( users_online* us );


/**
 * @function take_fd_user 
 * 
 * @brief prende il descrittore di connessione di uno specifico cliente
 * 
 * @param us struttura dei clienti online
 * @param utente nome del cliente di cui vogliamo prendere il descrittore di connessione
 * 
 * @return descrittore di connessione del cliente "utente", -1 altrimenti
 */
int take_fd_user (users_online* us, char *utente);



/**
 * @function getlist_user_online
 * 
 * @brief restituisce la lista dei clienti online
 * 
 * @param struttura che contiene clienti connessi
 * @param num_online copia del numero dei clienti attualmente connessi
 * 
 * @return stringa di clienti online
 */
char* getlist_user_online (users_online* us, int* num_online );



/**
 *@function add_user_online 
 * @brief mette un cliente online
 * 
 * @param us struttura dei clienti online
 * @param fd descrittore di connessione del cliente
 * @param user nome del cliente  
 * 
 */
int add_user_online(users_online* us, int fd, char* user);
