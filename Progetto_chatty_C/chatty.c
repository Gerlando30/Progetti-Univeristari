/*
 * membox Progetto del corso di LSO 2017/2018
 *
 * Dipartimento di Informatica Università di Pisa
 * Docenti: Prencipe, Torquati
 * 
 * \file chatty.c  
     \author Gramaglia Gerlando 530269
      Si dichiara che il contenuto di questo file e' in ogni sua parte opera  
      originale dell'autore  
 * 
 * 
 */
/**
 * @file chatty.c
 * @brief File principale del server chatterbox
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stats.h>
#include <message.h>
#include <config.h>
#include <lista_task.h>
#include <set_descr.h>
#include <tabClient.h>

#define CONTROL_MENOUNO(VAL, STRINGA)	\
	if (VAL == -1) {	\
		perror(#STRINGA);	\
	} 




volatile sig_atomic_t termina= 0;	//flag usato per la terminazione del thread listener
static int fd_conn=0;				//file descriptor del socket su cui accettiamo le connessioni
set_descriptor descriptors;			//struttura contenente maschera dei descrittori delle connessione e altri parametri	(set_descr.h)
tab_clienti* tabella=NULL;			//struttura contenente i clienti registrati e altri parametri						(tabClient.h)
users_online *users=NULL;			//struttura contenente la lista degli utenti online e altri paramentri				(fdclient.h)
coda_task* coda_fd=NULL;			//struttura contenente la lista degli fd											(lista_task.h)
static pthread_t* Pool=NULL;		//threads Pool

int MaxConnection = 0;
int ThreadsInPool = 0;
int MaxMsgSize = 0;
int MaxFileSize = 0;
int MaxHistMsgs = 0;
char DirName[32];
char StatFileName[32];
char UnixPath[32];





/* struttura che memorizza le statistiche del server, struct statistics 
 * e' definita in stats.h.
 *
 */
 
//fase di gestione delle statistiche  
struct statistics  chattyStats = { 0,0,0,0,0,0,0 };

pthread_mutex_t statlock; //lock statistiche 

/**
 * @function inc
 * @brief incrementa di un certo valore la statistica opportuna
 *
 * @param val puntatore alla specifica statistica prensente in chattyStats
 * @param incremento valore da sommare a val
 */
void inc (long unsigned int* val, int incremento) {
	pthread_mutex_lock(&statlock);
	*val += incremento;
	pthread_mutex_unlock(&statlock);
}

/**
 * @function dec
 * @brief decrementa di un certo valore la statistica opportuna
 *
 * @param val puntatore alla specifica statistica prensente in chattyStats
 * @param incremento valore da sottrarre a val
 */
void dec (long unsigned int* val, int decremento) {
	pthread_mutex_lock(&statlock);
	*val -= decremento;
	pthread_mutex_unlock(&statlock);
}

/**
 * @function control_reg_with_close
 * @brief controlla se l'utente è registrato utilizzando l'opportuna funzione della libreria tabClient.h 
 * in caso negativo invia l'op OP_NICK_UNKNOWN e chiude la connessione
 * 
 * @param fd il file descriptor dell'utente 
 * @param msg messaggio contenente nome dell'utente da controllare 
 */
static int control_reg_with_close(int fd, message_t msg) {
	if (utente_registrato(tabella, msg.hdr.sender) == -1) {
		free(msg.data.buf);
		msg.data.buf =NULL;
		send_op(fd, &msg, OP_NICK_UNKNOWN);
		printf("Utente %s : non registrato\n", msg.hdr.sender);
		close(fd);
		//inc(&chattyStats.nerrors, 1);
		return -1;
	}
	
	return 1;
}
/**
 * @function close_or_add_user_online
 * @brief controlla se l'utente può connettersi utilizzando l'opportuna funzione della libreria fdclient.h
 * in caso negativo invia l'op OP_FAIL e chiude la connessione
 * 
 * @param fd il file descriptor dell'utente 
 * @param msg messaggio contenente nome dell'utente da controllare 
 * @return 1 utente aggiunto; 0 utente già online; -1 fail
 */
static int close_or_add_user_online ( int fd, message_t msg ) {
	int ok;
	if ( (ok =add_user_online(users, fd, msg.hdr.sender)) == -1 ){
		free(msg.data.buf);
		msg.data.buf =NULL;
		send_op(fd, &msg, OP_FAIL);
		printf("Non puoi connetterti : %s\n",  msg.hdr.sender);
		close(fd);
		//inc(&chattyStats.nerrors, 1);
		return -1;
	}
	return ok; 
}

/**
 * @function print_stat
 * @brief stampa le statistiche 
 * 
 */
void print_stat () {

FILE* filestat=fopen(StatFileName, "w");	
if(filestat==NULL){     
	perror("Non è possibile aprire il file delle statistiche");
	exit(EXIT_FAILURE);
} 
	pthread_mutex_lock(&statlock);
	
	if (fprintf(filestat, "%ld - %ld %ld %ld %ld %ld %ld %ld\n",
		(unsigned long)time(NULL),
		chattyStats.nusers, 
		chattyStats.nonline,
		chattyStats.ndelivered,
		chattyStats.nnotdelivered,
		chattyStats.nfiledelivered,
		chattyStats.nfilenotdelivered,
		chattyStats.nerrors) 
		< 0) perror("Impossibile stampare le statistiche nel file");
		else printf("Statistiche inserite con successo!\n");
		 
	pthread_mutex_unlock(&statlock);
	
	fclose(filestat);
}

static void usage(const char *progname) {
    fprintf(stderr, "Il server va lanciato con il seguente comando:\n");
    fprintf(stderr, "  %s -f conffile\n", progname);
}


/**
 * @function configuration
 * @brief assegna alle variabili di configurazione i valori opportuni
 * ottenuti analizzando il file di configurazione riga per riga
 * 
 * @param conf stringa contenente il nome del file di configurazione
 */

void configuration (const char* conf) {	
	FILE *fd;
	char buf[200];
	char *res;

	
	fd=fopen(conf, "r");
	if( fd==NULL ) {
		perror("Errore in apertura del file");
		exit(1);
	}
	
	while(1) {
		res=fgets(buf, 200, fd);
		if( res==NULL )
		break;
		
		char* token = strtok( buf, " =		");
		if (token) {
			if (strcmp( token , "MaxConnections") == 0) {
				printf("%s ", token);
				token = strtok(NULL, " =	");
				int valore = strtol(token, NULL, 10);
				if (errno == ERANGE) {
					perror("Range error");
					exit(EXIT_FAILURE);
				}
				MaxConnection = valore;
				printf("%d\n", MaxConnection);
			}
			else if (strcmp( token , "ThreadsInPool" ) == 0) {
				printf("%s ", token);
				token = strtok(NULL, " =	");
				int valore = strtol(token, NULL, 10);
				if (errno == ERANGE) {
					perror("Range error");
					exit(EXIT_FAILURE);
				}
				ThreadsInPool = valore;
				printf("%d\n", ThreadsInPool);
			}
			else if (strcmp( token , "MaxMsgSize" ) == 0) {
				printf("%s ", token);
				token = strtok(NULL, " =	");
				int valore = strtol(token, NULL, 10);
				if (errno == ERANGE) {
					perror("Range error");
					exit(EXIT_FAILURE);
				}
				MaxMsgSize = valore;
				printf("%d\n", MaxMsgSize);
			}
			else if (strcmp( token , "MaxFileSize") == 0) {
				printf("%s ", token);
				token = strtok(NULL, " =	");
				int valore = strtol(token, NULL, 10);
				if (errno == ERANGE) {
					perror("Range error");
					exit(EXIT_FAILURE);
				}
				MaxFileSize = valore;
				printf("%d\n", MaxFileSize);
			}
			else if (strcmp( token , "MaxHistMsgs") == 0) {
				printf("%s ", token);
				token = strtok(NULL, " =	");
				int valore = strtol(token, NULL, 10);
				if (errno == ERANGE) {
					perror("Range error");
					exit(EXIT_FAILURE);
				}
				MaxHistMsgs = valore;
				printf("%d\n", MaxHistMsgs);
			}
			else if (strcmp( token , "UnixPath") == 0) {
				printf("%s ", token);
				token = strtok(NULL, " =	 \n");
				if (strlen(token) > 32) {
					perror("Nome UnixPath troppo lungo");
					exit(EXIT_FAILURE);
				}
				strncpy(UnixPath, token, 32);
				printf("%s\n", UnixPath);
			}
			else if (strcmp( token , "DirName") == 0) {
				printf("%s ", token);
				token = strtok(NULL, " =	\n");
				if (strlen(token) > 32) {
					perror("Nome DirName troppo lungo");
					exit(EXIT_FAILURE);
				}
				strncpy(DirName, token, 32);
				printf("%s\n", DirName);
			}
			else if (strcmp( token , "StatFileName") == 0) {
				printf("%s ", token);
				token = strtok(NULL, " =	\n");
				if (strlen(token) > 32) {
					perror("Nome StatFileName troppo lungo");
					exit(EXIT_FAILURE);
				}
				strncpy(StatFileName, token, 32);
				printf("%s\n", StatFileName);
			}
		
		}
	}
  fclose(fd);
}

/**
 * @function signal_thread
 * @briefsi occupa della gestioni dei segnali 
 * 
 * @param maschera: maschera contenente i segnali da gestire
 */
void* signal_thread (void *maschera) {
	while (termina == 0) {
		int segnale;
		sigwait(maschera, &segnale);
		switch (segnale) {
			case SIGQUIT:
			case SIGTERM:
			case SIGINT: {
				termina = 1;
			}break;
			case SIGUSR1: {
				print_stat();
			}break;
		}
	}
return NULL;
}

/**
 * @function gestisci richiesta
 * @brief gestisce la richiesta di un determinato fd
 */


void* gestisci_richiesta () {
	message_t msg;
	msg.data.buf = NULL;	//contenuto del messaggio ricevuto
	int num_online;			//copia numero utenti online
	int fd;					//descrittore della connessione
	
	int ok = 0;				//flag di controllo
	while (1) {
		
		fd = DeQueue(coda_fd); //estraggo il file descriptor in cima alla lista 
		if (fd == -1) break;   //ho ricevuto dal listener il messaggio speciale di terminazione
		
		memset(&msg, 0, sizeof(message_t));
				
		int control_read = readMsg(fd, &msg);  //leggo la richiesta di fd
		
		//se la lettura non è andata a buon fine chiudo la connessione e se rimuovo l'utente dalla lista degli utenti online (se presente)
		if ( control_read <0) {
			free(msg.data.buf);
			msg.data.buf =NULL;
			inc(&chattyStats.nerrors, 1);
			perror("Errore con readMsg, con il fd");
			if ( remove_user_online(users, fd) == 1) {
				dec(&chattyStats.nonline, 1);
				printf("utente con fd: %d disconnesso\n", fd);
				
			}
			close(fd);
		} else if ( control_read == 0) {
			if (remove_user_online(users, fd) == 1) {
				free(msg.data.buf);
				msg.data.buf =NULL;
				dec(&chattyStats.nonline, 1);
				perror("Disconnesso");
			}
			close(fd);
		}  else {  //altrimenti gestisco la richiesta
			op_t op = msg.hdr.op;
			switch (op) {
				case REGISTER_OP: {
					char* lista_users_online =NULL;
					if (utente_registrato(tabella,  msg.hdr.sender) == -1 ) { // se l'utente non è registrato 
						
						if (close_or_add_user_online(fd, msg) == 1 ){ //controllo se posso metterlo online (e quindi gestire la sua richiesta)  
							
							//utente aggiunto alla lista degli utenti online
							reg_client(msg.hdr.sender, tabella,  MaxHistMsgs);  //registro l'utente
							inc(&chattyStats.nonline, 1);
							inc(&chattyStats.nusers, 1);
							printf("registro cliente: %s\n", msg.hdr.sender);
							lista_users_online = getlist_user_online(users, &num_online);	//lista degli utenti online  
							free(msg.data.buf);
							msg.data.buf =NULL;
							if (send_op_ok_list(fd, &msg, lista_users_online, num_online) == -1 ){ //provo ad inviare la lista degli utenti online 
								perror("Non riusciamo ad inviare la lista");
							}
								set_fd(fd, &descriptors);
								free(msg.data.buf);
								msg.data.buf =NULL;
						}
					}
					else { //utente già registrato
						free(msg.data.buf);
						msg.data.buf =NULL;
						send_op(fd, &msg, OP_NICK_ALREADY);
						printf("utente %s già registrato\n", msg.hdr.sender);
						set_fd(fd, &descriptors);
					}
				} break;
				case CONNECT_OP: {
					int ok;		//flag di controllo
					char* lista_users_online =NULL;
					if (control_reg_with_close(fd, msg) != -1) {
						
						if ((ok = close_or_add_user_online(fd, msg)) != -1) {
							
							lista_users_online = getlist_user_online(users, &num_online);
							free(msg.data.buf);
							msg.data.buf =NULL;
							if (send_op_ok_list(fd, &msg, lista_users_online, num_online) != -1){
								if (ok == 1 ) inc(&chattyStats.nonline, 1);  //ok == 1 : nuovo utente aggiunto alla lista online 
								printf("utente connesso\n");
								set_fd(fd, &descriptors);
							} else {
								inc(&chattyStats.nerrors,1);
							}
							free(msg.data.buf);
							msg.data.buf =NULL;
						} 
					} 
				} break;
				case POSTTXT_OP: {
					if ( control_reg_with_close(fd, msg) != -1) {
						if ( utente_registrato(tabella, msg.data.hdr.receiver) == -1) {
						free(msg.data.buf);
						msg.data.buf= NULL;
						perror("Destinatario non registrato");
					} else {
							
							msg.hdr.op = TXT_MESSAGE;  
							if ( (ok = send_msg(tabella, msg.data.hdr.receiver, &msg, users, MaxMsgSize)) == -1 ){  //invio il messaggio utilizzando la funzione di libreria tabClient.h
							
							//errore nell'invio del messaggio 
							perror("Non è possibile inviare messaggio");
							inc(&chattyStats.nerrors, 1);
							free(msg.data.buf);
							msg.data.buf= NULL;
							send_op(fd, &msg, OP_FAIL);
							
							} else if ( ok ==-2 ) {  //messaggio troppo lungo
								free(msg.data.buf);
								msg.data.buf= NULL;
								send_op(fd, &msg, OP_MSG_TOOLONG);
								perror("impossibile inviare il messggio, troppo lungo");
								inc(&chattyStats.nerrors, 1);
							} else{  // messaggio inviato correttamente
								if ( ok == 0) inc(&chattyStats.nnotdelivered, 1);
								else inc(&chattyStats.ndelivered, 1);
								printf("Messaggio inviato\n");
								free(msg.data.buf);
								msg.data.buf= NULL;
								send_op(fd, &msg, OP_OK);
							}
							set_fd(fd, &descriptors); 
						}
					}
					
				} break;
				case POSTTXTALL_OP: {
					if ( control_reg_with_close(fd, msg) != -1) {
						msg.hdr.op = TXT_MESSAGE;
						if ( (ok =send_msg_to_all(tabella, &msg, users, MaxMsgSize)) >= 0 ) {  //invio messaggio a tutti gli utenti registrati
							free(msg.data.buf);
							msg.data.buf= NULL;
							send_op(fd, &msg, OP_OK);
							inc(&chattyStats.ndelivered, ok);
							inc(&chattyStats.nnotdelivered, (chattyStats.nusers - ok));
						}else if ( ok == -2 ){ //messaggio troppo lungo
							perror("Non è possibile inviare messaggio, troppo lungo");
							inc(&chattyStats.nerrors, 1);
							free(msg.data.buf);
							msg.data.buf =NULL;
							send_op(fd, &msg, OP_MSG_TOOLONG);
						} else if ( ok == -1 ){  //errore invio messaggio 
							perror("Non è possibile inviare messaggio");
							inc(&chattyStats.nerrors, 1);
							free(msg.data.buf);
							msg.data.buf =NULL;
							send_op(fd, &msg, OP_MSG_TOOLONG);
						}
						set_fd(fd, &descriptors);
					}
					
				} break;
				case GETPREVMSGS_OP: {
					msg.hdr.op = TXT_MESSAGE;
					if ( control_reg_with_close(fd, msg) != -1) {
						if ((ok = send_history(tabella, msg.hdr.sender, fd)) == -1)  {  //invio la history dei messaggi; in ok salvo i messaggi effettivamente inviati
							free(msg.data.buf);
							msg.data.buf= NULL;
							perror("Non è stato possibile inviare history ");
							inc(&chattyStats.nerrors, 1);
						}else {  //messaggi inviati
							free(msg.data.buf);
							msg.data.buf= NULL;
							inc(&chattyStats.ndelivered, ok);
							dec(&chattyStats.nnotdelivered, ok);
						}
					  set_fd(fd, &descriptors);
					}
					
				} break;
				case POSTFILE_OP: {
					msg.hdr.op = FILE_MESSAGE;
					if ( control_reg_with_close(fd, msg) != -1) {
						struct stat t;
					if (stat(DirName, &t) == -1) {  //controllo che la cartella esista, se non esite la creo
						if(mkdir(DirName, 0700)==-1){		
							free(msg.data.buf);
							msg.data.buf =NULL;
							perror("Impossibile creare Dir");
							}
						} else if ( utente_registrato ( tabella, msg.data.hdr.receiver) == -1){
						perror("ricevente non registrato");
						free(msg.data.buf);
						msg.data.buf =NULL;
						}else if ( (ok = send_msg(tabella, msg.data.hdr.receiver, &msg, users, MaxMsgSize)) == -1 ){  //invio il nome del file
							free(msg.data.buf);
							msg.data.buf =NULL;
							perror("Non è possibile inviare file");
							inc(&chattyStats.nerrors, 1);
							send_op(fd, &msg, OP_FAIL);
						} else if ( ok == -2 ){  //nome file troppo lungo
							perror("Non è possibile inviare file");
							free(msg.data.buf);
							msg.data.buf =NULL;
							send_op(fd, &msg, OP_MSG_TOOLONG);
						} else { //nome file inviato correttamente
						
							if (ok ==0) inc(&chattyStats.nfilenotdelivered, 1);
							else inc(&chattyStats.nfiledelivered, 1);
							
							char* name_file = malloc(sizeof(char)*(msg.data.hdr.len+1));
							strcpy(name_file, msg.data.buf);
							printf("%s\n", name_file);
							message_data_t msg_file;
							msg_file.buf = NULL;
							
							if (readData(fd, &msg_file)<= 0){
								free(msg.data.buf);
								msg.data.buf =NULL;
								inc(&chattyStats.nerrors, 1); 
								perror("Errore nella lettura file");
							} else if (msg_file.hdr.len > MaxFileSize*1024) {  //controllo grandezza file
								free(msg.data.buf);
								msg.data.buf =NULL;
								perror("Errore file troppo grande");
								}
								else {
								char* path = malloc(sizeof(char)*((strlen(DirName)+1) + (strlen(msg.data.buf)+1))); //creo il percorso file
								strcpy(path, DirName);
								char *name_file_slash = strrchr(name_file, '/');
								path = strcat(path, "/");
								if ( name_file_slash != NULL) path = strcat(path, name_file_slash);
								else path = strcat(path, name_file);
								printf("il path è %s\n", path);
								
								FILE* f;
								if ( (f = fopen( path, "w") )== NULL){ perror("NON RIUSCIAMO AD APRIRE IL FILE");}
								else if (fwrite( msg_file.buf, sizeof(char), msg_file.hdr.len, f) != msg_file.hdr.len) { //scrivo il contenuto del file che mi è stato inviato nel file f
									perror("errore");
									inc(&chattyStats.nerrors, 1);
								} else {
									free(msg.data.buf);
									msg.data.buf =NULL;
									send_op(fd, &msg, OP_OK);
								}
								free(msg_file.buf);
								fclose(f);
								free(name_file);
								free(path);
							}
							
						}
						if (msg.data.buf) free(msg.data.buf);
						msg.data.buf =NULL;
						set_fd(fd, &descriptors);
					}
					
				}break;
				case GETFILE_OP: {
					
					msg.hdr.op = FILE_MESSAGE;
					if ( control_reg_with_close(fd, msg) != -1) {
						
						char* path = malloc(sizeof(char)*((strlen(DirName)+1) + (strlen(msg.data.buf)+1))); //creo percorso file
						strcpy(path, DirName);
						strcat(path, "/");
						path = strcat(path, msg.data.buf);
						printf("il path del file è: %s\n", path);
						
						char* nome = calloc(MAX_NAME_LENGTH+1, sizeof(char));  
						strcpy(nome, msg.hdr.sender);
						
						if ( send_file(path, nome, tabella, users) == -1) perror("FILE NON INVIATO");  //invio il file con l'opportuna funzione di libreria presente in tabClient.h
						
						free(path);
						free(nome);
						set_fd(fd, &descriptors);
					}
					 
					if (msg.data.buf) free(msg.data.buf);
					msg.data.buf =NULL;
				
				
				}break;
				case USRLIST_OP: {
					char* lista_users_online = NULL;
					if ( control_reg_with_close(fd, msg) != -1) {
						free(msg.data.buf);
						msg.data.buf =NULL;
						lista_users_online = getlist_user_online(users, &num_online);
						if (send_op_ok_list(fd, &msg, lista_users_online, num_online) != -1){
							printf("utente connesso\n");
							set_fd(fd, &descriptors);
						} else inc(&chattyStats.nerrors, 1);
					}
					free(msg.data.buf);
					msg.data.buf =NULL;
				} break;
				//È NECESSARIO???????????????
				/*case DISCONNECT_OP: {
					if ( control_reg_with_close(fd, msg) != -1) {
						if ( remove_user_online(users, fd) ==1 ) {
							dec(&chattyStats.nonline, 1);
							send_op(fd, &msg, OP_OK);
							printf("Utente rimosso con successo\n");
							close(fd);
						} else send_op(fd, &msg, OP_NICK_UNKNOWN);
					}	
					
				} break;*/
				//SEMPRE SOPRA: È NECESSARIO?????????????????????
				case UNREGISTER_OP: {
					if ( unregister_user(tabella, msg.hdr.sender) != -1){
						dec(&chattyStats.nusers, 1);
						dec(&chattyStats.nonline,1);
						send_op(fd, &msg, OP_OK);
						remove_user_online(users, fd);
						close(fd);
					}
					else send_op(fd, &msg, OP_NICK_UNKNOWN);
				}break;
				default: {
					
					printf("operazione non riconosciuta \n");
					inc(&chattyStats.nerrors, 1);
					close(fd);
					}
			}
		}
		if (msg.data.buf!= NULL){ // se il buffer di msg non è ancora stato liberato, lo libero
				 free(msg.data.buf);
				msg.data.buf = NULL;
			}
}	
	
return NULL;
}
/**
 * @function initPool
 * @brief creo il threadPool
 * 
 */
void initPool () {
	
	Pool = malloc(sizeof(pthread_t)*ThreadsInPool);
	
	for ( int i = 0; i< ThreadsInPool; i++){
		if (pthread_create(&Pool[i], NULL, gestisci_richiesta, NULL) == -1){
			perror("Errore nella creazione dei thread");
			exit(EXIT_FAILURE);
		}
		}
			
}

/**
 * @function listener
 * @brief 
 */


void* listener () {
	
	int fd_client, fd_select;  
	fd_set* rdset =(fd_set*) malloc(sizeof(fd_set));  
	int fd_num;  //indica l'ultimo fd attivo
	struct timeval t;
	int val;
	while (termina == 0) {
		val=0;
		copyset(&descriptors, &fd_num, rdset);
		t.tv_sec = 0;
		t.tv_usec = 100000;
		
		CONTROL_MENOUNO( (val = select(fd_num+1, rdset, NULL, NULL, &t)), "ERRORE NELLA SELECT");
		if ( val == 0) continue;

		for (fd_select = 0; fd_select<=fd_num; fd_select++) {
			
			if (FD_ISSET(fd_select, rdset)) {  
				
				if ( fd_select == fd_conn ) { //stabiliamo la connessione
					CONTROL_MENOUNO((fd_client=accept (fd_conn, NULL, 0)), "ERRORE NELLA ACCEPT");
					set_fd(fd_client, &descriptors);	//setto il bit di fd_client

				}
				else {									//connessione già stabilita
					
					clr_fd(fd_select, &descriptors);	//setto a zero fd_select
					EnQueue(coda_fd, fd_select);		//aggiungo alla lista fd
				}
			}
		}
	}
	//mi è arrivato il flag termina == 1
	
	for (int i = 0; i< ThreadsInPool; i++) //aggiungo un messaggio speciale per far terminare i thread di Pool 
		EnQueue(coda_fd, -1);
		
	free(rdset);
	return NULL;
}	

int main(int argc, char *argv[]) {
		
	if ( argc < 3 ) {
		usage("./chatty");
		exit(EXIT_FAILURE);
	}
	char name_config[32];
	if ( strcmp( argv[1], "-f" ) != 0 ) {
		usage("./chatty");
		exit(EXIT_FAILURE);
	}
	strcpy(name_config, argv[2]); 
	configuration(name_config);	
	
	pthread_mutex_init(&statlock, NULL);
	
	
	tabella=init_tab(1013, ThreadsInPool);
	users = init_users_online(MaxConnection);
	coda_fd = init();
	
	
	sigset_t sig_mask, oldmask;    
	sigemptyset(&sig_mask); 
	//inserisco i segnali che voglio gestire nella maschera sig_mask
	sigaddset(&sig_mask, SIGQUIT);
	sigaddset(&sig_mask, SIGTERM);
	sigaddset(&sig_mask, SIGINT);
	sigaddset(&sig_mask, SIGUSR1);
	//blocco i segnali presenti nella maschera
	sigprocmask(SIG_BLOCK, &sig_mask, &oldmask);
	
	struct sigaction s;
	memset( &s, 0, sizeof(s) );
	s.sa_handler=SIG_IGN;
	sigaction(SIGPIPE,&s,NULL);  // ignoro i segnali SIGPIPE
	
	pthread_t sig; 
	pthread_create (&sig, NULL, &signal_thread, (void*) &sig_mask);		//thread gestione segnali  
	
	unlink(UnixPath);
	struct sockaddr_un sa;
	strcpy(sa.sun_path, UnixPath);
	sa.sun_family = AF_UNIX;
	fd_conn = socket(AF_UNIX, SOCK_STREAM, 0);
	int val;
	CONTROL_MENOUNO(( val = bind(fd_conn, (struct sockaddr *)&sa, sizeof(sa))), "ERRORE NELLA FASE DI BIND"); //assegna l'indirizzo in sa al socket
	
	CONTROL_MENOUNO((val=listen(fd_conn, MaxConnection)), "ERRORE NELLA FASE DI LISTEN");

	init_set(&descriptors, fd_conn);		//inizializzo il set di fd
	
	set_fd(fd_conn, &descriptors);			//setto il bit di fd_conn

	pthread_t th;
	if ( pthread_create( &th, NULL , listener, NULL) != 0 ) {  //thread del listener
		perror("ERRORE NELLA CREAZIONE DEL THREAD Listener()");
		exit(EXIT_FAILURE);
	} 
	
	initPool();
	
	//fase di terminazione: aspetto che i thread terminano e libero tutte le strutture allocate
	
	pthread_join(th, NULL);
	
	for (int i = 0 ; i< ThreadsInPool; i++)
		pthread_join(Pool[i], NULL);
	
	destroy(coda_fd);
	remove_all_user_online(users);
	
	
	pthread_join(sig, NULL);
	
	destroy_tab(tabella);
	
	
    return 0;
}
