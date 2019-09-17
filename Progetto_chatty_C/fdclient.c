/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file fdclient.c  
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
 
#include "fdclient.h"



/**
 * 
 * @function init_users_online 
 * 
 * @brief inizializza la struttura dei clienti online
 * @param max numero massimo di clienti online
 * 
 * @return struttura inizializzata
 */
users_online* init_users_online (int max) {
	
	users_online* us = malloc(sizeof(users_online));
	CONTROL_NULL(us, "Errore nella creazione della coda");
	pthread_mutex_init(&(us->fdm), NULL); 
	us->num_online =0;
	us->max_online = max;
	us->user_head=NULL;
	
	return us; 
}

/**
 * @function users_online_lock 
 * 
 * @brief prende la lock 
 * 
 * @param q struttura su cui vogliamo operare in mutua esclusione 
 */ 
void users_online_lock(users_online* q) { pthread_mutex_lock(&q->fdm); }




/**
 * @function users_online_unlock 
 * 
 * @brief rilascia la lock 
 * 
 * @param q struttura su cui abbiamo operato in mutua esclusione 
 */
void users_online_unlock(users_online* q) { pthread_mutex_unlock(&q->fdm); }




/**
 * @function remove_user_online
 * 
 * @brief mette offline un cliente
 * 
 * @param us struttura dei clienti online
 * @param fd descrittore di connessione del cliente che si vuole mettere offline
 * @return .1 se il cliente non è presente
 */
int remove_user_online (users_online* us, int fd) {
	int ok=-1;
	
	users_online_lock(us);
	user_on* curr;
	curr = us->user_head;
	user_on* prev = NULL;
	if ( curr != NULL ) {  //se la lista non è vuota
		if ( curr -> fd == fd ) {  //controllo la testa
			us->user_head = us->user_head->next;
			free(curr->name);
			free(curr);
			us->num_online--;
			ok = 1;
		} else {
			prev = curr;
			curr = curr->next;
			while (curr != NULL) { //scorro la lista
				if ( curr ->fd == fd) {
					prev->next = curr->next;
					free(curr->name);
					free(curr);
					us->num_online --;
					ok = 1;
					break;
				} else {
					prev = curr;
					curr = curr->next;
				}
			}
		}
	}
    
	users_online_unlock(us);
	return ok;
}


/**
 * @function remove_all_user_online
 * 
 * 
 * @brief rimuove tutti gli utenti connessi e dealloca la struttura che li contiene
 * 
 * @param us struttura da liberare
 * 
 */
void remove_all_user_online ( users_online* us ) {
	
	
	while ( us->user_head != NULL ) {
		user_on* curr; 
		curr = us->user_head;
		us->user_head = us->user_head->next;
		free(curr);
	}
	free(us);
}

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
int take_fd_user (users_online* us, char *utente) {
	int fd = -1;
	
	user_on* curr = us->user_head;
	while (curr != NULL) { //scorro la lista fino a quando non trovo il nome del cliente
		if ( strcmp(curr->name, utente) == 0) {
			fd = curr->fd;
			break;
		} else {
			curr = curr->next;
			}
		}
	return fd; 
}

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
char* getlist_user_online (users_online* us, int* num_online ) {
	
	users_online_lock(us);
	
	user_on* curr = us->user_head;
	*num_online = us->num_online;
	char* lista= malloc(sizeof(char)*((MAX_NAME_LENGTH+1)*(us->num_online)));  //stringa in cui inserisco i nomi dei clienti online
	memset(lista, 0, sizeof(char));
	char* testa = NULL; //mi salvo il puntatore in testa della stringa
	testa=lista;
	
	
	while (curr != NULL) {
		printf("Nome utente Online: %s\n", curr->name);
		strncpy(lista, curr->name, MAX_NAME_LENGTH+1);
		
		curr = curr->next;
		lista += MAX_NAME_LENGTH+1;
		
	}
	users_online_unlock(us);
	
	return testa;
}


/**
 *@function add_user_online 
 * @brief mette un cliente online
 * 
 * @param us struttura dei clienti online
 * @param fd descrittore di connessione del cliente
 * @param user nome del cliente  
 * @return 1 cliente aggiunto, 0 cliente già presente, -1 altrimenti
 * 
 */
int add_user_online(users_online* us, int fd, char* user) {
	
	int ok = 0; 
	
	int i;  
	
	users_online_lock(us);
	
	if ( us->num_online == us->max_online ) ok = -1;  //se ho ragiunto il massimo numero di connessioni non lo inserisco 
	else if ((i= take_fd_user(us, user)) == -1){  //controllo che non sia già connesso
		
		user_on* new_user=NULL;
		new_user = calloc(1,sizeof(user_on));
		CONTROL_NULL(new_user, "Errore nella creazione nuovo task");
		
		new_user->name = calloc((MAX_NAME_LENGTH+1), sizeof(char));
		CONTROL_NULL(new_user->name,  "errore nome");
		strcpy(new_user->name,  user);
		
		new_user->fd=fd;
		new_user->next = us->user_head; //aggiungo in testa
		us->user_head = new_user;
		ok = 1;
		us->num_online++;
	}
	
	
	users_online_unlock(us);
return ok;
}
