import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
/**
 *  associato a un singolo utente e che si mette in attesa di ricevere messsaggi di notifica
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class Notify implements Runnable {
	
	private SocketChannel socketnotify; //socket con cui comunicare con il server
	private String Name; //nome dell'utente
	public Notify(String Name, int port) {
		// TODO Auto-generated constructor stub
		try {
			SocketAddress address = new InetSocketAddress("127.0.0.1", port);
			this.socketnotify = SocketChannel.open();
			this.socketnotify.connect(address);
			this.Name = Name;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	@Override
	public void run() {
		// TODO Auto-generated method stub
		
		try {
			ByteBuffer buffer = ByteBuffer.allocate(1024);
			String notify = "notify "+ this.Name;
			buffer.put(notify.getBytes());
			buffer.flip();
			while (buffer.hasRemaining())
				this.socketnotify.write(buffer);
			buffer.clear();
			
			int bytes = 0;
			while ((bytes = this.socketnotify.read(buffer)) != -1) { //leggo fin quando è attivo il server
				StringBuilder doc = new StringBuilder();
				//System.out.println("bytes letti "+ bytes);
				buffer.flip();
				while (buffer.hasRemaining()) { //costruisco una stringa che rappresenta il contenuto del buffer
					char nextchar = (char) buffer.get();
					if (nextchar == '\n') {
						System.out.println(doc.toString());
						doc = new StringBuilder();
					} else doc.append(nextchar);
				}
				buffer.clear();
			}
			
		} catch (IOException e) {
			System.out.println("la connessione con il server è caduta");
		}
		
	}

}
