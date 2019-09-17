/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file tabClient.h
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */
  /**
 * @file  tabClient.h
 * @brief si occupa di gestire gestire la tabella dei clienti registrati e le strutture ad essi associati 
 * e di svolgere varie operazioni tra le quali invio messaggio/file
 * 		 
 * 		   
 */

#include <icl_hash.h>
#include <message.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <connections.h>
#include <fdclient.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <config.h>

/**
 *@struct cliente
 * @param history: history del cliente
 * @param size_coda dimensione della history
 * @param head indice della testa della history 
 * @param tail indice della coda della history
 * @param msg_non_letti numero messaggi non ancora letti
 */

typedef struct cliente {
	message_t* history;   
	int size_coda;
	int head;
	int tail;
	size_t msg_non_letti;
} cliente;

/**
 *@struct tab_clienti
 * @param clienti tabella hash contenente i clienti registrati
 * @param m array di variabili per lock 
 * @param size_tab dimensione della tabella
 * @param size_mutex dimensione di m
 * @param msg_non_letti numero messaggi non ancora letti
 */
typedef struct tab_clienti {
	icl_hash_t* clienti;
	pthread_mutex_t* m;
	int size_tab;
	int size_mutex;
} tab_clienti;

/**
 * @function tablock 
 * @brief prende la lock della porzione di tabella in cui è registrato il cliente c
 * @param t tabella dei clienti registrati
 * @param c nome del cliente
 */
void tablock(tab_clienti* t, char* c);

/**
 * @function tabunlock 
 * @brief rilascia la lock della porzione di tabella in cui è registrato il cliente c
 * @param t tabella dei clienti registrati
 * @param c nome del cliente
 */
void tabunlock(tab_clienti *t, char* c);

/**
 * @function init_tab
 * @brief crea una struttura tab_client e inizializza i parametri al suo interno
 * @param size dimensione di icl_hash_t* clienti
 * @param mutex dimensione del vettore di mutex
 * @return struttura del cliente inizializzata
 */
tab_clienti* init_tab (int size, int mutex);

/**
 * @function reg_client 
 * @brief inizializza una struttura cliente e inserisce nella tabella dei clienti registrati la coppia <chiave, valore> = <nome del cliente, struct cliente>
 * @param nome_cliente nome del cliente da registrare
 * @param tabella tabella dei clienti registrati
 * @param dim_hist dimensione della history da allocare per il cliente 
 * 
 */
void reg_client (char* nome_cliente, tab_clienti* tabella, int dim_hist);


/**
 * @function utente_registrato 
 * @brief controlla se un utente è già registrato 
 * @param tabella è la tabella dei clienti già registrati
 * @param utente nome del cliente che si vuole controllare
 * @return 1 se l'utente è già registrato, -1 altrimenti
 * 
 */
int utente_registrato (tab_clienti* tabella, char *utente);

/**
 * @function save_in_history 
 * @brief salva un messaggio nella history di uno specifico cliente
 * @param tabella è la tabella dei clienti registrati 
 * @param utente nome del cliente che riceve il messaggio 
 * @param msg il messaggio da salvare nella history
 * @return struttura del cliente se registrato, NULL altrimenti
 */
cliente* save_in_history(tab_clienti*tabella, char* utente, message_t* msg);
/**
 * @function send_msg 
 * @brief salva il messaggio nella history del ricevente invocando save_in_history e se il ricevente è online invia il messaggio
 * @param tabella è la tabella dei clienti registrati
 * @param utente è il nome del cliente che deve ricevere il messaggio
 * @param msg è il messaggio da inviare
 * @param us è la struttura contente la lista degli utenti online
 * @param maxsize è la massima dimensione del messaggio
 * @return 1 operazione avvenuta con successo, -1 errore, -2 messaggio troppo grande
 */ 
int send_msg ( tab_clienti*tabella, char* utente, message_t* msg, users_online* us, int maxsize);

/**
 * @function leggi_msg
 * @brief legge il messaggio in testa alla history
 * @param c cliente di cui vogliamo leggere il messaggio della history
 * @param msg in cui copiamo il messaggio in testa alla history
 */
void leggi_msg(cliente* c, message_t* msg);

/**
 * @function send_history
 * @brief invia la history al cliente (solo messaggi non letti)
 * @param tabella è la tabella dei clienti registrati
 * @param utente è il nome del cliente che ha richiesto la history
 * @param fd descrittore della connessione
 */
int send_history (tab_clienti*tabella, char* utente,int fd);

/**
 * @function free_client 
 * @brief dealloca la struttura di un certo cliente
 * @param c cliente che si vuole eliminare
 */

void free_client(cliente *c);

/**
 * @function unregister_user
 * @param tabella tabella clienti registrati
 * @param name nome cliente da deregistrare
 * @return 1 successo, -1 altrimenti
 */
int unregister_user ( tab_clienti* tabella, char* name );

/**
 * @function send_msg_to_all 
 * @brief invia un messaggio a tutti i clienti registrati
 * @param tabella tabella clienti registrati
 * @param msg messaggio da inviare
 * @param us utenti online
 * @param maxsize massima dimensione messaggio
 * @return numero messaggi inviati, -1 errore
 */

int send_msg_to_all (tab_clienti* tabella, message_t* msg, users_online* us, int maxsize);

/**
 * @function destroy_tab
 * @brief dealloca la tabella dei clienti registrati
 * @param tabella tabella clienti registrati
 */ 
void destroy_tab( tab_clienti* tabella);

/**
 * @function send_file 
 * @brief invio di un file 
 * @param path percorso file 
 * @param nome cliente a cui inviare il file
 * @param tabella tabella clienti registrati
 * @param us utenti online
 * @return 1 file inviato, -1 errore
 */
int send_file ( char* path, char* nome, tab_clienti* tabella, users_online* us );





























