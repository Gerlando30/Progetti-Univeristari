import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
/**
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class  MainClassTuringServer{

	public static int PORT = 5000; 
	public static int port_registry = 9999;
	public final static int BLOCK_SIZE = 1024;
	public static void main(String[] args) {  
		try {
			if (args.length == 2) {
				if (args[0] != null) {
					port_registry = Integer.parseInt(args[0]);
					if (args[1] != null) 
					PORT = Integer.parseInt(args[1]);
				}
			}
		} catch (NumberFormatException e) {}
			
		
		
		Registrazione reg_client = null;
		Registry r;
		if (!Files.exists(Paths.get("C:\\DOCUMENTI-TURING"))) {
			try {
				Files.createDirectories(Paths.get("C:\\DOCUMENTI-TURING"));
			} catch (IOException e) {
				System.out.println("Impossibile creare directory");
			}
		}
		HashMap<String,UtenteOnline> clienti_online = new HashMap<String, UtenteOnline>(); //se un utente è già online e proviamo a rifare il login value viene sovrascritto
		HashMap<String, Documento> tabella_documenti = new HashMap<String, Documento>(); //tabella che associa al nome del documento l'istanza della classe Documento corrispondente
		HashMap<String, chat> tab_chat = new HashMap<String, chat>(); //associa a un documento la coppia <indirizzo multicast, porta> necessari alla chat
		//FASE RMI
		try {
			reg_client = new RegistrazioneImpl();
			LocateRegistry.createRegistry(port_registry);
			r = LocateRegistry.getRegistry(port_registry);
			r.rebind("REGISTRAZIONE", reg_client);
		} catch (RemoteException e) {
			System.out.println("Impossibile creare il registro"); System.exit(0);
		}
		
		
		Selector selector= null;
		ServerSocketChannel server;
		try {
			server = ServerSocketChannel.open();
			server.bind(new InetSocketAddress("127.0.0.1", MainClassTuringServer.PORT)); 
			System.out.printf("[TURING server] %s in ascolto sulla porta %d\n", InetAddress.getLocalHost().getHostAddress(), MainClassTuringServer.PORT);
			server.configureBlocking(false); 
			selector = Selector.open(); 
			server.register(selector, SelectionKey.OP_ACCEPT); 
		} catch (IOException e) {
			System.out.println("Impossibile avviare il server"); System.exit(0);
			} 	
		while (true) {
			try {
				selector.select();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} 
			Set<SelectionKey> readyKeys = selector.selectedKeys(); 
			Iterator<SelectionKey> iterator = readyKeys.iterator(); 
			while (iterator.hasNext()) { 
				SelectionKey key = iterator.next(); 
				iterator.remove(); 
				try {
					if ((key.isValid()) && (key.isAcceptable())) {
					server = (ServerSocketChannel) key.channel(); 
					SocketChannel client = server.accept(); 
					client.configureBlocking(false); 
					client.register(selector, SelectionKey.OP_READ); //server non deve inviare nulla al client fin quando questo non invia una richiesta
				}	 
					
				if ((key.isValid()) && (key.isReadable())) { 
					String []Op = ServerMethods.LeggiOperazione(key, clienti_online, tabella_documenti).split("\\s");
					switch(Op [0]) {
					case "login" : {
						User user = reg_client.is_register(Op[1], Op[2]);  //controllo se l'utente è registrato 
						UtenteOnline utente = (UtenteOnline) key.attachment(); //oggetto contente info sull'utente
						if (!clienti_online.containsKey(Op[1]) && user != null) { //se l'utente esiste e non è già online
							String username = new String (Op[1]);
							clienti_online.put(username, null); //inserisco l'utente nella tabella degli utenti online
							
							if (utente == null) utente = new UtenteOnline(user, username); //se l'oggetto sulle info non esiste lo creo
							else {utente.setUser(user); utente.setUsername(username);}
							
							utente.setOperation("login", 1);
							key.attach(utente); //associo info all'utente corrispondente 
							clienti_online.put(username, utente);
						} else if (utente != null) utente.setOperation("fail", 0); //l'ogetto sulle info dell'utente viene creato anche se il login fallisce
						else {
							utente = new UtenteOnline(null, null); 
							utente.setOperation("fail", 0);
							key.attach(utente);
						}
						key.interestOps(SelectionKey.OP_WRITE);
					}break;
					case "notify": { //richiesta arrivata dal thread che gestisce le notifiche dell'utente. Inviata in automatico quando si effettua il login con successo. 
						UtenteOnline utente = clienti_online.get(Op[1]); 
						if (utente == null ) key.channel().close(); //se l'utente associato al thread di notifica non è online allora chiudo la connessione
						else {
							key.attach(utente);
							utente.setSocket_notify((SocketChannel) key.channel()); //aggiorno info dell'utente
							ServerMethods.sendMessaggiPendenti(key, clienti_online); //invio i messaggi pendenti
						}
					} break;
					case "chat": { //invita in automatico NON esplicitamente
						key.interestOps(SelectionKey.OP_WRITE);
					}break;
					case "create": {
						String NomeDocumento = Op[1]; int num_sezioni = Integer.parseInt(Op[2]);
						UtenteOnline utente = (UtenteOnline) key.attachment(); 
						if (!tabella_documenti.containsKey(NomeDocumento)) { //se il nome del documento che si vuole creare NON è già in uso 
							Documento doc = new Documento(utente.getUserName(), NomeDocumento, num_sezioni);
							clienti_online.get(utente.getUserName()).addDocumento(NomeDocumento);
							tabella_documenti.put(NomeDocumento, doc);
							utente.setOperation("create", 1);
						} else utente.setOperation("fail", 0);
						key.interestOps(SelectionKey.OP_WRITE);
					}break;
					case "list": {
						UtenteOnline utente = (UtenteOnline) key.attachment();
						String lista = ServerMethods.getList(key, tabella_documenti);  //reperisco la lista dei documenti
						System.out.println(lista);
						utente.setOperation("list", lista);
						key.interestOps(SelectionKey.OP_WRITE);
					} break;
					case "share": {
						String doc = Op[1]; String username= Op[2]; //controllare che sia registrato
						UtenteOnline utente = (UtenteOnline) key.attachment(); Documento d = tabella_documenti.get(doc);
						if (utente.getUser() != null && d != null) { //se il documento usiste e..
							UtenteOnline utenteonline = clienti_online.get(username);
							if (utenteonline == null) {
								User user = reg_client.is_register(username); 
								if (user != null) { //se utente con cui si vuole condividere il documento è registrato DA CAMBIARE QUESTO CONTROLLO
									utente.setOperation("share", 1);
									user.addMessaggioPendente("Sei stato aggiunto al documento: " +doc); //notifico 
									d.invitaCollaboratore(utente.getUserName(), username); //aggiunto un nuovo nuovo utente nella lista degli utenti che possono modificare il documento
								} else utente.setOperation("fail", 0);
							} else {
								d.invitaCollaboratore(utente.getUserName(), username);
								utente.setOperation("share", 1);
								ServerMethods.sendNotifica(doc, utenteonline);
							}
							key.interestOps(SelectionKey.OP_WRITE);
						} else ServerMethods.disconnect(key, clienti_online, tabella_documenti);
					}break;
					case "show": {
						String doc = Op[1];
						UtenteOnline utente = (UtenteOnline) key.attachment();
						if (Op.length == 3) { //utente ha chiesto una sezione specifica
						int sezione = Integer.parseInt(Op[2]);
						utente.setOperation("show", doc, sezione);
						}else if (Op.length == 2) //utente chiede tutto il documento
							utente.setOperation("show", doc, -1);
						key.interestOps(SelectionKey.OP_WRITE);
					}break;
					case "edit": { //utente ha chiesto di modificare una sezione di un documento
						UtenteOnline utente = (UtenteOnline) key.attachment(); //reperisco info sull'utente
						//reperisco documento 
						Documento doc = tabella_documenti.get(Op[1]); int sezione = Integer.parseInt(Op[2]); 
						if (doc != null) {
							String PathSezione = doc.locksezione(utente.getUserName(), sezione); //reperisco il path della sezione
							if (PathSezione != null) { //se la sezione è libera ed esiste
								utente.setOperation("edit", new String (Op[1]), sezione);
								utente.setediting();
								key.interestOps(SelectionKey.OP_WRITE); continue;
							}
						}
						utente.setOperation("fail", (long)0);
						key.interestOps(SelectionKey.OP_WRITE);
						} break;
					case "end-edit": { 
						UtenteOnline utente = (UtenteOnline) key.attachment();
						utente.setOperation("end-edit", 1);
						key.interestOps(SelectionKey.OP_WRITE); 
					} break;
					default:break;
					}
				}	
				if ((key.isValid()) && (key.isWritable())) { 
					SocketChannel client = (SocketChannel) key.channel(); 
					UtenteOnline utente = (UtenteOnline) key.attachment();
					String operation = utente.getOperation();
					switch (operation) {  //completo le operazioni richeste dagli utenti
					case "login":
					case "create":
					case "share":
					case "fail": { //invio un flag per avvenuta/rifiutata operazione
						ByteBuffer buffer = utente.getBuffer();
						while (buffer.hasRemaining())
							client.write(buffer);
						buffer.clear();
						key.interestOps(SelectionKey.OP_READ);
					}break;
					case "list":
					case "msg": { //invio una stringa che contiene la lista dei documenti oppure i messaggi inviati da altri utenti
						String messaggio = utente.getMessaggio();
						ByteBuffer buffer = ByteBuffer.allocate(messaggio.length());
						buffer.clear(); 
						buffer.put(messaggio.getBytes());
						buffer.flip();
						while (buffer.hasRemaining())
							client.write(buffer);
						buffer.clear();
						utente.setMessaggio(null);
						key.interestOps(SelectionKey.OP_READ);
					}break;
					case "show": { //invio il contenuto della sezione o dell'intero documento richiesto
						String NomeDocumento = utente.getNomeDocumento();
						Documento doc = tabella_documenti.get(NomeDocumento); 
						int sezione = utente.getsezione(); int fail = 0;
						if (doc != null) { //documento esiste
							if (sezione == -1) fail = ServerMethods.sendDocumento(key, doc, clienti_online, tabella_documenti);
							else if (sezione < doc.getNumero_sezioni()) {
								String Pathsezione= doc.getPathSezione(sezione);
								fail =ServerMethods.sendShow(key, Pathsezione,clienti_online, tabella_documenti);
							}
							ServerMethods.sendMessaggioGenerico(key, "\nfinish\n", clienti_online, tabella_documenti); //messaggio che indica al client che ho finito di inviare dati
						} 
						if (fail == 0) {
							utente.setOperation("msg", "Impossibile inviare il documento\nfinish\n");
							key.interestOps(SelectionKey.OP_WRITE);
						} else key.interestOps(SelectionKey.OP_READ);
						
					} break;
				case "edit": {
					Documento doc = tabella_documenti.get(utente.getNomeDocumento());
					String PathSezione = doc.getPathSezione(utente.getsezione()); int flag = 0;
					if (PathSezione != null) {
						flag = ServerMethods.sendSezione(key, PathSezione, clienti_online, tabella_documenti);
					}
					if (flag == 0 || PathSezione == null) {
						utente.setOperation("fail", (long)0);
						key.interestOps(SelectionKey.OP_WRITE);
						continue;
					}
					else { //ho inviato la sezione, aggiungo l'utente alla chat associata al documento che sta editando
						chat ip_chat = tab_chat.get(utente.getNomeDocumento()); //reperisco indirizzo e porta della chat
						if (ip_chat == null) { //se nessuno sta editando il documento
							ip_chat = ServerMethods.createchat(tab_chat);
							tab_chat.put(utente.getNomeDocumento(), ip_chat);
							System.out.println(ip_chat.getPort()+ ip_chat.getAddress().toString());
						}
						utente.setOperation("msg", ip_chat.getPort()+ " "+ ip_chat.getAddress().getHostAddress()+"\nfinish\n");
					}
					key.interestOps(SelectionKey.OP_READ);
				}break;
				case "end-edit": {
						Documento doc = tabella_documenti.get(utente.getNomeDocumento()); int sezione = utente.getsezione();
						long bytes = 0;
						ByteBuffer buffer = utente.getBuffer(); buffer.clear(); buffer.putInt(1);
						buffer.flip(); 
						try {
							//leggo la dimensione della sezione
							client.write(buffer); 
							buffer.clear();
							while (bytes < 8) { 
								bytes += client.read(buffer);
							}
							buffer.flip(); long size = buffer.getLong();
							buffer.compact();
							FileChannel file;
							try {
								file = FileChannel.open(Paths.get(doc.getPathSezione(sezione)), StandardOpenOption.WRITE); //apro la sezione dove andare a ricopiare il contenuto inviato
							} catch (IOException e) {
								System.out.println("Impossibile aprire il file"); return;
							}
							while (bytes <= size) { //fino a quando non ho letto tutti i bytes della sezione modificata
								bytes += client.read(buffer);
								buffer.flip();
								while(buffer.hasRemaining())
									file.write(buffer); 
								buffer.clear();
							}
							file.close();
							doc.unlocksezione(sezione);
							utente.end_edit();
							key.interestOps(SelectionKey.OP_READ);
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							ServerMethods.disconnect(key, clienti_online, tabella_documenti);
						}
					}break;
					}
					}
				} catch (IOException e) {ServerMethods.disconnect(key, clienti_online, tabella_documenti);}
				}
			}
		}
	}
		
		
