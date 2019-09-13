import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.ArrayList;
import java.util.Vector;
/**
 * Classe associata ad un utente che si registra. Contiene una serie di informazioni associate all'utente
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class User extends UnicastRemoteObject implements UserInt{
	private static final long serialVersionUID = 1L;
	protected ArrayList<String> lista_documenti; //lista documenti su cui l'utente può effettuare l'editing
	protected Vector<String> messaggi_pendenti; //messaggi da spedire all'utente quando è online
	protected String Password; //password usata per accedere
	public User (String Password) throws RemoteException {
		this.Password = Password;
		lista_documenti = new ArrayList<String>();
		messaggi_pendenti = new Vector<String>();
		
	}
	
	//insieme di metodi con operazioni elementari
	public void add_doc(String doc) {
		if (!this.lista_documenti.contains(doc))
			this.lista_documenti.add(doc);
	}

	public ArrayList<String> getListaDocumenti() {
		return this.lista_documenti;
	}
	public void addMessaggioPendente (String msg) {
		this.messaggi_pendenti.add(msg);
	}
	public Vector<String> getMsg () {
		if (this.messaggi_pendenti.size() != 0)
			return this.messaggi_pendenti;
		return null;
	}
	public void removeMsg () {
		this.messaggi_pendenti.removeAllElements();
	}
	public String getPassword() {
		// TODO Auto-generated method stub
		return this.Password;
	}

}
