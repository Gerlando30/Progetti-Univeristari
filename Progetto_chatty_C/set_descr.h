/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file set_descr.h  
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */
/**
 * @file set_descr.h
 * @brief gestisce la maschera dei descrittori in mutua esclusione
 */

#ifndef SET_DESCRIPTOR_H_
#define SET_DESCRIPTOR_H_

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


/**
 * @struct set_descriptor 
 * 
 * @param set maschera descrittori di connessione
 * @param fd_num valore del descrittore più grande attivo
 * @param m variabile per operare in mutua esclusione con la maschera
 */
typedef struct set_descriptor {
	fd_set set;
	int fd_num;
	pthread_mutex_t m;
} set_descriptor;


/**
 * @function set_lock 
 * 
 * @brief prende la lock
 * 
 * @param q struttura su cui vogliamo operare in mutua esclusione
 */
void  set_lock(set_descriptor *q);

/**
 * @function set_unlock
 * 
 * @brief rilascia la lock 
 * 
 * @param q struttura su cui abbiamo operato in mutua esclusione 
 */
void  set_unlock(set_descriptor *q);


/**
 * @function aggiorna_fd_num
 * 
 * @brief aggiorna il valore del descrittore più grande
 * 
 * @param q struttura contenente la maschera dei descrittori
 * 
 */
void  aggiorna_fd_num ( set_descriptor* s);

/**
 * @function set_fd 
 * 
 * @brief operazione di FD_SET in mutua esclusione
 * 
 * @param q struttura contenente la maschera dei descrittori
 * 
 */
void  set_fd(int fd, set_descriptor* s);


/**
 * @function clr_fd 
 * 
 * @brief operazione di FD_CRL in mutua esclusione
 * 
 * @param q struttura contenente la maschera dei descrittori
 * 
 */
void  clr_fd (int fd, set_descriptor* s );

/**
 * @function fd_isset 
 * 
 * @brief operazione di FD_ISSET in mutua esclusione
 * 
 * @param q struttura contenente la maschera dei descrittori
 * 
 */
int  fd_isset ( int fd , set_descriptor* s);



/**
 * @function copyset 
 * 
 * @brief copia la maschera di descrittori in mutua esclusione
 * 
 * @param q struttura contenente la maschera dei descrittori
 * @param fd_num copia del descrittore più grande attivo
 * @param set copia della maschera dei descrittori
 * 
 */
fd_set* copyset ( set_descriptor* s, int* fd_num, fd_set* set);


/**
 * @function init_set 
 * 
 * @brief inizializza la maschera di descrittori 
 * @param s struttura contenente la maschera da iniziallizare
 * @param fd_conn descrittore del socket server
 */
void  init_set ( set_descriptor* s, int fd_conn);


		
#endif
