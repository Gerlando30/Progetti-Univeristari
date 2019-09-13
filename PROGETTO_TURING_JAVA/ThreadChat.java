import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.nio.file.DirectoryStream;
import java.util.ArrayList;
/**
 * Thread che viene attivato quando un utente sta effettuando l'edeting di una certa sezione. si mette in attesa di ricevere
 * nuovi messaggi sulla chat del documento 
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class ThreadChat implements Runnable {
	MulticastSocket group; 
	ArrayList<String> lista_messaggi; //lista dei messaggi che arrivano nella chat
	/**
	 * 
	 * @param ip_chat coppia <porta, indirizzo> della chat
	 * @param lista_messaggi 
	 * @throws NumberFormatException 
	 * @throws IOException
	 */
	public ThreadChat(String[] ip_chat, ArrayList<String> lista_messaggi) throws NumberFormatException, IOException {
		this.group = new MulticastSocket(Integer.parseInt(ip_chat[0]));
		InetAddress addressg = InetAddress.getByName(ip_chat[1]);
		group.joinGroup(addressg); //mi unisco al gruppo 
		this.lista_messaggi = lista_messaggi;
	}
	@Override
	public void run() {
		// TODO Auto-generated method stub
		while (!Thread.interrupted()) { 
			DatagramPacket packet = new DatagramPacket(new byte[1024],1024);;
			try {
				//ricevo il pacchetto e aggiungo il messaggio nella lista_messaggi
				this.group.receive(packet); 
				String messaggio = new String(
						packet.getData(),
						packet.getOffset(),
						packet.getLength(),
						"UTF-8");
				synchronized (lista_messaggi) {
					this.lista_messaggi.add(messaggio);
				}
			} catch (IOException e) {break;}
		}
	}
	

}
