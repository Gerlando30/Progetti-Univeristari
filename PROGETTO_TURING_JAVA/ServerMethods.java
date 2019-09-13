import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;
import java.nio.channels.WritableByteChannel;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;
import java.util.Vector;
/**
 * Insieme di metodi statici utilizzati dal server per rispondere a una richiesta dal client
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class ServerMethods {
	/**
	 * 
	 * @param doc nome del documento che riguarda la notifica
	 * @param utente a cui inviare la notifica
	 */
	static void sendNotifica(String doc, UtenteOnline utente) {
		SocketChannel notify = utente.getSocket_notify(); 
		String msg = "Puoi modificare il documento: " + doc+ '\n';
		System.out.println(notify);
		if (notify == null) { //se il thread notify non è ancora attivo
			utente.getUser().addMessaggioPendente(msg);
			return;
		} else { //invio la notifica
			try {
				ByteBuffer buffer = ByteBuffer.allocate(msg.length());
				buffer.put(msg.getBytes());
				buffer.flip();
				System.out.println("inviamo "+ msg);
				while (buffer.hasRemaining()) {
					notify.write(buffer);
				}
				
				buffer.clear();
			} catch (IOException e) { //se invio fallisce, aggiungo il messaggio nella lista messaggi pendenti per inviarla in un secondo momento
				utente.getUser().addMessaggioPendente(msg);
			}
		}
	}
	/**
	 * 
	 * @param key chiave associata all'utente che richiede la lista dei documenti che può modificare 
	 * @param tabella_documenti tabella da cui reperiamo il documento richiesto
	 * @return lista sottoforma di stringa con tutti i documenti modificabili + creatore + collaboratori
	 * 			se è vuota ritorna stringa contenente un messaggio speciale
	 */
	static String getList(SelectionKey key, HashMap<String, Documento> tabella_documenti) {
		// TODO Auto-generated method stub
		UtenteOnline utente = (UtenteOnline) key.attachment();
		ArrayList<String> list_doc = utente.getListaDocumenti();
		String list = "";
		if (list_doc == null || list_doc.size() == 0) //se non ci sono documenti associati all'utente
			list = "Non ci sono documenti che puoi modificare\n";
		else { 
			for (int i = 0; i<list_doc.size(); i++) { 
				Documento d = tabella_documenti.get(list_doc.get(i));
				list = list + "\n"+d.getNomeDocumento()+"\n"+ 
				"Creatore: "+ d.getCreatore() + "\n"+
						d.getCollaboratori()+ "\n";
			}
		}
		list += "finish\n";//messaggio speciale che indica la terminazione della lista
		return list;
	}
	/**
	 * 
	 * @param key chiave associata all'utente che chiede l'operazione
	 * @param clienti_online tabella contenente tutti i clienti che sono online 
	 * 
	 */
	static void sendMessaggiPendenti(SelectionKey key, HashMap<String, UtenteOnline> clienti_online) {
		
		UtenteOnline utente = (UtenteOnline) key.attachment();
		Vector<String> list_doc = utente.getMessaggiPendenti();
		String list = "";
		if (list_doc == null) list = "Non ci sono messaggi pendenti\n";
		else {
			for (int i = 0; i<list_doc.size(); i++) //concateno i messaggi pententi in una stringa
				list = list +list_doc.get(i) + "\n";
		}
		//invio stringa
		ByteBuffer buffer = ByteBuffer.allocate(list.length());
		buffer.put(list.getBytes());
		SocketChannel socketclient = (SocketChannel) key.channel();
		buffer.flip();
		
			try {
				while (buffer.hasRemaining())
					socketclient.write(buffer);
				utente.removeMessaggiPendenti();
				buffer.clear();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				disconnect(key, clienti_online, null);
			}
		
	}
	/**
	 * reperisco l'operazione richiesta dall'utente
	 * @param key chiave associata all'utente che ha chiesto l'operazione
	 * @param clienti_online tabella dei clienti online
	 * @param tabella_documenti tabella dei documenti 
	 * @return stringa che rappresenta l'operazione richiesta
	 */
	static String LeggiOperazione(SelectionKey key, HashMap<String, UtenteOnline> clienti_online, HashMap<String,Documento> tabella_documenti) {
		// TODO Auto-generated method stub
		ByteBuffer buffer = ByteBuffer.allocate(1024);
		SocketChannel client = (SocketChannel) key.channel();
		StringBuilder Op = new StringBuilder();
		int bytes= 0;
		try {
			while ((bytes=client.read(buffer)) >0) {
				buffer.flip();
				while (buffer.hasRemaining()) {
					char c = (char)buffer.get();
					Op.append(c);
				}
				buffer.clear();
			}
			if (bytes == -1) disconnect(key, clienti_online, tabella_documenti);
		} catch (IOException e) {
			disconnect(key, clienti_online, tabella_documenti);
		}
		String Operazione = Op.toString();
		System.out.println(Operazione);
		
		return Operazione;
	}
	/**
	 * 
	 * @param key chiave utente da disconnettere
	 * @param clienti_online tabella degli utenti online
	 * @param tabella_documenti tabella dei documenti
	 */
	static void disconnect(SelectionKey key, HashMap<String,UtenteOnline> clienti_online, HashMap<String,Documento> tabella_documenti) {
		System.out.println("Disconnettiamo un utente");
		UtenteOnline utente = (UtenteOnline) key.attachment();
		key.cancel();
		try {
			key.channel().close();
		} catch (IOException e1) {}
		if (utente != null) { //se ci sono info associate all'utente 
			String Nomedocumento = utente.getNomeDocumento(); 
			SocketChannel notify = utente.getSocket_notify();
			if (notify != null) //chiudo la connessione verso il thread notify associato all'utente
				try {
					notify.close();
				} catch (IOException e) {}
			Documento doc = null;
			if ((doc = tabella_documenti.get(Nomedocumento)) != null && utente.getsezione()>=0 && utente.editing() == true) //se l'utente stava editando un documento
				doc.unlocksezione(utente.getsezione());
				clienti_online.remove(utente.getUserName()); 
		}
	}
	/**
	 * 
	 * @param key chiave associata all'utente
	 * @param PathSezione il percorso file della sezione da inviare
	 * @param clienti_online tabella dei clienti online
	 * @param tabella_documenti tabella dei documenti
	 * @return 1 se la sezione viene inviata correttamente, 0 altrimenti
	 */
	public static int sendSezione(SelectionKey key, String PathSezione, HashMap<String,UtenteOnline> clienti_online, HashMap<String,Documento> tabella_documenti) {
		try {
			FileInputStream fis;
			if (PathSezione == null || !Files.exists(Paths.get(PathSezione))) return 0; //controllo che la sezione esiste 
			fis = new FileInputStream(PathSezione); 
			FileChannel channel = fis.getChannel();
			//invio la dimensione del file
			ByteBuffer buffer = ByteBuffer.allocate(Long.SIZE);
			buffer.clear(); 
			buffer.putLong((channel.size()+1));
			System.out.println("la dimensione del file è "+channel.size());
			buffer.flip(); 
			while (buffer.hasRemaining()) 
				((SocketChannel) key.channel()).write(buffer); 
			buffer.clear(); 
			//invio il file
			channel.transferTo(0,channel.size(), (WritableByteChannel) key.channel());
			channel.close();
			fis.close();
		} catch (FileNotFoundException e) {
			System.out.println("File non trovato");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			disconnect(key, clienti_online, tabella_documenti); return 0;
		}
		return 1;
	}
	/**
	 * 
	 * @param key chiave associata all'utente
	 * @param messaggio da inviare all'utente
	 * @param clienti_online tabella dei clienti online
	 * @param tabella_documenti tabella dei documenti
	 */
	public static void sendMessaggioGenerico(SelectionKey key, String messaggio, HashMap<String,UtenteOnline> clienti_online, HashMap<String,Documento> tabella_documenti)  {
		
			try {
				ByteBuffer buffer = ByteBuffer.allocate(messaggio.length());
				buffer.clear();
				buffer.put(messaggio.getBytes());
				buffer.flip();
				SocketChannel socketclient = (SocketChannel) key.channel();
				while (buffer.hasRemaining())
					socketclient.write(buffer);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				disconnect(key, clienti_online, tabella_documenti);
			}
		
	}
	/**
	 * 
	 * @param key chiave associata all'utente
	 * @param Documento documento da inviare
	 * @param clienti_online tabella clienti online
	 * @param tabella_documenti tabella dei documenti
	 * @return 1 se il documento viene inviato correttamente, 0 altrimenti
	 */
	public static int sendDocumento(SelectionKey key, Documento Documento, HashMap<String,UtenteOnline> clienti_online, HashMap<String,Documento> tabella_documenti){
		// TODO Auto-generated method stub
		int sezioni = Documento.getNumero_sezioni(); int flag = 0;
		ArrayList<Integer> sezioni_occupate = Documento.getSezioni_Occupate();
		for(int i = 0; i<sezioni; i++) { //invio le sezioni una per volta
			if (sezioni_occupate.get(i)== 1)
				sendMessaggioGenerico(key, "La sezione "+ i+ "è occupata\n", clienti_online, tabella_documenti);
			else sendMessaggioGenerico(key, "La sezione "+ i+ "è libera\n", clienti_online, tabella_documenti);
			if ((flag =sendShow(key, Documento.getPathSezione(i), clienti_online, tabella_documenti)) == 0) return 0;
			else if (flag == -1) return -1;
			sendMessaggioGenerico(key, "\n", null, null); //carattere di separazione tra una sezione e l'altra
		}
		return 1;
	}
	/**
	 * 
	 * @param tab_chat tabella contenente le chat attive
	 * @return oggetto che rappresenta la chat 
	 */
	public static chat createchat(HashMap<String,chat> tab_chat) {
		// TODO Auto-generated method stub
		InetAddress address = null;
		MulticastSocket s = null;
		Random random = new Random();
		while (true) {
			String ip = "";
			 ip = (random.nextInt(240)+224) + "." + random.nextInt(256)+ "."+ random.nextInt(256)+ "."+random.nextInt(256); //creo random l'indirizzo
			 if (ip.equals("224.0.0.2") ||ip.equals("224.0.0.1") || ip.equals("224.0.1.1")) //controllo che non sia un indirizzo speciale
				 continue;
			 try {
				 address = InetAddress.getByName(ip);
				 boolean repeat = false;
				 for (chat value : tab_chat.values()) { //controllo che non sia già stato assegnato ad un altro documento
				        if (value.getAddress().toString().equals(address.toString())) {
				        	repeat = true; break;
				        }
				    }
				 if (repeat == true) continue;
			 }catch (UnknownHostException e) {continue;}
			for (int i=1024; i<65535; i++){ //cerco una porta libera
				 try {
					 s =new MulticastSocket(i);
					 s.setTimeToLive(1);
					 try {
						 s.joinGroup(address);
					 } catch (Exception e) {break;}
					 s.close();
					 return (new chat(address, i));
				 } catch (Exception e) {continue;}
			 }
		}
	}
	
	/**
	 * 
	 * @param key chiave associata all'utente
	 * @param PathSezione percorso file della sezione da inviare
	 * @param clienti_online tabella clienti online
	 * @param tabella_documenti tabella dei documenti
	 * @return 1 se la sezione del documento identificata da PathSezione viene inviata correttamente, 0 altrimenti
	 */
	public static int sendShow(SelectionKey key, String PathSezione, HashMap<String, UtenteOnline> clienti_online,
			HashMap<String, Documento> tabella_documenti) {
		FileInputStream fis;
		if (PathSezione == null || !Files.exists(Paths.get(PathSezione))) return 0;
		try {
			fis = new FileInputStream(PathSezione);
			FileChannel channel = fis.getChannel();
			channel.transferTo(0,channel.size(), (WritableByteChannel) key.channel());
			channel.close();
			fis.close();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			return 0;
		} catch (IOException e) {
			disconnect(key, clienti_online, tabella_documenti);
		}
		return 1;
	}

}
