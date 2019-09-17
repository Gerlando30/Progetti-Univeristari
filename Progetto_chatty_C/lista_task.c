/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file lista_task.c  
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */
  /**
 * @file  lista_task.c
 * @brief gestisce la lista degli fd secondo il modello FIFO 
 * 		 
 * 		   
 */ 
#include <lista_task.h>

/**
 * @function tasklock 
 * @brief prende la lock 
 * @param q coda su cui effettuare la lock
 * 
 */
void tasklock(coda_task *q) { pthread_mutex_lock(&q->m); }

/**
 * @function taskunlock 
 * @brief rilascia la lock 
 * @param q coda su cui effettuare la unlock
 * 
 */
void taskunlock(coda_task *q) { pthread_mutex_unlock(&q->m); }


/**
 * @function istaskempty 
 * @brief fa la wait 
 * @param q coda su cui fare la wait
 */
void istaskempty (coda_task *q) { pthread_cond_wait(&q->e , &q->m); }

/**
 * @function nottaskempty 
 * @brief fa la signal
 * @param q coda su cui fare la signal
 */
void nottaskempty (coda_task *q) { pthread_cond_signal(&q->e); }

/**
 * @function istaskempty 
 * @brief controlla se la coda è vuota
 * @param q coda da controllare
 * @return 1 se vuota, 0 altrimenti
 */
int IsEmpty(coda_task* cd) { if (cd->elm == 0 ) return 1; else return 0; }

/**
 * @function EnQueue 
 * @brief aggiunge un elemento elm_list in fondo alla coda
 * @param cd coda a cui aggiungere l'elemento
 * @param fd_client descrittore della connessione da aggiungere alla coda
 * 
 */
void EnQueue (coda_task* cd, int fd_client) {
	
	
	tasklock(cd);  //acquisiamo la lock
	
	//creiamo e aggiungiamo il nuovo task nella coda dei task
	
	
	if (cd->head == NULL) { //se non ci sono elementi lo aggiungo in testa
		cd -> head = malloc(sizeof(elm_list));
		cd->head->fd_client=fd_client;
		cd->head->next = NULL;
		cd -> tail = cd ->head;
	}
	else { //altrimenti lo aggiungo in fondo
		cd ->tail ->next = malloc(sizeof(elm_list));
		cd ->tail->next->fd_client = fd_client;
		cd->tail->next->next = NULL;
		cd->tail = cd->tail->next;
	}
	cd->elm ++;
	nottaskempty(cd); //segnalo che la coda non è più vuota
	taskunlock(cd);    //libero la lock
}

/**
 * @function DeQueue 
 * @brief rimuove  l'elemento elm_list in testa alla coda
 * @param cd coda a cui aggiungere l'elemento
 * @return fd_client descrittore della connessione rimosso dalla coda
 * 
 */
int DeQueue (coda_task *cd) {
	
	tasklock(cd); //prendo la lock
	
	while (IsEmpty(cd) == 1) //se la coda è vuota faccio la wait sulla variabile "e"
		istaskempty(cd);
		
	int fd_client = cd -> head->fd_client;  //mi salvo il valore di fd_client in testa alla coda
	if ( cd->head == cd->tail) { //se c'è un solo elemento
		
		elm_list* job = cd -> head;
		free(job);
		cd->head = NULL;
		cd->tail = NULL;
	}else { // se ci sono due o più elementi
		elm_list* job=cd->head;
		cd->head = cd->head->next;
		free(job);
	}
	cd->elm  --;
	taskunlock(cd);  //libero la lock
	
	return fd_client;
}

/**
 * @function destroy
 * @brief dealloca una coda
 * @param cd coda da deallocare
 */
void destroy (coda_task *cd) {
	
     
      while(cd->head != NULL) {
		  elm_list* rimuovi;
          rimuovi = cd->head;
          cd->head= cd->head->next;
          free(rimuovi);
        }
    }
/**
 * @function init 
 * @brief inizializza una stuttura di tipo coda_task
 * @return elemento di tipo coda_task inizializzato
 * 
 */
coda_task* init() {
	
	//alloco la memoria per la coda
	coda_task* cd = malloc(sizeof(coda_task));
	CONTROL_NULL(cd, "Errore nella creazione della coda");
	//inizializzo i parametri della coda
	cd->head=NULL;
	cd->tail=NULL;
	cd->elm =0;
	pthread_mutex_init(&(cd->m), NULL);
	pthread_cond_init(&(cd->e), NULL);
	
	return cd;
}







