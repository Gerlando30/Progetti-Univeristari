/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file lista_task.h
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */
 
  /**
 * @file  lista_task.h
 * @brief gestisce la lista degli fd secondo il modello FIFO 
 * 		 
 * 		   
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#ifndef lista_task_h
#define lista_task_h

#define CONTROL_NULL(PUNTATORE, STRINGA)	\
 if ((PUNTATORE) == NULL ) { \
	 perror(#STRINGA); \
	 exit(EXIT_FAILURE); \
 }	  
/*#define CONTROL_ZERO(VALORE, STRINGA)	\
 if ((VALORE) != 0 ) { \
	 perror(#STRINGA); \
	 exit(EXIT_FAILURE); \
 }*/	  
#define N 20

/**
 *@struct elm_list
 * @param fd_client descrittore della connessione
 * @param next puntatore al successivo fd da ascoltare
 */
typedef struct elm_list {
	int fd_client;
	struct elm_list* next;
} elm_list;


/**
 *@struct coda_task
 * @param head puntatore alla testa della coda
 * @param tail puntatore all'ultimo elemento della coda
 * @param elm numero di elementi nella coda
 * @param m variabile per la lock della coda
 * @param e variabile di condizione 
 */
typedef struct coda_task {
	elm_list* head;  //puntatore alla testa dei task
	elm_list* tail;  //puntatore alla coda dei task
	int elm;
	pthread_mutex_t m;
	pthread_cond_t e;
} coda_task;

 
 
/**
 * @function tasklock 
 * @brief prende la lock 
 * @param q coda su cui effettuare la lock
 * 
 */
void tasklock(coda_task *q);
/**
 * @function taskunlock 
 * @brief rilascia la lock 
 * @param q coda su cui effettuare la unlock
 * 
 */
void taskunlock(coda_task *q) ;


/**
 * @function istaskempty 
 * @brief fa la wait 
 * @param q coda su cui fare la wait
 */
void istaskempty (coda_task *q);



/**
 * @function nottaskempty 
 * @brief fa la signal
 * @param q coda su cui fare la signal
 */
void nottaskempty (coda_task *q);

/**
 * @function istaskempty 
 * @brief controlla se la coda è vuota
 * @param q coda da controllare
 * @return 1 se vuota, 0 altrimenti
 */
int IsEmpty(coda_task* cd);


/**
 * @function init 
 * @brief inizializza una stuttura di tipo coda_task
 * @return elemento di tipo coda_task inizializzato
 * 
 */
coda_task* init();


/**
 * @function EnQueue 
 * @brief aggiunge un elemento elm_list in fondo alla coda
 * @param cd coda a cui aggiungere l'elemento
 * @param fd_client descrittore della connessione da aggiungere alla coda
 * 
 */
void EnQueue (coda_task* cd, int fd_client);


/**
 * @function DeQueue 
 * @brief rimuove  l'elemento elm_list in testa alla coda
 * @param cd coda a cui aggiungere l'elemento
 * @return fd_client descrittore della connessione rimosso dalla coda
 * 
 */
int DeQueue (coda_task *cd);


/**
 * @function destroy
 * @brief dealloca una coda
 * @param cd coda da deallocare
 */
void destroy (coda_task *cd);


#endif


