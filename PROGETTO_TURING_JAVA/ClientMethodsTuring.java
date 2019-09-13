import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.SocketChannel;
import java.nio.channels.WritableByteChannel;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
/**
 * Insiemi di mettodi statici usati dal client per inviare richieste al server
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class ClientMethodsTuring {
	
	/**
	 * 
	 * @param Operazione tipo di operazione da richiedere al server
	 * @param PathDocumento percorso file della sezione da inviare al server
	 * @param socketclient socket con cui comunico con il server
	 * @param buffer buffer per leggere/scrivere dal/sul canale
	 * @return true in caso la sezione viene inviata con successo, false altrimenti
	 */
	public static boolean sendSezione(String Operazione, String PathDocumento,  SocketChannel socketclient, ByteBuffer buffer) {
		FileInputStream fis;
		try {
			//invio richiesta di operazione
			buffer.clear();
			buffer.put(Operazione.getBytes());
			buffer.flip();
			while (buffer.hasRemaining())
				socketclient.write(buffer);
			//aspetto che mi arrivi l'ok 
			buffer.clear();
			socketclient.read(buffer);
			buffer.clear();
			
			//invio la sezione
			fis = new FileInputStream(PathDocumento);
			FileChannel channel = fis.getChannel();
			long size = channel.size();
			System.out.println(size);
			buffer.putLong(size); buffer.flip();
			while (buffer.hasRemaining())
				socketclient.write(buffer);
			buffer.clear();
			channel.transferTo(0,channel.size(), (WritableByteChannel) socketclient);
			channel.close();
			fis.close();
		} catch (IOException e) {
			System.out.println("La connessione con il server è caduta"); return false;
		}
		return true;
		
	}
	/**
	 * 
	 * @param Nome nome dell'utente che fa richiesta
	 * @param Documento nome del documento richiesto
	 * @param sezione numero di sezione del documento
	 * @param socketclient socket con cui comunico con il server
	 * @param buffer buffer per leggere/scrivere dal/sul canale
	 * @return 1 in caso ricevo la sezione del documento correttamente, 0 se non ho i permessi per editare, -1 se cade la connessione con il server
	 */
	public static int requestedit(String Nome, String Documento, String sezione, SocketChannel socketclient, ByteBuffer buffer, String editor) {
	// TODO Auto-generated method stub
		long sizefile = 1;
		String Operazione = "edit "+ Documento + " "+ sezione;
		buffer.clear();
		buffer.put(Operazione.getBytes());
		buffer.flip();
		try {
			while (buffer.hasRemaining())
				socketclient.write(buffer);
			buffer.clear();
			int bytes = 0;
			while (bytes <8) { //leggo fino a ricevere la dimensione della sezione
				bytes+=socketclient.read(buffer);
				System.out.println(bytes);
			}
			
			buffer.flip();
			sizefile= buffer.getLong(); //mi salvo la dimensione della sezione
		} catch (IOException e) {
			System.out.println("Impossibile contattare il server");
			return -1;
		}
		if (sizefile == 0) return 0; //se posso effettuare editing la dimensione ricevuta è >= 1
		System.out.println("LA DIMENSIONE DEL FILE è DI "+ sizefile);
		int bytes= 0;
		buffer.clear();			
		try {
			//creo una copia locale del documento e sezione richiesto
			if (!Files.exists(Paths.get("C:\\Client"+Nome))) 
				Files.createDirectories(Paths.get("C:\\Client"+Nome));	
			String Nomefile = "C:\\Client"+Nome+"\\"+Documento+"sezione"+sezione+".txt";
			if (Files.exists(Paths.get(Nomefile))) Files.delete(Paths.get(Nomefile));
			Files.createFile(Paths.get(Nomefile));
			FileChannel file = FileChannel.open(Paths.get(Nomefile), StandardOpenOption.WRITE);
			while ( bytes < (sizefile-1) ) { //leggo tutto il contenuto della sezione 
				bytes+=socketclient.read(buffer);
				System.out.println("bytes letti: "+bytes);
				buffer.flip();
				while (buffer.hasRemaining())
					file.write(buffer);
				buffer.clear();
			}
			file.close();
			ProcessBuilder pb = new ProcessBuilder(editor, Nomefile); //eseguo l'editor
			pb.start();	
			} catch (IOException e) {
				// TODO Auto-generated catch block
				System.out.println("la connessione con il server è caduta");
				return -1;
			}
		return 1;
	}
	/**
	 * 
	 * @param Operazione operazione richiesta 
	 * @param socketclient socket con cui comunico con il server
	 * @param buffer buffer per leggere/scrivere dal/sul canale
	 * @return indirizzo della chat
	 */
	public static String getIPChat(String Operazione, SocketChannel socketclient, ByteBuffer buffer) {
		// TODO Auto-generated method stub
		buffer.clear();
		buffer.put(Operazione.getBytes());
		buffer.flip();
		String laststring=null;
		try {
		while (buffer.hasRemaining())
			socketclient.write(buffer); 
		buffer.clear();
		StringBuilder list = new StringBuilder();
		int bytes= 0;
		boolean stop = false;
		while ( stop == false ) {
			bytes=socketclient.read(buffer);
			buffer.flip();
			while (buffer.hasRemaining()) {
				char nextchar = (char)buffer.get();
				if (nextchar == '\n') { 
					String string = list.toString();
					if (string.equals("finish")) {stop = true; break;} //se la riga letta indica che l'indirizzo è stato letto allora termino
					System.out.println(string);
					laststring = string; //mi salvo l'ultima riga letta che non coincide con "finish"
					list = new StringBuilder(); //mi preparo a costruire un'altra riga
				} else list.append(nextchar); //costruisco la stringa
			}
			buffer.clear();
		}
		} catch (IOException e) {
			System.out.println("Problemi con la connessione server"); return null;
		}
		return laststring;
	}
	/**
	 * 
	 * @param Operazione richiesta al server
	 * @param socketclient socket con cui comunico con il server
	 * @param buffer buffer per leggere/scrivere dal/sul canale
	 * @return true se ho letto e stampato correttamente il contenuto, false altrimenti
	 */
	public static boolean getlist(String Operazione, SocketChannel socketclient, ByteBuffer buffer) {
		// TODO Auto-generated method stub
		buffer.clear();
		buffer.put(Operazione.getBytes());
		buffer.flip();
		try {
		while (buffer.hasRemaining())
			socketclient.write(buffer); 
		buffer.clear();
		StringBuilder list = new StringBuilder();
		int bytes= 0;
		boolean stop = false;
		while ( stop == false ) {
			bytes=socketclient.read(buffer);
			buffer.flip();
			while (buffer.hasRemaining()) {
				char nextchar = (char)buffer.get();
				if (nextchar == '\n') { //ho letto una riga
					String string = list.toString(); //mi salvo la stringa 
					if (string.equals("finish")) {stop = true; break;} //incontro carattere di terminazione
					System.out.println(string); 
					list = new StringBuilder();//mi preparo a leggere un'altra riga
				} else list.append(nextchar); 
			}
			buffer.clear();
		}
		} catch (IOException e) {
			System.out.println("Problemi con la connessione server"); return false;
		}
		return true;
	}
	/**
	 * @param Operazione richiesta al server
	 * @param socketclient socket con cui comunico con il server
	 * @param buffer buffer per leggere/scrivere dal/sul canale
	 * @return esito dell'operazione
	 */
	public static int sendOp(String Operazione, SocketChannel socketclient, ByteBuffer buffer) {
		buffer.clear();
		buffer.put(Operazione.getBytes());
		buffer.flip();
		int flag = 0;
		try {
			while (buffer.hasRemaining())
				socketclient.write(buffer);
			buffer.clear();
			socketclient.read(buffer);
			buffer.flip();
			flag = buffer.getInt();
		} catch (IOException e) {
			System.out.println("Impossibile contattare il server");
			return -1;
		}
		return flag;
	}
}
