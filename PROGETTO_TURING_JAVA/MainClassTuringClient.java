import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.util.ArrayList;
import java.util.Scanner;
/**
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class MainClassTuringClient {

	public static int port_registry = 9999;
	public static int port_tcp = 5000;
	public static String editor = "Notepad.exe";
	
	public static void main(String[] args) {
		if (args.length == 3) {
			if (args[0] != null) {
				port_registry = Integer.parseInt(args[0]);
				if (args[1] != null) 
					port_tcp = Integer.parseInt(args[1]);
				if (args[2] != null)
					editor = args[2];
			}
		}
			
		SocketChannel socketclient = null; 
		InetSocketAddress address;
		Registry r;
		ByteBuffer buffer = ByteBuffer.allocate(1024); //buffer usato nella comunicazione client-server
		boolean endclient = false; //se settata a true il client viene chiuso 
		Scanner input = new Scanner(System.in);
		String [] richiesta = null; 
		String Operazione; //contiene l'operazione da inoltrare al server
		String Nome = null;
		String Password = null;
		while (!endclient) {
			richiesta = input.nextLine().split("\\s"); 
			if (richiesta.length<=0) {
				System.out.println("Argomenti non validi, inserire il comando --help per info");
				continue;
			}
			switch (richiesta[0]) { //esamino il tipo di richiesta {register, login, help}
			case "--help": {
				System.out.println("usage: turing COMMAND [ARGS...]\r\n" + 
						"commands: register <username > <password > registra l’utente\n" + 
						"login <username > <password > effettua il login logout effettua il logout\n" + 
						"create <doc> <numsezioni > crea un documento\n" + 
						"share <doc> <username > condivide il documento\n" +
						"show <doc> <sec> mostra una sezione del documento\n"+ 
						"show <doc> mostra l’intero documento list mostra la lista dei documenti\n" + 
						"edit <doc> <sec> modifica una sezione del documento\n"+ 
						"end-edit <doc> <sec> fine modifica della sezione del doc\n" + 
						"send <msg> invia un msg sulla chat receive visualizza i msg ricevuti sulla chat\n"
						);
			} break;
			case "register": {
				Nome = null; Password = null;
				if (richiesta.length != 3) {
					System.out.println("Argomenti non validi, inserire username e password");
					continue;
				}
				Nome = richiesta[1]; Password = richiesta[2];
				if (Nome != null && Password != null) { 
					try {
						r = LocateRegistry.getRegistry(port_registry);
						Registrazione reg = (Registrazione) r.lookup("REGISTRAZIONE");
						if (reg.registrami(Nome, Password))
							System.out.println(Nome + " registrato");
						else System.out.println("Nome utente già usato");
					
					} catch (RemoteException | NotBoundException e) {
						// TODO Auto-generated catch block
						endclient = true;
						System.out.println("Impossibile collegarsi al registry o lookup");
					}
				
				} else System.out.println("Numero argomenti non valido, inserire Nome Utente e password");
			
			} break;
			case "login": {
				Nome = null; Password = null;
				if (richiesta.length != 3) {
					System.out.println("Argomenti non validi, inserire login username password");
					continue;
				}
				Nome = richiesta[1]; Password = richiesta[2];
				try {
					address = new InetSocketAddress("127.0.0.1", port_tcp);
					socketclient = SocketChannel.open(address); //apro il socket
					Operazione = "login " + Nome + " "+ Password; //stringa da inviare al server
					int login = ClientMethodsTuring.sendOp(Operazione, socketclient, buffer); //invio al server richiesta di login
					if (login == -1) {endclient= true;continue;} //non riesco a contattare il server
					if (login != 1) { 
						System.out.println("Operazione di login di "+ Nome + "fallita: password o username errato");
						continue;
					}
					//login riuscito
					System.out.println("login "+ Nome+ " avvenuto con successo");
					//creo thread che si occupa delle notifiche che vengono inviate all'utente
					Notify notifiche = new Notify(Nome, port_tcp); 
					Thread threadnotify = new Thread(notifiche);
					threadnotify.setDaemon(true);
					threadnotify.start();
					boolean logout = false; 
					 while (!logout) {
						richiesta =null;
						richiesta = input.nextLine().split("\\s");
						if (richiesta.length <= 0) {
							System.out.println("inserire comando e parametri");
						}
						switch (richiesta[0]) { //esamino che tipo di richesta {create, show, edit, logout}
						case "logout": {
							System.out.println("stai effettuando il logout");
							logout = true;
						}break;
						case"create": {
							if (richiesta.length != 3) {
								System.out.println("Argomenti per create non validi: inserire nome documento e numero di sezioni");
								continue;
							}
							String documento = richiesta[1]; int num_sezioni;
							try {
								num_sezioni= Integer.parseInt(richiesta[2]);
							} catch (NumberFormatException e) { //se l'argomento di indice 2 in richiesta[] non è un intero
								System.out.println("inserire un numero che indichi la sezione, riprova la create");
								continue;
							}
							Operazione = "create"+ " " + documento + " " + num_sezioni;
							int flag;
							if ((flag=ClientMethodsTuring.sendOp(Operazione, socketclient, buffer)) == -1) { //errore nel conttare server, chiudo client
								logout = true; endclient = true; continue;
							} else if (flag != 1)  System.out.println("Non è stato possibile creare il documento");
							else System.out.println("documento creato");							
						}break;
						case "list" : {
							if (!ClientMethodsTuring.getlist("list", socketclient, buffer)) { //richiedo la lista dei documenti al server
								logout = true; endclient = true;
							}
						}break;
						case "share": {
							if (richiesta.length != 3) {
								System.out.println("Argomenti per share non validi: inserire nome documento e username");
								continue;
							}
							String doc = richiesta[1]; String username = richiesta[2];
							Operazione = "share"+ " "+ doc + " "+ username;
							int flag;
							if ((flag = ClientMethodsTuring.sendOp(Operazione,socketclient, buffer)) == -1) {
								logout = true; endclient = true; continue;
							} else if (flag != 1) System.out.println("Non è possibile condividere il documento");
							else System.out.println("Documento condiviso con successo");
						}break;
						case "show" : {
							if (richiesta.length < 2) {
								System.out.println("Argomenti per show non validi: inserire nome documento oppure nome documento e sezione");
								continue;
							}
							String Documento = richiesta[1];
							boolean flag= true;
							if (richiesta.length == 3) { //si sta chiedendo una sezione specifica
								int sezione=-1;
								try {
									sezione = Integer.parseInt(richiesta[2]);
								} catch (NumberFormatException e) {
									System.out.println("Sezione non valida, inserire un intero. Riprova la show");
									continue;
								}
								flag = ClientMethodsTuring.getlist("show "+ Documento+ " "+ sezione, socketclient, buffer); //richiedo sezione del documento
							} else if (richiesta.length == 2) flag=ClientMethodsTuring.getlist("show "+ Documento, socketclient, buffer); //richiedo tutto il documento
							if (!flag) { //connessione con il server caduta 
								logout = true; endclient = true; continue;
							}
						}break;
						case "edit": {
							if (richiesta.length != 3) {
								System.out.println("Argomenti per edit non validi: inserire nome documento e sezione");
								continue;
							}
							String Documento = richiesta[1]; String sezione = richiesta[2]; //tengo traccia del documento e sezione che si sta per editare (se concesso)
							int flag = ClientMethodsTuring.requestedit(Nome, Documento,sezione, socketclient, buffer, editor); //invio richiesta di editing
							if (flag == -1) { //connessione con il server caduta
								logout = true; endclient = true; continue;
							}
							if (flag == 0) {
								System.out.println("NON puoi editare"); continue;
							}
							System.out.println("Attendi di essere inserito nella chat");
							String [] ip_chat;
							MulticastSocket group = null;
							Thread chat = null;
							ArrayList<String> lista_messaggi = null; //lista dove salvo i messaggi che arrivano da altri utenti che stanno editando il documento comune
							int port = 0; 
							InetAddress addressg = null;
							ip_chat= ClientMethodsTuring.getIPChat("chat", socketclient, buffer).split("\\s"); //richiedo indirizzo chat
							lista_messaggi = new ArrayList<String>();
							ThreadChat c = new ThreadChat(ip_chat, lista_messaggi); //creo thread che si occupa di ricevere i messaggi della chat
							addressg = InetAddress.getByName(ip_chat[1]);
							port = Integer.parseInt(ip_chat[0]);
							group = new MulticastSocket();
							chat = new Thread(c); 
							chat.start(); //avvio il thread
							System.out.println("Sei stato inserito, ora puoi comunicare con gli altri clienti che stanno editando lo stesso documento!");
							boolean edit = false;
							if (flag != 0) edit = true;
							while (edit) {
								richiesta = null;
								richiesta = input.nextLine().split("\\s"); 
								if (richiesta.length == 0) continue;
								switch (richiesta[0]) { //richiesta {send, receive, end-edit}
								case "end-edit": {
									if (richiesta.length != 3) {
										System.out.println("End-edit: argomenti non validi");
										continue;
									}
									if (Documento.equals(richiesta[1]) && sezione.equals(richiesta[2])) //controllo che il nome del documento e sezione siano quelli che si stanno editando
										if (ClientMethodsTuring.sendSezione("end-edit "+ Documento + " "+ sezione,"C:\\Client"+Nome+"\\"+Documento+"sezione"+sezione+".txt", socketclient, buffer)== false) {
											logout = true; endclient = true; 
										}
									System.out.println("Abbiamo finito di editare");
									edit =false;
									chat.interrupt(); //esco dalla chat
									group.close();
								}break;
								case "send": {
									String messaggio = "["+Nome+"]";
									for (int i = 1; i< richiesta.length; i++)
										messaggio = messaggio + " "+richiesta[i];
									DatagramPacket packet = new DatagramPacket(messaggio.getBytes(), messaggio.length(), addressg,port);
									group.send(packet);
								}break;
								case "receive": {
									synchronized (lista_messaggi) {
										while (lista_messaggi.size() > 0) { //leggo tutti i messaggi inviati sulla chat non ancora letti
											System.out.println(lista_messaggi.get(0));
											lista_messaggi.remove(0);
										}
									}
								}break;
								default: {
									System.out.println("operazione non ammessa.Devi prima effettuare l'end-edit del documento che si sta editando");
								}break;
								}
							}
						} break;
						default: {
							System.out.println("Operazione non ammessa. Le operazioni ammesse da LOGGATO sono le seguenti:\n"
								+"create <doc> <numsezioni > crea un documento\n" + 
								"share <doc> <username > condivide il documento\n" +
								"show <doc> <sec> mostra una sezione del documento\n"+ 
								"show <doc> mostra l’intero documento list mostra la lista dei documenti\n" + 
								"edit <doc> <sec> modifica una sezione del documento\n"+ 
								"end-edit <doc> <sec> fine modifica della sezione del doc\n" + 
								"send <msg> invia un msg sulla chat receive visualizza i msg ricevuti sulla chat\n");
						}break;	
						}	
					}
					 //cancello file temporanei associati all'utente usati per effettuare editing in precedenza
					 try (DirectoryStream<Path> entries = Files.newDirectoryStream(Paths.get("C:\\Client"+Nome))) {
							for (Path entry : entries) {
								Files.delete(entry);
						      }
							 Files.delete(Paths.get("C:\\Client"+Nome)); 
						} catch (IOException e) {}
					socketclient.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					System.out.println("Impossibile connettersi al server");
					endclient = true; 
				}
			} break;
			default: {
				System.out.println("Operazione non concessa. Prima di fare qualsiasi operazione è necessario effettuare il login");
			}break;
			case "termina": endclient = true; break;
			}
		}
		input.close();
		
	}
}