/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file connection.c  
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */
/**
 * @file  connection.c
 * @brief Contiene le funzioni che implementano il protocollo 
 *        tra i clients ed il server
 */
#include "connections.h"

static pthread_mutex_t mutex_write = PTHREAD_MUTEX_INITIALIZER;  //lock per la scrittura
static pthread_mutex_t mutex_read = PTHREAD_MUTEX_INITIALIZER;   //lock per la lettura



/**
 * @function openConnection
 * @brief Apre una connessione AF_UNIX verso il server 
 *
 * @param path Path del socket AF_UNIX 
 * @param ntimes numero massimo di tentativi di retry
 * @param secs tempo di attesa tra due retry consecutive
 *
 * @return il descrittore associato alla connessione in caso di successo
 *         -1 in caso di errore
 */
int openConnection(char* path, unsigned int ntimes, unsigned int secs) {
	
	int fd=-1;
	struct sockaddr_un sa;
	fd = socket (AF_UNIX, SOCK_STREAM, 0);
	
	if (strlen(path) > UNIX_PATH_MAX) {
		perror("Errore: path troppo lungo");
		return -1;
	}
	printf("IL PATH DEL CLIENT È%s\n", path);
	strncpy(sa.sun_path, path, UNIX_PATH_MAX);
	sa.sun_family = AF_UNIX;
	int tentativo = 0;
	while ( connect(fd, (struct sockaddr*)&sa, sizeof(sa)) == -1 && tentativo < ntimes ) {
		printf("provo\n");
		if (errno == ENOENT ) sleep(secs);
		else return -1;
		
		tentativo++;
		
	}
	if (tentativo==ntimes) return -1;
	
	return fd;
}


/**
 * @function sendHeader 
 * @brief invia header del messaggio
 * @param fd descrittore della connessione
 * @param hdr puntatore all'header del messaggio da inviare 
 * @return 1 successo, -1 altrimenti
 */
int sendHeader (long fd, message_hdr_t* hdr) {
	
	int control =1;
	control = write((int) fd, hdr, sizeof(message_hdr_t));
	
return control;
}
	
	
	
/**
 * @function sendRequest
 * @brief Invia un messaggio di richiesta al server 
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da inviare
 *
 * @return <=0 se c'e' stato un errore
 */
int sendRequest(long fd, message_t *msg) {
		pthread_mutex_lock(&mutex_write);
		if (sendHeader( fd, &(msg->hdr)) == -1 ){
			pthread_mutex_unlock(&mutex_write); 
			return -1;
		}
		
		if (sendData(fd, &(msg->data)) == -1 ) {
			pthread_mutex_unlock(&mutex_write);
			return -1;
		}
		pthread_mutex_unlock(&mutex_write);

return 1;
}


/**
 * @function sendData
 * @brief Invia il body del messaggio al server
 *
 * @param fd     descrittore della connessione
 * @param msg    puntatore al messaggio da inviare
 *
 * @return <=0 se c'e' stato un errore
 */
int sendData(long fd, message_data_t *msg) {
	
	int written=0;
    if ( (written = write ((int)fd, &(msg->hdr), sizeof(message_data_hdr_t))) == -1) {
		return -1;
	}
    
    size_t len = msg->hdr.len;
    char *buf = NULL;
    buf = (char*)msg->buf;
    
    while( len > 0 ) {
		if ((written=write((int)fd ,buf,len)) == -1) {
			return -1;
		}
		len -= written;
		buf += written;
    }
    
return 1;
}

/**
 * @function readHeader
 * @brief Legge l'header del messaggio
 *
 * @param fd     descrittore della connessione
 * @param hdr    puntatore all'header del messaggio da ricevere
 *
 * @return <=0 se c'e' stato un errore 
 *         (se <0 errno deve essere settato, se == 0 connessione chiusa) 
 */
int readHeader(long connfd, message_hdr_t *hdr) {
	
	memset(hdr, 0, sizeof(message_hdr_t));

	int r =read( (int)connfd, hdr, sizeof(message_hdr_t));


return r;

}
/**
 * @function readData
 * @brief Legge il body del messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore al body del messaggio
 *
 * @return <=0 se c'e' stato un errore
 *         (se <0 errno deve essere settato, se == 0 connessione chiusa) 
 */
int readData(long fd, message_data_t *data) {
	
	memset(data, 0, sizeof(message_data_t));
    int bytes_read=0;
    if ( (bytes_read = read((int)fd, &(data->hdr), sizeof(message_data_hdr_t))) == -1 ) {
		return -1;
	}
    
    
    size_t len = data->hdr.len;
    if (len == 0 ) data->buf = NULL;
    else {
		if ( data->buf != NULL) {
			free(data->buf);
			data->buf = NULL;
		}
		data->buf =(char *)malloc(sizeof(char)*len);
		//memset(data->buf, 0, sizeof(char));
   
		//occhio per il test 5
		char *buf = NULL;
		buf =(char*)data->buf;
    
    
		while( len > 0 ) {
			if ((bytes_read=read((int)fd ,buf,len)) == -1) {
				return -1;
			}
	
			if (bytes_read == 0) return -1; 
    
			len -= bytes_read;
			buf += bytes_read;
		}
	}
    return 1;
}



/**
 * @function readMsg
 * @brief Legge l'intero messaggio
 *
 * @param fd     descrittore della connessione
 * @param data   puntatore al messaggio
 *
 * @return <=0 se c'e' stato un errore
 *         (se <0 errno deve essere settato, se == 0 connessione chiusa) 
 */
int readMsg(long fd, message_t *msg) {
	pthread_mutex_lock(&mutex_read);
	memset(msg, 0, sizeof(message_t));
	
	int r = readHeader(fd, &(msg->hdr));
	if (r<=0){
		pthread_mutex_unlock(&mutex_read); 
		return r;
	}
	
	int d = readData(fd, &(msg->data));
	if (d<0) {
		pthread_mutex_unlock(&mutex_read); 
		return d;
	}
	pthread_mutex_unlock(&mutex_read); 
return 1;

}

/**
 * @function send_op_list
 * @brief invia al cliente lista utenti connessi e OP_OK
 * @param fd descrittore della connessione
 * @param msg puntatore al messaggio da inviare
 * @param buf lista utenti connessi
 * @param online numero di clienti connessi
 * 
 * @return 1 successo, -1 altrimenti
 */
int send_op_ok_list (long fd, message_t *msg,  char* buf,int online) {

	pthread_mutex_lock(&mutex_write);
	memset(msg, 0, sizeof(message_t));
	
	setHeader(&msg->hdr, OP_OK, "");
	printf("-------------utenti online: %d------------------------\n", online);					
	setData(&msg->data, "", buf, online*(MAX_NAME_LENGTH+1));
	
	if ( sendHeader(fd, &msg->hdr) <= 0) {
		pthread_mutex_unlock(&mutex_write);
		printf("Non è possibile inviare la lista degli utenti online\n");
		close(fd);
		return -1;
	}
	if ( sendData(fd, &msg->data) <= 0) {
		pthread_mutex_unlock(&mutex_write);
		printf("Non è possibile inviare la lista degli utenti online\n");
		close(fd);
		return -1;
	}
	pthread_mutex_unlock(&mutex_write);
	
return 1;
}
/**
 * @function send_op
 * @brief invia al client operazione opportuna
 * @param fd descrittore della connessione
 * @param msg  puntatore al messaggio da inviare
 * @param op operazione da inviare
 * 
 */	
void send_op (long fd, message_t* msg, op_t op) {
	memset(msg, 0, sizeof(message_t));
	
	setHeader(&msg->hdr, op, "");
	sendHeader(fd, &msg->hdr);
}


