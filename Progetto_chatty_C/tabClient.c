/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file tabClient.c  
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */
   /**
 * @file  tabClient.c
 * @brief si occupa di gestire gestire la tabella dei clienti registrati e le strutture ad essi associati 
 * e di svolgere varie operazioni tra le quali invio messaggio/file
 * 		 
 * 		   
 */
 
#include "tabClient.h"

/**
 * @function tablock 
 * @brief prende la lock della porzione di tabella interessata
 * @param t tabella dei clienti registrati
 * @param c nome del cliente
 */
void tablock(tab_clienti* t, char* c) {
	
	int indice_lock;
	indice_lock = (*t->clienti->hash_function)(c) % t->size_tab; //utilizzo la funzione hash fornita per calcolarmi l'indice della lock da prendere
	indice_lock = indice_lock % t->size_mutex;
	
	pthread_mutex_lock( &t->m[indice_lock] );
	
}



/**
 * @function tabunlock 
 * @brief rilascia la lock della porzione di tabella interessata
 * @param t tabella dei clienti registrati
 * @param c nome del cliente
 */
void tabunlock(tab_clienti *t, char* c) { 
	
	int indice_lock;
	indice_lock = ( *t->clienti->hash_function)(c) % t->size_tab; //utilizzo la funzione hash fornita per calcolarmi l'indice della lock da rilasciare
	indice_lock = indice_lock % t->size_mutex;
	
	pthread_mutex_unlock( &t->m[indice_lock] );
}



/**
 * @function init_tab
 * @brief crea una struttura tab_client e inizializza i parametri al suo interno
 * @param size dimensione di icl_hash_t* clienti
 * @param mutex dimensione del vettore di mutex
 * @return struttura del cliente inizializzata
 */
tab_clienti* init_tab (int size, int mutex) {
	
	tab_clienti* tab = malloc(sizeof(tab_clienti));
	if ( tab == NULL ) exit(EXIT_FAILURE);
	
	tab->clienti = icl_hash_create(size, NULL, NULL);
	tab->m = malloc(sizeof(pthread_mutex_t)*mutex);
	for (int i = 0; i<mutex; i++) {
		pthread_mutex_init(&(tab->m[i]), NULL);
	}
	tab->size_tab = size;
	tab->size_mutex = mutex;
	return tab;
}

/**
 * @function reg_client 
 * @brief inizializza una struttura cliente e inserisce nella tabella dei clienti registrati la coppia <chiave, valore> = <nome del cliente, struct cliente>
 * @param nome_cliente nome del cliente da registrare
 * @param tabella tabella dei clienti registrati
 * @param dim_hist dimensione della history da allocare per il cliente 
 * 
 */
void reg_client (char* nome_cliente, tab_clienti* tabella, int dim_hist) {
	
	tablock(tabella, nome_cliente);
	
	if ( icl_hash_find( tabella->clienti, nome_cliente ) == NULL ) { //se l'utente non è presente nella tabella 
		cliente* client = malloc(sizeof(cliente));
		client->history = malloc(sizeof(message_t)*dim_hist);
		for (int i = 0; i<dim_hist; i++) {
			client->history[i].data.buf = NULL;
		}
		client->size_coda=dim_hist;
		client->head=0;
		client->tail=0;
		client->msg_non_letti=0;
		icl_entry_t* c;
		char *utente = malloc(sizeof(char)*(MAX_NAME_LENGTH+1));  //alloco la chiave
		strncpy(utente, nome_cliente, MAX_NAME_LENGTH+1);
		
		c = icl_hash_insert(tabella->clienti, utente, client);
		if ( (c->data) == NULL) {  //controllo se è stato inserito
			printf("Impossibile registrare utente %s\n", nome_cliente);
		} 
	}
	 
	tabunlock(tabella, nome_cliente); 
}
	
/**
 * @function utente_registrato 
 * @brief controlla se un utente è già registrato 
 * @param tabella è la tabella dei clienti già registrati
 * @param utente nome del cliente che si vuole controllare
 * @return 1 se l'utente è già registrato, -1 altrimenti
 * 
 */
int utente_registrato (tab_clienti* tabella, char *utente) {
	
	int reg = 1;
	
	tablock(tabella, utente);
	if (icl_hash_find(tabella->clienti, utente) == NULL){
		 reg = -1;
	 }
	tabunlock(tabella, utente);
	
return reg;
}




/**
 * @function save_in_history 
 * @brief salva un messaggio nella history di uno specifico cliente
 * @param tabella è la tabella dei clienti registrati 
 * @param utente nome del cliente che riceve il messaggio 
 * @param msg il messaggio da salvare nella history
 * @return struttura del cliente se registrato, NULL altrimenti
 */
cliente* save_in_history(tab_clienti*tabella, char* utente, message_t* msg) {
	
	cliente* c = NULL;
	//controllo se utente è già registrato
	if ( (c = icl_hash_find(tabella->clienti, utente)) == NULL ){  //cliente non è registrato 
		return c;
	}
	else {  //utente registrato
		if (c->history[c->tail].data.buf){   //se c'è un messaggio nella coda della history lo dealloco
			free(c->history[c->tail].data.buf);
			c->history[c->tail].data.buf = NULL;
		}
		//inserisco il nuovo messaggio
		memset( &(c->history[c->tail]), 0, sizeof(message_t) );
		char* contenuto = calloc((strlen(msg->data.buf)+1), sizeof(char));
		strcpy(contenuto, msg->data.buf);
		
		setHeader(&(c->history[c->tail].hdr), msg->hdr.op, msg->hdr.sender);
		setData(&(c->history[c->tail].data), msg->data.hdr.receiver, contenuto, msg->data.hdr.len);
		
		//aggiorno indice alla coda
		c->tail++; 
		c->tail = c->tail % c->size_coda;
		
		}
return c;
}



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
int send_msg ( tab_clienti*tabella, char* utente, message_t* msg, users_online* us, int maxsize) {
	
	cliente* c= NULL;
	int ok =0;
	
	if ( msg->data.hdr.len > maxsize){
		perror("Messaggio troppo lungo");
		return -2;
	}
	
	tablock(tabella, utente);
	
	c = save_in_history( tabella, utente, msg);
	
	if ( c == NULL ) ok = -1;
	else{ 
		
		users_online_lock(us);
		int fd =take_fd_user (us,utente);
		users_online_unlock(us);
		
		if ( fd != -1 ){
			printf(" Nome del ricevenete %s il suo fd è %d\n", utente, fd);
			
			if ( sendRequest(fd, msg)<=0 ){  //invio messaggio fallito
				ok = 0;
				if ( c->msg_non_letti != c->size_coda)  //se i messaggi non letti sono minori della dimensione della history
					c->msg_non_letti++; 
			} else ok=1; // messaggio inviato
		}
		else {
				if ( c->msg_non_letti != c->size_coda)  //se i messaggi non letti sono minori della dimensione della history
					c->msg_non_letti++;
			}
		printf(" Nome del ricevenete %s, messaggi da leggere: %ld\n", utente, c->msg_non_letti);
	}
	tabunlock(tabella, utente);	
	return ok; 
}



/**
 * @function leggi_msg
 * @brief legge il messaggio in testa alla history
 * @param c cliente di cui vogliamo leggere il messaggio della history
 * @param msg in cui copiamo il messaggio in testa alla history
 */
void leggi_msg(cliente* c, message_t* msg) {
	
	memset( msg, 0, sizeof(message_t));
	char *buf = NULL;
	buf = (char *)c->history[c->head].data.buf;
	setHeader( &(msg->hdr), (c->history[c->head].hdr).op, (c->history[c->head].hdr.sender)); 
	setData( &(msg->data), (c->history[c->head].data.hdr.receiver), buf,(c->history[c->head].data.hdr.len) );
}



/**
 * @function send_history
 * @brief invia la history al cliente (solo messaggi non letti)
 * @param tabella è la tabella dei clienti registrati
 * @param utente è il nome del cliente che ha richiesto la history
 * @param fd descrittore della connessione
 */
int send_history (tab_clienti*tabella, char* utente,int fd) {
	
	int num_send = 0; //numero messaggi inviati
	
	cliente* c = NULL;
	char *u = malloc(sizeof(char)*(MAX_NAME_LENGTH+1)); //nome del cliente 
	strcpy(u, utente);
	
	tablock(tabella, u);
	
	if ( (c = icl_hash_find(tabella->clienti, u)) == NULL ){ //prendo la struttura associata al cliente c
		num_send = -1;
	} else {
		
		message_t msg;
		setHeader(&msg.hdr, OP_OK, "");
		size_t mes_non_letti = 0;
		mes_non_letti = c->msg_non_letti;
		setData(&msg.data, "", (char*)&mes_non_letti, sizeof(size_t));
		sendRequest(fd, &msg);  //invio il numero di messaggi che gli sto per inviare
		while (c->msg_non_letti>0) {
			printf("I messaggi da leggere sono %ld\n", c->msg_non_letti);
			leggi_msg(c, &msg);
			if (sendRequest(fd, &msg) <=0) {  //invio messaggi fallito
				perror("Impossibile inviare messaggio");
				break;
			}
			else {  //messaggio inviato
				num_send++;
				//aggiorno testa history
				c->head++;
				c->head= c->head % c->size_coda;
				
				c->msg_non_letti --;
				
				
			}
		}
	}
	tabunlock(tabella, u);
	free(u);
return num_send; 
}




/**
 * @function free_client 
 * @brief dealloca la struttura di un certo cliente
 * @param c cliente che si vuole eliminare
 */
void free_client(cliente *c) {
	
	
	for (int i = 0; i< c->size_coda; i++){
		
		if (c->history[i].data.buf != NULL){
			 free(c->history[i].data.buf);
			 c->history[i].data.buf =NULL;
		 }
	}
	
	free(c->history);
	free(c);
	
}



/**
 * @function unregister_user
 * @param tabella tabella clienti registrati
 * @param name nome cliente da deregistrare
 * @return 1 successo, -1 altrimenti
 */
int unregister_user ( tab_clienti* tabella, char* name ) {
	int ok;
	tablock(tabella, name);
	char * n = malloc(sizeof(char)*(MAX_NAME_LENGTH+1));
	strcpy(n, name);
	ok = icl_hash_delete(tabella->clienti, n, free,  free_client);
	tabunlock(tabella,  n);
	free(n);
	
return ok;
}




/**
 * @function send_msg_to_all 
 * @brief invia un messaggio a tutti i clienti registrati
 * @param tabella tabella clienti registrati
 * @param msg messaggio da inviare
 * @param us utenti online
 * @param maxsize massima dimensione messaggio
 * @return numero messaggi inviati, -1 errore
 */
int send_msg_to_all (tab_clienti* tabella, message_t* msg, users_online* us, int maxsize) {
	
	 int num_send = 0;
	 icl_entry_t *bucket, *curr;
	 int i;
	 if(!tabella->clienti) return -1;
	
	
    for (i=0; i<tabella->clienti->nbuckets; i++) {  //scorro tutta la tabella dei clienti registrati
        bucket = tabella->clienti->buckets[i];
        for (curr=bucket; curr!=NULL; ) {
            if ( strcmp(msg->hdr.sender, curr->key) != 0){
				
				if ( send_msg(tabella, curr->key, msg, us, maxsize) ==1)
				num_send++;
            }
            curr=curr->next;
        }
    }
    free(msg->data.buf);
    msg->data.buf = NULL;
    return num_send;
}



/**
 * @function send_file 
 * @brief invio di un file 
 * @param path percorso file 
 * @param nome cliente a cui inviare il file
 * @param tabella tabella clienti registrati
 * @param us utenti online
 * @return 1 file inviato, -1 errore
 */
int send_file ( char* path, char* nome, tab_clienti* tabella, users_online* us ) {
	
	
	int ok = 1;					
	FILE *file;
	long lSize;
	char *buffer = NULL;
	tablock(tabella, nome);
	if( (file = fopen (path , "rb" )) == NULL ) {
		perror("ERRORE APERTURA FILE");
		ok = -1;
	} else {
		
		fseek( file , 0L , SEEK_END);//sposto l'offset della posizione corrente nel file, alla fine
		
		lSize = ftell( file ); //prendo l'offset attuale che mi indicherà la dimensione del file
		
		rewind( file );  //sposto l'offset all'inizio del file
		
		buffer = calloc( 1, lSize+1 );
		//copio il contenuto del file nel buffer
		if( fread( buffer , lSize, 1 , file) != 1){ //operazione fallita
			fclose(file);
			perror("NON È POSSIBILE LEGERE IL FILE");
			ok = -1;
		} else { //operazione riuscita
		
		fclose(file);
	
		message_t msg;
		msg.data.buf = NULL;
		setHeader(&msg.hdr,OP_OK,"");
		setData(&msg.data,"",buffer,lSize);
		
		users_online_lock(us);
		int fd = take_fd_user(us, nome);
		users_online_unlock(us);
		
		if (fd != -1){ 
			if ( sendRequest(fd, &msg) < 0 ) perror("NON SIAMO RIUSCITI AD INVIARE IL FILE");
			else perror("FILE INVIATO");
			} else perror("UTENTE NON CONNESSO");				
		}
		free(buffer);
	}	
	tabunlock(tabella, nome);
	
	return ok;
}

/**
 * @function destroy_tab
 * @brief dealloca la tabella dei clienti registrati
 * @param tabella tabella clienti registrati
 */ 
void destroy_tab(tab_clienti* tabella) {
	icl_hash_destroy(tabella->clienti, free, free_client);
}

