import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.Vector;
/**
 * Classe contenente un insieme di informazioni necessarie per gestire lo stato dell'utente quando questo è online
 * @author GERLANDO GRAMAGLIA 530269
 *
 */
public class UtenteOnline {
	
	//protected String Documento;
	//protected int sezione;
	protected String Username;
	protected User user; //info "permanenti" dell'utente
	protected SocketChannel socket_notify; //socket per invio notifiche
	protected SocketChannel socketclient; //socket per la comunicazione standard
	protected String typeOperation; //tipo di operazione da svolgere { SENDFILE, SENDMSG, SENDBOOLEAN}
	protected String Nomedocumento; //documento che si sta editando
	protected int sezione; //sezione che si sta editando
	protected String messaggio; //messaggio da inviare{LIST, MESSAGGIO GENERICO, ECC}
	protected ByteBuffer Buffer; //buffer per inviare esito operazione e/o dimensione file
	private boolean editing;
	
	
	public UtenteOnline(User user, String username) {
		this.Username= username;
		this.user = user;
		this.socket_notify = null;
		this.socketclient = null;
		this.typeOperation = null;
		this.Nomedocumento = null;
		this.sezione = -1;
		this.messaggio = null;
		this.editing = false;
		this.Buffer = ByteBuffer.allocate(1024);
	}
	/**
	 * 
	 * @param op tipo di operazione da completare { show, edit, end-edit }
	 * @param Nomedocumento nome del documento
	 * @param sezione sezione del documento
	 */
	public void setOperation (String op, String Nomedocumento, int sezione) {
		this.typeOperation = op;
		this.Nomedocumento =Nomedocumento;
		this.sezione = sezione;
	}
	/**
	 * 
	 * @param op tipo di operazione da completare
	 * @param intbuffer valore intero da inserire nel buffer
	 * prepara il buffer con il contenuto da spedire, che può essere un flag di avvenuta/rifiuto operazione
	 */
	public void setOperation (String op, int intbuffer) {
		this.typeOperation = op;
		this.Buffer.clear(); this.Buffer.putInt(intbuffer);
		this.Buffer.flip();
	}
	
	public void setOperation(String op, String messaggio) {
		// TODO Auto-generated method stub
		this.typeOperation = op;
		this.messaggio = messaggio;
	}
	/**
	 * 
	 * @param op tipo di operazione da completare
	 * @param l valore long da inviare al client
	 */
	public void setOperation(String op, long l) {
		// TODO Auto-generated method stub
		this.typeOperation = op;
		this.Buffer.clear(); this.Buffer.putLong(l);
		this.Buffer.flip();
	}
	public void setOperation(String string) {
		// TODO Auto-generated method stub
		this.typeOperation = string;
		
	}
	/**
	 * invocata dal server quando il cliente termina l'editing di un documento
	 */
	public void end_edit () {
		this.Nomedocumento = null;
		this.sezione = -1;
		this.editing = false;
	}
	//Seguono una serie di metodi get e set
	public void setUser (User user) {
		this.user = user;
	}
	public void setUsername (String username) {
		this.Username = username;
	}
	public ByteBuffer getBuffer () {
		return this.Buffer;
	}
	public String getOperation () {
		return this.typeOperation;
	}
	public String getNomeDocumento () {
		return this.Nomedocumento;
	}
	public User getUser () {
		return user;
	}
	public SocketChannel getSocketclient() {
		return socketclient;
	}
	public void setSocketclient(SocketChannel socketclient) {
		this.socketclient = socketclient;
	}
	public SocketChannel getSocket_notify() {
		return socket_notify;
	}
	public void setSocket_notify(SocketChannel socket_notify) {
		this.socket_notify = socket_notify;
	}
	
	
	public ArrayList<String> getListaDocumenti () {
		return new ArrayList<String>(this.user.getListaDocumenti());
	}
	public Vector<String> getMessaggiPendenti() {
		// TODO Auto-generated method stub
		return this.user.getMsg();
	}
	public void addMessaggioPendente (String Msg) {
		this.user.addMessaggioPendente(Msg);
	}

	public void addDocumento(String nomeDocumento) {
		// TODO Auto-generated method stub
		this.user.lista_documenti.add(nomeDocumento);
	}
	public void removeMessaggiPendenti() {
		// TODO Auto-generated method stub
		this.user.messaggi_pendenti.removeAllElements();
	}
	
	public int getsezione() {
		// TODO Auto-generated method stub
		return this.sezione;
	}
	public String getUserName() {
		// TODO Auto-generated method stub
		return this.Username;
	}
	public void setMessaggio (String messaggio) {
		this.messaggio = messaggio;
	}
	public String getMessaggio () {
		return this.messaggio;
	}
	
	public boolean editing() {
		// TODO Auto-generated method stub
		return this.editing;
	}
	public void setediting () {
		this.editing = true;
	}

}
