/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file set_descr.c  
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */


/**
 * @file set_descr.c
 * @brief gestisce la maschera dei descrittori in mutua esclusione
 */
#include "set_descr.h"

void  set_lock(set_descriptor *q) { pthread_mutex_lock(&q->m); }
void  set_unlock(set_descriptor *q) { pthread_mutex_unlock(&q->m); }



/**
 * @function aggiorna_fd_num
 * 
 * @brief aggiorna il valore del descrittore più grande
 * 
 * @param q struttura contenente la maschera dei descrittori
 * 
 */
 void aggiorna_fd_num ( set_descriptor* s) {
	while (s->fd_num >0) {
		if (FD_ISSET(s->fd_num, &(s->set))) break;
		else {
			s->fd_num--;
		}
	}
} 
/**
 * @function set_fd 
 * 
 * @brief operazione di FD_SET in mutua esclusione
 * 
 * @param q struttura contenente la maschera dei descrittori
 * 
 */
void set_fd(int fd, set_descriptor* s) {
	set_lock(s);
	FD_SET(fd, &(s->set));
	if (fd > s->fd_num) s->fd_num = fd; 
	set_unlock(s);
}

/**
 * @function clr_fd 
 * 
 * @brief operazione di FD_CRL in mutua esclusione
 * 
 * @param q struttura contenente la maschera dei descrittori
 * 
 */
 void clr_fd ( int fd, set_descriptor* s ) {
	set_lock(s);
	FD_CLR( fd , &(s->set));
	if ( fd >= s->fd_num)
		aggiorna_fd_num (s);
	set_unlock(s);
} 


/**
 * @function fd_isset 
 * 
 * @brief operazione di FD_ISSET in mutua esclusione
 * 
 * @param q struttura contenente la maschera dei descrittori
 * 
 */
int fd_isset ( int fd , set_descriptor* s) {
	set_lock(s);
	int isset = FD_ISSET( fd , &(s->set));
	set_unlock(s);
	return isset;
}

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
fd_set* copyset ( set_descriptor* s, int* fd_num, fd_set* set) {
	set_lock(s);
	memcpy(set, &s->set, sizeof(s->set));
	int f = s->fd_num;
	*fd_num= f;
	set_unlock(s);
	return set;
}

/**
 * @function init_set 
 * 
 * @brief inizializza la maschera di descrittori 
 * @param s struttura contenente la maschera da iniziallizare
 * @param fd_conn descrittore del socket server
 */
void init_set ( set_descriptor* s, int fd_conn) {
	pthread_mutex_init( &s->m , NULL);
	FD_ZERO(&(s->set));
	if ( fd_conn > (s->fd_num)) s->fd_num = fd_conn;
}
